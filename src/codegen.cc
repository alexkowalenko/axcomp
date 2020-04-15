//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "codegen.hh"

#include <iostream>

#include "llvm/Support/FormatVariadic.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <memory>

#include "ast.hh"
#include "error.hh"
#include "parser.hh"
#include "type.hh"

namespace ax {

inline constexpr bool debug_codegen{false};

template <typename... T> inline void debug(const T &... msg) {
    if constexpr (debug_codegen) {
        std::cerr << std::string(formatv(msg...)) << std::endl;
    }
}

using namespace llvm::sys;

constexpr auto file_ext_llvmri{".ll"};
constexpr auto file_ext_obj{".o"};

CodeGenerator::CodeGenerator(Options &o, TypeTable &t, Importer &i)
    : options(o), types(t), importer(i), filename("main"), builder(context), last_value(nullptr) {
    top_symboltable = std::make_shared<SymbolTable<std::pair<Value *, Attr>>>(nullptr);
    current_symboltable = top_symboltable;
    TypeTable::setTypes(context);
}

void CodeGenerator::visit_ASTModule(ASTModulePtr ast) {
    // Set up code generation
    module_name = ast->name;
    init();

    // Set up builtins
    setup_builtins();

    if (ast->import) {
        ast->import->accept(this);
    }

    // Top level consts
    top_level = true;
    doTopDecs(ast->decs);

    top_level = false;
    // Do procedures which generate functions first
    doProcedures(ast->procedures);

    // Set up the module as a function
    // Make the function type:  (void) : int
    // main returns int
    std::vector<llvm::Type *> proto;
    FunctionType *ft = FunctionType::get(llvm::Type::getInt64Ty(context), proto, false);

    auto function_name = filename;
    if (options.output_funct) {
        function_name = "output";
    } else if (options.output_main) {
        function_name = filename;
    } else {
        function_name = ASTQualident::make_coded_id(ast->name, "main");
    }
    Function *f = Function::Create(ft, Function::ExternalLinkage, function_name, module.get());

    // Create a new basic block to start insertion into.
    BasicBlock *block = BasicBlock::Create(context, "entry", f);
    builder.SetInsertPoint(block);

    // Do declarations - vars
    top_level = true; // we have done the procedures
    ast->decs->accept(this);

    // Go through the statements
    has_return = false;
    std::for_each(ast->stats.begin(), ast->stats.end(),
                  [this](auto const &x) { x->accept(this); });
    if (!has_return) {
        builder.CreateRet(TypeTable::IntType->get_init());
    }

    // Validate the generated code, checking for consistency.
    verifyFunction(*f);

    // change the filename to generate module.obj
    filename = ast->name;
}

void CodeGenerator::visit_ASTImport(ASTImportPtr ast) {
    debug("CodeGenerator::visit_ASTImport");

    auto symbols = make_Symbols(nullptr);
    std::for_each(begin(ast->imports), end(ast->imports), [this, &symbols](auto const &i) {
        auto found = importer.find_module(i.first->value, symbols, types);
        assert(found);

        // convert table to ValueSymboltable
        std::for_each(symbols->cbegin(), symbols->cend(), [this, i](auto const &s) {
            auto name = s.first;
            auto type = s.second.first;
            debug("CodeGenerator::visit_ASTImport get {0} : {1}", name, type->get_name());

            if (is_referencable(type->id)) {

                module->getOrInsertGlobal(name, type->get_llvm());
                GlobalVariable *gVar = module->getNamedGlobal(name);

                debug("CodeGenerator::visit_ASTImport var {0}", name);

                top_symboltable->put(name, {gVar, Attr::global});
            } else if (type->id == TypeId::procedure) {
                debug("CodeGenerator::visit_ASTImport proc {0}", name);
                auto *funcType = (FunctionType *)type->get_llvm();

                auto *func = Function::Create(funcType, Function::LinkageTypes::ExternalLinkage,
                                              name, module.get());
                verifyFunction(*func);
                current_symboltable->put(name, {func, Attr::null});
                types.put(name, type); // we should stop putting functions into the type table
            } else {
                // ignore functions for the moment
            }
        });
    });
}

void CodeGenerator::doTopDecs(ASTDeclarationPtr ast) {
    if (ast->cnst) {
        doTopConsts(ast->cnst);
    }
    if (ast->var) {
        doTopVars(ast->var);
    }
}

void CodeGenerator::doTopVars(ASTVarPtr ast) {
    debug("CodeGenerator::doTopVars");
    for (auto const &c : ast->vars) {
        llvm::Type *type = getType(c.second);
        auto        var_name = gen_module_id(c.first->value);
        module->getOrInsertGlobal(var_name, type);
        GlobalVariable *gVar = module->getNamedGlobal(var_name);

        GlobalValue::LinkageTypes linkage = GlobalValue::LinkageTypes::InternalLinkage;
        if (c.first->is(Attr::global)) {
            linkage = GlobalValue::LinkageTypes::ExternalLinkage;
        }
        gVar->setLinkage(linkage);
        auto *init = getType_init(c.second);
        gVar->setInitializer(init);
        current_symboltable->put(c.first->value, {gVar, Attr::null});
    }
}

void CodeGenerator::doTopConsts(ASTConstPtr ast) {
    debug("CodeGenerator::doTopConsts");
    for (auto const &c : ast->consts) {
        debug("CodeGenerator::doTopConsts type: {0}", c.type->type_info->get_name());
        auto *type = getType(c.type);
        auto  const_name = gen_module_id(c.ident->value);
        module->getOrInsertGlobal(const_name, type);
        GlobalVariable *gVar = module->getNamedGlobal(const_name);

        c.value->accept(this);
        GlobalValue::LinkageTypes linkage = GlobalValue::LinkageTypes::InternalLinkage;
        if (c.ident->is(Attr::global)) {
            linkage = GlobalValue::LinkageTypes::ExternalLinkage;
        }
        gVar->setLinkage(linkage);
        if (isa<ConstantInt>(last_value)) {
            gVar->setInitializer(dyn_cast<ConstantInt>(last_value));
        } else {
            throw CodeGenException("Expression based CONSTs not supported.", ast->get_location());
        }
        gVar->setConstant(true);

        current_symboltable->put(c.ident->value, {gVar, Attr::null});
    }
}

void CodeGenerator::doProcedures(std::vector<ASTProcedurePtr> const &procs) {
    std::for_each(procs.begin(), procs.end(), [this](auto const &x) { x->accept(this); });
}

void CodeGenerator::visit_ASTDeclaration(ASTDeclarationPtr ast) {
    if (ast->cnst && !top_level) {
        ast->cnst->accept(this);
    }
    if (ast->var && !top_level) {
        ast->var->accept(this);
    }
}

void CodeGenerator::visit_ASTConst(ASTConstPtr ast) {
    debug("CodeGenerator::visit_ASTConst");
    for (auto const &c : ast->consts) {
        c.value->accept(this);
        auto *val = last_value;

        auto name = c.ident->value; // consts within procedures don't need
                                    // to be renamed
        debug("create const: {}", name);

        // Create const
        auto *function = builder.GetInsertBlock()->getParent();
        auto *alloc = createEntryBlockAlloca(function, name, c.type);
        builder.CreateStore(val, alloc);

        current_symboltable->put(name, {alloc, Attr::null});
    }
}

void CodeGenerator::visit_ASTVar(ASTVarPtr ast) {
    debug("CodeGenerator::visit_ASTVar");
    for (auto const &c : ast->vars) {

        // Create variable
        auto name = c.first->value;
        debug("create var: {}", name);

        auto *      function = builder.GetInsertBlock()->getParent();
        auto        type = c.second;
        AllocaInst *alloc = createEntryBlockAlloca(function, name, type);
        auto *      init = getType_init(type);
        builder.CreateStore(init, alloc);

        alloc->setName(name);
        debug("set name: {}", name);
        current_symboltable->put(name, {alloc, Attr::null});
    }
    debug("finish var");
}

void CodeGenerator::visit_ASTProcedure(ASTProcedurePtr ast) {
    debug("CodeGenerator::visit_ASTProcedure {0}", ast->name->value);
    // Make the function arguments
    std::vector<llvm::Type *> proto;
    std::for_each(ast->params.begin(), ast->params.end(), [this, ast, &proto](auto const &p) {
        auto type = getType(p.second);
        if (p.first->is(Attr::var)) {
            debug(" CodeGenerator::visit_ASTProcedure VAR {0}", p.first->value);
            type = type->getPointerTo();
        }
        proto.push_back(type);
    });

    llvm::Type *returnType = nullptr;
    if (ast->return_type == nullptr) {
        returnType = llvm::Type::getVoidTy(context);
    } else {
        returnType = getType(ast->return_type);
    }

    auto                      proc_name = gen_module_id(ast->name->value);
    FunctionType *            ft = FunctionType::get(returnType, proto, false);
    GlobalValue::LinkageTypes linkage = Function::InternalLinkage;
    if (ast->name->is(Attr::global)) {
        linkage = Function::ExternalLinkage;
    }
    Function *f = Function::Create(ft, linkage, proc_name, module.get());

    // Create a new basic block to start insertion into.
    BasicBlock *block = BasicBlock::Create(context, "entry", f);
    builder.SetInsertPoint(block);

    auto former_symboltable = current_symboltable;
    current_symboltable =
        std::make_shared<SymbolTable<std::pair<Value *, Attr>>>(former_symboltable);

    // Set paramater names
    unsigned i = 0;
    for (auto &arg : f->args()) {
        auto param_name = ast->params[i].first->value;
        arg.setName(param_name);
        auto type_name = ast->params[i].second;

        Attr attr = Attr::null;
        if (ast->params[i].first->is(Attr::var)) {
            attr = Attr::var;
        }
        // Create an alloca for this variable.
        AllocaInst *alloca = createEntryBlockAlloca(f, param_name, type_name, attr == Attr::var);

        // Store the initial value into the alloca.
        builder.CreateStore(&arg, alloca);

        // put also into symbol table
        current_symboltable->put(param_name, {alloca, attr});
        debug("put {} into symboltable", param_name);
        i++;
    };

    // Do declarations
    ast->decs->accept(this);

    // for recursion
    top_symboltable->put(ast->name->value, {f, Attr::null});

    // Go through the statements
    has_return = false;
    std::for_each(ast->stats.begin(), ast->stats.end(),
                  [this](auto const &x) { x->accept(this); });
    if (!has_return) {
        if (ast->return_type == nullptr) {
            builder.CreateRetVoid();
        } else {
            builder.CreateRet(last_value);
        }
    }
    current_symboltable = former_symboltable;
    // Validate the generated code, checking for consistency.
    verifyFunction(*f);
}

void CodeGenerator::visit_ASTAssignment(ASTAssignmentPtr ast) {
    debug(" CodeGenerator::visit_ASTAssignment {0}", std::string(*(ast->ident)));
    ast->expr->accept(this);
    auto *val = last_value;

    auto var = find_var_Identifier(ast->ident);
    debug(" CodeGenerator::visit_ASTAssignment VAR {0}", var);
    if (var) {
        // Handle VAR assignment
        is_var = true; // Set change in visit_ASTIdentifierPtr to notify
                       // this is a write of a VAR variable
        visit_ASTDesignator(ast->ident);
    } else {
        visit_ASTDesignatorPtr(ast->ident);
    }
    builder.CreateStore(val, last_value);
    is_var = false;
}

void CodeGenerator::visit_ASTReturn(ASTReturnPtr ast) {
    if (ast->expr) {
        ast->expr->accept(this);
        if (top_level && last_value->getType() != llvm::Type::getInt64Ty(context)) {
            // in the main module - return i64 value
            last_value = builder.CreateZExt(last_value, llvm::Type::getInt64Ty(context));
        }
        builder.CreateRet(last_value);
    } else {
        builder.CreateRetVoid();
    }
    has_return = true;
}

void CodeGenerator::visit_ASTExit(ASTExitPtr ast) {
    if (last_end) {
        builder.GetInsertBlock();
        builder.CreateBr(last_end);
    } else {
        throw CodeGenException("EXIT: no enclosing loop.", ast->get_location());
    }
}

void CodeGenerator::visit_ASTCall(ASTCallPtr ast) {
    debug("CodeGenerator::visit_ASTCall");
    // Look up the name in the global module table.
    Function *callee = nullptr;
    auto      name = ast->name->ident->make_coded_id();
    try {
        auto res = current_symboltable->find(name);
        if (!res) {
            throw CodeGenException(formatv("function: {0} not found 1", name),
                                   ast->get_location());
        }
        callee = llvm::dyn_cast<Function>(res->first);
        assert(callee != nullptr);
    } catch (...) {
        debug("CodeGenerator::visit_ASTCall exception");
        throw CodeGenException(formatv("function: {0} not found 2", name), ast->get_location());
    }

    auto res = types.find(name);
    if (!res) {
        throw CodeGenException(formatv("function: {0} not found 3", name), ast->get_location());
    }
    auto typeFunction = std::dynamic_pointer_cast<ProcedureType>(*res);
    assert(typeFunction);

    std::vector<Value *> args;
    auto                 i = 0;
    for (auto const &a : ast->args) {
        if (typeFunction->params[i].second == Attr::var) {
            debug("CodeGenerator::visit_ASTCall VAR parameter");
            // Var Parameter

            // Get Identifier, get pointer set to last_value
            // visit_ASTIdentifierPtr
            // This works, since the Inspector checks that VAR arguments are
            // only identifiers
            auto ptr = a->expr->term->factor->factor;
            auto p2 = std::get<ASTDesignatorPtr>(ptr)->ident;

            debug("CodeGenerator::visit_ASTCall identifier {0}", p2->id->value);
            visit_ASTIdentifierPtr(p2->id);
        } else {
            // Reference Parameter
            a->accept(this);
        }
        args.push_back(last_value);
        i++;
    }
    last_value = builder.CreateCall(callee, args);
}

void CodeGenerator::visit_ASTIf(ASTIfPtr ast) {
    debug("CodeGenerator::visit_ASTIf");

    // IF
    ast->if_clause.expr->accept(this);

    // Create blocks, insert then block
    auto *                    funct = builder.GetInsertBlock()->getParent();
    BasicBlock *              then_block = BasicBlock::Create(context, "then", funct);
    BasicBlock *              else_block = BasicBlock::Create(context, "else");
    std::vector<BasicBlock *> elsif_blocks;
    int                       i = 0;
    std::for_each(begin(ast->elsif_clause), end(ast->elsif_clause),
                  [&](auto const & /*not used*/) {
                      auto *e_block = BasicBlock::Create(context, formatv("elsif{0}", i++));
                      elsif_blocks.push_back(e_block);
                  });
    BasicBlock *merge_block = BasicBlock::Create(context, "ifcont");

    if (!ast->elsif_clause.empty()) {
        builder.CreateCondBr(last_value, then_block, elsif_blocks[0]);
    } else {
        if (ast->else_clause) {
            builder.CreateCondBr(last_value, then_block, else_block);
        } else {
            builder.CreateCondBr(last_value, then_block, merge_block);
        }
    }

    // THEN
    builder.SetInsertPoint(then_block);

    std::for_each(begin(ast->if_clause.stats), end(ast->if_clause.stats),
                  [this](auto const &s) { s->accept(this); });
    builder.CreateBr(merge_block);

    i = 0;
    for (auto const &e : ast->elsif_clause) {

        // ELSEIF
        funct->getBasicBlockList().push_back(elsif_blocks[i]);
        builder.SetInsertPoint(elsif_blocks[i]);

        // do expr
        e.expr->accept(this);
        BasicBlock *t_block = BasicBlock::Create(context, "then", funct);

        if (&e != &ast->elsif_clause.back()) {
            builder.CreateCondBr(last_value, t_block, elsif_blocks[i + 1]);
        } else {
            // last ELSIF block - branch to else_block
            if (ast->else_clause) {
                builder.CreateCondBr(last_value, t_block, else_block);
            } else {
                builder.CreateCondBr(last_value, t_block, merge_block);
            }
        }

        // THEN
        builder.SetInsertPoint(t_block);
        std::for_each(begin(e.stats), end(e.stats), [this](auto const &s) { s->accept(this); });
        builder.CreateBr(merge_block);
        i++;
    }

    // Emit ELSE block.

    if (ast->else_clause) {
        funct->getBasicBlockList().push_back(else_block);
        builder.SetInsertPoint(else_block);
        auto elses = *ast->else_clause;
        std::for_each(begin(elses), end(elses), [this](auto const &s) { s->accept(this); });
        builder.CreateBr(merge_block);
    }

    // codegen of ELSE can change the current block, update else_block
    // else_block = builder.GetInsertBlock();

    // Emit merge block.
    funct->getBasicBlockList().push_back(merge_block);
    builder.SetInsertPoint(merge_block);
}

void CodeGenerator::visit_ASTFor(ASTForPtr ast) {
    debug("CodeGenerator::visit_ASTFor");

    // do start expr
    ast->start->accept(this);

    // Make the new basic block for the loop header, inserting after current
    // block.
    auto *      funct = builder.GetInsertBlock()->getParent();
    BasicBlock *loop = BasicBlock::Create(context, "loop", funct);

    auto former_symboltable = current_symboltable;
    current_symboltable =
        std::make_shared<SymbolTable<std::pair<Value *, Attr>>>(former_symboltable);
    auto *index = createEntryBlockAlloca(funct, ast->ident->value, TypeTable::IntType->get_llvm());
    builder.CreateStore(last_value, index);
    current_symboltable->put(ast->ident->value, {index, Attr::null});

    // Insert an explicit fall through from the current block to the Loop.
    // Start insertion in LoopBB.
    builder.CreateBr(loop);
    builder.SetInsertPoint(loop);

    // DO
    std::for_each(begin(ast->stats), end(ast->stats), [this](auto const &s) { s->accept(this); });

    // Emit the step value.
    Value *step = nullptr;
    if (ast->by) {
        (*ast->by)->accept(this);
        step = last_value;
    } else {
        step = TypeTable::IntType->make_value(1);
    }
    auto * tmp = builder.CreateLoad(index);
    Value *nextVar = builder.CreateAdd(tmp, step, "nextvar");
    builder.CreateStore(nextVar, index);

    // Compute the end condition.
    ast->end->accept(this);
    auto *end_value = last_value;

    // Convert condition to a bool
    Value *endCond = builder.CreateICmpSLE(nextVar, end_value, "loopcond");

    // Create the "after loop" block and insert it.
    BasicBlock *after = BasicBlock::Create(context, "afterloop", funct);

    // Insert the conditional branch into the end of Loop.
    builder.CreateCondBr(endCond, loop, after);

    // Any new code will be inserted in AfterBB.
    builder.SetInsertPoint(after);

    current_symboltable = former_symboltable;
}

void CodeGenerator::visit_ASTWhile(ASTWhilePtr ast) {
    debug("CodeGenerator::visit_ASTWhile");

    // Create blocks
    auto *      funct = builder.GetInsertBlock()->getParent();
    BasicBlock *while_block = BasicBlock::Create(context, "while", funct);
    BasicBlock *loop = BasicBlock::Create(context, "loop");
    BasicBlock *end_block = BasicBlock::Create(context, "end");
    last_end = end_block;

    builder.CreateBr(while_block);
    builder.SetInsertPoint(while_block);

    // Expr
    ast->expr->accept(this);
    builder.CreateCondBr(last_value, loop, end_block);

    // DO
    funct->getBasicBlockList().push_back(loop);
    builder.SetInsertPoint(loop);

    std::for_each(begin(ast->stats), end(ast->stats), [this](auto const &s) { s->accept(this); });

    builder.CreateBr(while_block);

    // END
    funct->getBasicBlockList().push_back(end_block);
    builder.SetInsertPoint(end_block);
}

void CodeGenerator::visit_ASTRepeat(ASTRepeatPtr ast) {
    debug("CodeGenerator::visit_ASTRepeat");

    // Create blocks
    auto *      funct = builder.GetInsertBlock()->getParent();
    BasicBlock *repeat_block = BasicBlock::Create(context, "repeat", funct);
    BasicBlock *end_block = BasicBlock::Create(context, "end");
    last_end = end_block;

    // REPEAT
    builder.CreateBr(repeat_block);
    builder.SetInsertPoint(repeat_block);

    std::for_each(begin(ast->stats), end(ast->stats), [this](auto const &s) { s->accept(this); });

    // Expr
    ast->expr->accept(this);
    builder.CreateCondBr(last_value, end_block, repeat_block);

    // END
    funct->getBasicBlockList().push_back(end_block);
    builder.SetInsertPoint(end_block);
}

void CodeGenerator::visit_ASTLoop(ASTLoopPtr ast) {
    debug("CodeGenerator::visit_ASTLoop");

    // Create blocks
    auto *      funct = builder.GetInsertBlock()->getParent();
    BasicBlock *loop_block = BasicBlock::Create(context, "loop", funct);
    BasicBlock *end_block = BasicBlock::Create(context, "end");
    last_end = end_block;

    // LOOP
    builder.CreateBr(loop_block);
    builder.SetInsertPoint(loop_block);

    std::for_each(begin(ast->stats), end(ast->stats), [this](auto const &s) { s->accept(this); });
    builder.CreateBr(loop_block);

    // END
    funct->getBasicBlockList().push_back(end_block);
    builder.SetInsertPoint(end_block);
}

void CodeGenerator::visit_ASTBlock(ASTBlockPtr ast) {
    debug("CodeGenerator::visit_ASTBlock");

    // Create blocks
    auto *      funct = builder.GetInsertBlock()->getParent();
    BasicBlock *begin_block = BasicBlock::Create(context, "begin", funct);
    BasicBlock *end_block = BasicBlock::Create(context, "end");
    last_end = end_block;

    // BEGIN
    builder.CreateBr(begin_block);
    builder.SetInsertPoint(begin_block);

    // BEGIN
    std::for_each(begin(ast->stats), end(ast->stats), [this](auto const &s) { s->accept(this); });

    builder.CreateBr(end_block);
    // END
    funct->getBasicBlockList().push_back(end_block);
    builder.SetInsertPoint(end_block);
}

void CodeGenerator::visit_ASTExpr(ASTExprPtr ast) {
    ast->expr->accept(this);

    if (ast->relation) {
        auto *L = last_value;
        (*ast->relation_expr)->accept(this);
        auto *R = last_value;
        switch (*ast->relation) {
        case TokenType::equals:
            last_value = builder.CreateICmpEQ(L, R);
            break;
        case TokenType::hash:
            last_value = builder.CreateICmpNE(L, R);
            break;
        case TokenType::less:
            last_value = builder.CreateICmpSLT(L, R);
            break;
        case TokenType::leq:
            last_value = builder.CreateICmpSLE(L, R);
            break;
        case TokenType::greater:
            last_value = builder.CreateICmpSGT(L, R);
            break;
        case TokenType::gteq:
            last_value = builder.CreateICmpSGE(L, R);
            break;
        default:;
        }
    }
}

void CodeGenerator::visit_ASTSimpleExpr(ASTSimpleExprPtr ast) {
    ast->term->accept(this);
    Value *L = last_value;
    // if initial sign exists and is negative, negate the integer
    if (ast->first_sign && ast->first_sign.value() == TokenType::dash) {
        L = builder.CreateSub(TypeTable::IntType->get_init(), L, "negtmp");
        last_value = L;
    }

    for (auto const &t : ast->rest) {
        t.second->accept(this);
        Value *R = last_value;
        switch (t.first) {
        case TokenType::plus:
            last_value = builder.CreateAdd(L, R, "addtmp");
            break;
        case TokenType::dash:
            last_value = builder.CreateSub(L, R, "subtmp");
            break;
        case TokenType::or_k:
            last_value = builder.CreateOr(L, R, "subtmp");
            break;
        default:
            throw CodeGenException("ASTSimpleExpr with sign" + string(t.first),
                                   ast->get_location());
        }
        L = last_value;
    }
}

void CodeGenerator::visit_ASTTerm(ASTTermPtr ast) {
    ast->factor->accept(this);
    Value *L = last_value;
    for (auto const &t : ast->rest) {
        t.second->accept(this);
        Value *R = last_value;
        switch (t.first) {
        case TokenType::asterisk:
            last_value = builder.CreateMul(L, R, "multmp");
            break;
        case TokenType::div:
            last_value = builder.CreateSDiv(L, R, "divtmp");
            break;
        case TokenType::mod:
            last_value = builder.CreateSRem(L, R, "modtmp");
            break;
        case TokenType::ampersand:
            last_value = builder.CreateAnd(L, R, "modtmp");
            break;
        default:
            throw CodeGenException("ASTTerm with sign" + string(t.first), ast->get_location());
        }

        L = last_value;
    }
}

void CodeGenerator::visit_ASTFactor(ASTFactorPtr ast) {
    debug("CodeGenerator::visit_ASTFactor");
    // Visit the appropriate variant
    std::visit(overloaded{[this](auto arg) { arg->accept(this); },
                          [this, ast](ASTFactorPtr const &arg) {
                              debug("visit_ASTFactor: not ");
                              if (ast->is_not) {
                                  debug("visit_ASTFactor: not do");
                                  arg->accept(this);
                                  last_value = builder.CreateNot(last_value);
                              }
                          }},
               ast->factor);
}

void CodeGenerator::get_index(ASTDesignatorPtr ast) {
    debug("CodeGenerator::get_index");
    visit_ASTIdentifierPtr(ast->ident->id);
    auto *               arg_ptr = last_value;
    std::vector<Value *> index{TypeTable::IntType->make_value(0)};

    for (auto const &s : ast->selectors) {

        std::visit(
            overloaded{[this, &index](ASTExprPtr const &s) {
                           // calculate index;
                           s->expr->accept(this);
                           debug("GEP index is Int: {0}", last_value->getType()->isIntegerTy());
                           index.push_back(last_value);
                       },
                       [this, &index](FieldRef const &s) {
                           // calculate index
                           // extract the field index
                           debug("CodeGenerator::get_index record index {0} for {1}", s.second,
                                 s.first->value);
                           assert(s.second >= 0);

                           // record indexes are 32 bit integers
                           auto *idx = ConstantInt::get(llvm::Type::getInt32Ty(context), s.second);
                           index.push_back(idx);
                       }},
            s);
    }
    debug("GEP is Ptr: {0}", arg_ptr->getType()->isPointerTy());
    assert(arg_ptr->getType()->isPointerTy());
    last_value = builder.CreateGEP(arg_ptr, index, "idx");
}

/**
 * @brief Used to fetch values
 *
 * @param ast
 */
void CodeGenerator::visit_ASTDesignator(ASTDesignatorPtr ast) {
    debug("CodeGenerator::visit_ASTDesignator {0}", std::string(*ast));
    visit_ASTQualident(ast->ident);

    // Check if has selectors
    if (ast->selectors.empty()) {
        return;
    }

    // Array structure
    get_index(ast);
    last_value = builder.CreateLoad(last_value, "idx");
}

/**
 * @brief Used to assign values to
 *
 * @param ast
 */
void CodeGenerator::visit_ASTDesignatorPtr(ASTDesignatorPtr ast) {
    debug("CodeGenerator::visit_ASTDesignatorPtr {0}", std::string(*ast));

    visit_ASTQualidentPtr(ast->ident);

    // Check if has selectors
    if (ast->selectors.empty()) {
        return;
    }

    // Array structure
    get_index(ast);
}

void CodeGenerator::visit_ASTQualident(ASTQualidentPtr ast) {
    debug("CodeGenerator::visit_ASTQualident");
    if (ast->qual.empty()) {
        visit_ASTIdentifier(ast->id);
    } else {
        auto new_ast = std::make_shared<ASTIdentifier>();
        new_ast->value = ast->make_coded_id();
        visit_ASTIdentifier(new_ast);
    }
}

void CodeGenerator::visit_ASTQualidentPtr(ASTQualidentPtr ast) {
    debug("CodeGenerator::visit_ASTQualidentPtr");
    if (ast->qual.empty()) {
        visit_ASTIdentifierPtr(ast->id);
    } else {
        auto new_ast = std::make_shared<ASTIdentifier>();
        new_ast->value = ast->make_coded_id();
        visit_ASTIdentifierPtr(new_ast);
    }
}

void CodeGenerator::visit_ASTIdentifier(ASTIdentifierPtr ast) {
    debug("CodeGenerator::visit_ASTIdentifier {0}", ast->value);
    visit_ASTIdentifierPtr(ast);
    last_value = builder.CreateLoad(last_value, ast->value);
}

void CodeGenerator::visit_ASTIdentifierPtr(ASTIdentifierPtr ast) {
    debug("CodeGenerator::visit_ASTIdentifierPtr {0}", ast->value);

    if (auto res = current_symboltable->find(ast->value); res) {
        last_value = res->first;
        if (res->second == Attr::var) {
            debug("CodeGenerator::visit_ASTIdentifierPtr VAR {0}", ast->value);
            last_value = builder.CreateLoad(last_value, ast->value);
        }
        if (is_var) {
            // This is a write of a VAR variable, preseve this value
            last_value = res->first;
        }
        return;
    }
    throw CodeGenException(formatv("identifier {0} unknown", ast->value), ast->get_location());
}

bool CodeGenerator::find_var_Identifier(ASTDesignatorPtr ast) {
    if (auto res = current_symboltable->find(ast->ident->id->value); res) {
        if (res->second == Attr::var) {
            return true;
        }
    }
    return false;
}

void CodeGenerator::visit_ASTInteger(ASTIntegerPtr ast) {
    last_value = TypeTable::IntType->make_value(ast->value);
}

void CodeGenerator::visit_ASTBool(ASTBoolPtr ast) {
    last_value = TypeTable::BoolType->make_value(ast->value);
}

std::string CodeGenerator::gen_module_id(std::string const &id) const {
    return ASTQualident::make_coded_id(module_name, id);
}

/**
 * @brief Create an alloca instruction in the entry block of the function.
 * This is used for mutable variables etc.
 *
 * @param function
 * @param name
 * @return AllocaInst*
 */
AllocaInst *CodeGenerator::createEntryBlockAlloca(Function *function, std::string const &name,
                                                  ASTTypePtr type, bool var) {
    AllocaInst *res = nullptr;
    IRBuilder<> TmpB(&function->getEntryBlock(), function->getEntryBlock().begin());

    std::visit(
        [&](auto) {
            llvm::Type *t = getType(type);
            if (var) {
                t = t->getPointerTo();
            }
            res = TmpB.CreateAlloca(t, nullptr, name);
        },
        type->type);
    return res;
}

AllocaInst *CodeGenerator::createEntryBlockAlloca(Function *function, std::string const &name,
                                                  llvm::Type *type) {
    IRBuilder<> TmpB(&function->getEntryBlock(), function->getEntryBlock().begin());
    return TmpB.CreateAlloca(type, nullptr, name);
}

TypePtr CodeGenerator::resolve_type(ASTTypePtr const &t) {
    debug("CodeGenerator::resolve_type");
    TypePtr result;
    std::visit(overloaded{[this, &result](ASTQualidentPtr const &type) {
                              auto res = types.resolve(type->id->value);
                              if (!res) {
                                  // should be a resloved type this far down
                                  assert(!res);
                              };
                              result = *res;
                          },
                          [t, &result](auto /* not used*/) { result = t->type_info; }},
               t->type);
    return result;
}

llvm::Type *CodeGenerator::getType(ASTTypePtr const &t) {
    debug("CodeGenerator::getType");
    return resolve_type(t)->get_llvm();
}

Constant *CodeGenerator::getType_init(ASTTypePtr const &t) {
    debug("CodeGenerator::getType_init");
    return resolve_type(t)->get_init();
};

void CodeGenerator::setup_builtins() {
    debug("CodeGenerator::setup_builtins");

    for (auto const &f : builtins) {
        debug("function: {0} ", f.first);
        auto *funcType = (FunctionType *)f.second->get_llvm();

        auto *func = Function::Create(funcType, Function::LinkageTypes::ExternalLinkage, f.first,
                                      module.get());
        verifyFunction(*func);
        current_symboltable->put(f.first, {func, Attr::null});
    }
}

void CodeGenerator::init() {
    module = std::make_unique<Module>(module_name, context);
    module->setSourceFileName(module_name);
}

void CodeGenerator::generate_llcode() {
    debug("CodeGenerator::generate_llcode");
    auto            f = filename + file_ext_llvmri;
    std::error_code EC;
    raw_fd_ostream  out_file(f, EC, sys::fs::OF_None);

    if (EC) {
        throw CodeGenException("Could not open file: " + EC.message());
    }
    module->print(out_file, nullptr);
    out_file.flush();
}

void CodeGenerator::generate_objectcode() {
    debug("CodeGenerator::generate_objectcode");

    // Define the target triple
    auto targetTriple = sys::getDefaultTargetTriple();

    // Set up for future cross-compiler
    // InitializeAllTargetInfos();
    // InitializeAllTargets();
    // InitializeAllTargetMCs();
    // InitializeAllAsmParsers();
    // InitializeAllAsmPrinters();

    InitializeNativeTarget();
    InitializeNativeTargetAsmParser();
    InitializeNativeTargetAsmPrinter();

    // Get the target
    std::string error;
    const auto *target = TargetRegistry::lookupTarget(targetTriple, error);

    // Print an error and exit if we couldn't find the requested target.
    // This generally occurs if we've forgotten to initialise the
    // TargetRegistry or we have a bogus target triple.
    if (target == nullptr) {
        throw CodeGenException(error);
    }

    // Use generic CPU without features
    const auto *CPU = "generic";
    const auto *features = "";

    TargetOptions opt;
    auto          RM = Optional<Reloc::Model>();
    auto *targetMachine = target->createTargetMachine(targetTriple, CPU, features, opt, RM);

    module->setDataLayout(targetMachine->createDataLayout());
    module->setTargetTriple(targetTriple);

    auto            f = filename + file_ext_obj;
    std::error_code EC;
    raw_fd_ostream  dest_file(f, EC, sys::fs::OF_None);

    if (EC) {
        throw CodeGenException("Could not open file: " + EC.message());
    }

    legacy::PassManager pass;
    auto                file_type = CGFT_ObjectFile;

    if (targetMachine->addPassesToEmitFile(pass, dest_file, nullptr, file_type)) {
        throw CodeGenException("TargetMachine can't emit a file of this type");
    }
    pass.run(*module);
    dest_file.flush();
}

} // namespace ax