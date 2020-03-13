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
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

#include <fmt/core.h>

#include "astmod.hh"
#include "error.hh"

namespace ax {

inline constexpr bool debug_codegen{false};

template <typename... T> inline void debug(const T &... msg) {
    if constexpr (debug_codegen) {
        std::cerr << fmt::format(msg...) << std::endl;
    }
}

template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...)->overloaded<Ts...>;

using namespace llvm::sys;

inline const std::string file_ext_llvmri{".ll"};
inline const std::string file_ext_obj{".o"};

CodeGenerator::CodeGenerator(Options &o, TypeTable &t)
    : options(o), types(t), filename("output"), builder(context),
      last_value(nullptr) {
    top_symboltable = std::make_shared<SymbolTable<Value *>>(nullptr);
    current_symboltable = top_symboltable;
    types.setTypes(context);
}

void CodeGenerator::visit_ASTModule(ASTModule *ast) {
    // Set up code generation
    init(ast->name);

    // Top level consts
    top_level = true;
    doTopDecs(ast->decs.get());

    top_level = false;
    // Do procedures which generate functions first
    doProcedures(ast->procedures);

    // Set up the module as a function
    // Make the function type:  (void) : int
    // main returns int
    std::vector<llvm::Type *> proto;
    FunctionType *            ft =
        FunctionType::get(llvm::Type::getInt64Ty(context), proto, false);

    auto function_name = filename;
    if (options.main_module) {
        function_name = "main";
    }
    Function *f = Function::Create(ft, Function::ExternalLinkage, function_name,
                                   module.get());

    // Create a new basic block to start insertion into.
    BasicBlock *block = BasicBlock::Create(context, "entry", f);
    builder.SetInsertPoint(block);

    // Do declarations - vars
    top_level = true; // we have done the procedures
    ast->decs->accept(this);

    // Go through the statements
    std::for_each(ast->stats.begin(), ast->stats.end(),
                  [this](auto const &x) { x->accept(this); });

    // Validate the generated code, checking for consistency.
    verifyFunction(*f);

    // change the filename to generate module.obj
    filename = ast->name;
    if (!options.only_ll) {
        generate_objectcode();
    }
    print_code();
}

void CodeGenerator::doTopDecs(ASTDeclaration *ast) {
    if (ast->cnst) {
        doTopConsts(ast->cnst.get());
    }
    if (ast->var) {
        doTopVars(ast->var.get());
    }
}

void CodeGenerator::doTopVars(ASTVar *ast) {
    debug("CodeGenerator::doTopVars");
    for (auto const &c : ast->vars) {
        auto type = getType(c.second);
        module->getOrInsertGlobal(c.first->value, type);
        GlobalVariable *gVar = module->getNamedGlobal(c.first->value);

        gVar->setLinkage(GlobalValue::LinkageTypes::InternalLinkage);
        gVar->setInitializer(ConstantInt::get(context, APInt(64, 0, true)));

        current_symboltable->put(c.first->value, gVar);
    }
}

void CodeGenerator::doTopConsts(ASTConst *ast) {
    debug("CodeGenerator::doTopConsts");
    for (auto const &c : ast->consts) {
        auto type = getType(c.type);
        module->getOrInsertGlobal(c.ident->value, type);
        GlobalVariable *gVar = module->getNamedGlobal(c.ident->value);

        c.value->accept(this);
        gVar->setLinkage(GlobalValue::LinkageTypes::InternalLinkage);
        if (isa<ConstantInt>(last_value)) {
            gVar->setInitializer(dyn_cast<ConstantInt>(last_value));
        } else {
            throw CodeGenException("Expression based CONSTs not supported.", 0);
        }
        gVar->setConstant(true);

        current_symboltable->put(c.ident->value, gVar);
    }
}

void CodeGenerator::doProcedures(
    std::vector<std::shared_ptr<ASTProcedure>> const &procs) {
    std::for_each(procs.begin(), procs.end(),
                  [this](auto const &x) { x->accept(this); });
}

void CodeGenerator::visit_ASTDeclaration(ASTDeclaration *ast) {
    if (ast->cnst && !top_level) {
        ast->cnst->accept(this);
    }
    if (ast->var && !top_level) {
        ast->var->accept(this);
    }
}

void CodeGenerator::visit_ASTConst(ASTConst *ast) {
    debug("CodeGenerator::visit_ASTConst");
    for (auto const &c : ast->consts) {
        c.value->accept(this);
        auto val = last_value;

        auto name = c.ident->value;
        debug("create const: {}", name);

        // Create const
        auto function = builder.GetInsertBlock()->getParent();
        auto alloc = createEntryBlockAlloca(function, name, c.type);
        builder.CreateStore(val, alloc);

        current_symboltable->put(name, alloc);
    }
}

void CodeGenerator::visit_ASTVar(ASTVar *ast) {
    for (auto const &c : ast->vars) {

        // Create variable
        auto name = c.first->value;
        debug("create var: {}", name);

        auto function = builder.GetInsertBlock()->getParent();
        auto alloc = createEntryBlockAlloca(function, name, c.second);
        builder.CreateStore(ConstantInt::get(context, APInt(64, 0, true)),
                            alloc);
        alloc->setName(name);
        debug("set name: {}", name);
        current_symboltable->put(name, alloc);
    }
    debug("finish var");
}

void CodeGenerator::visit_ASTProcedure(ASTProcedure *ast) {
    debug(" CodeGenerator::visit_ASTProcedure");
    // Make the function arguments
    std::vector<llvm::Type *> proto;
    std::for_each(ast->params.begin(), ast->params.end(),
                  [this, &proto](auto const &p) {
                      debug("arg type: {}", p.second);
                      auto type = getType(p.second);
                      proto.push_back(type);
                  });

    llvm::Type *returnType;
    if (ast->return_type.empty()) {
        returnType = llvm::Type::getVoidTy(context);
    } else {
        debug("ret type: {}", ast->return_type);
        returnType = getType(ast->return_type);
    }

    FunctionType *ft = FunctionType::get(returnType, proto, false);

    Function *f = Function::Create(ft, Function::ExternalLinkage, ast->name,
                                   module.get());

    // Create a new basic block to start insertion into.
    BasicBlock *block = BasicBlock::Create(context, "entry", f);
    builder.SetInsertPoint(block);

    auto former_symboltable = current_symboltable;
    current_symboltable =
        std::make_shared<SymbolTable<Value *>>(former_symboltable);

    // Set paramater names
    unsigned i = 0;
    std::for_each(f->args().begin(), f->args().end(), [=, &i](auto &arg) {
        auto param_name = ast->params[i].first->value;
        arg.setName(param_name);
        auto type_name = ast->params[i].second;
        // Create an alloca for this variable.
        debug("set up param {}: {}.", param_name, type_name);
        AllocaInst *alloca = createEntryBlockAlloca(f, param_name, type_name);

        // Store the initial value into the alloca.
        builder.CreateStore(&arg, alloca);

        // put also into symbol table
        current_symboltable->put(param_name, alloca);
        debug("put {} into symboltable", param_name);
        i++;
    });

    // Do declarations
    ast->decs->accept(this);

    // Go through the statements
    std::for_each(ast->stats.begin(), ast->stats.end(),
                  [this](auto const &x) { x->accept(this); });

    // Validate the generated code, checking for consistency.
    verifyFunction(*f);

    current_symboltable = former_symboltable;
}

void CodeGenerator::visit_ASTAssignment(ASTAssignment *ast) {
    ast->expr->accept(this);
    auto val = last_value;

    auto var = current_symboltable->find(ast->ident->value);
    if (!var) {
        throw CodeGenException(
            fmt::format("identifier: {} not found.", ast->ident->value));
    }
    debug("CodeGenerator::visit_ASTAssignment value: {}", val->getName().str());
    builder.CreateStore(val, var.value());
}

void CodeGenerator::visit_ASTReturn(ASTReturn *ast) {
    if (ast->expr) {
        ast->expr->accept(this);
        if (top_level &&
            last_value->getType() != llvm::Type::getInt64Ty(context)) {
            // in the main module - return i64 value
            last_value =
                builder.CreateZExt(last_value, llvm::Type::getInt64Ty(context));
        }
        builder.CreateRet(last_value);
    } else {
        builder.CreateRetVoid();
    }
}

void CodeGenerator::visit_ASTCall(ASTCall *ast) {
    debug("CodeGenerator::visit_ASTCall");
    // Look up the name in the global module table.
    Function *callee;
    try {
        callee = module->getFunction(ast->name->value);
        if (!callee) {
            throw CodeGenException(
                fmt::format("function: {} not found", ast->name->value));
        }
    } catch (...) {
        debug("CodeGenerator::visit_ASTCall exception");
        throw CodeGenException(
            fmt::format("function: {} not found", ast->name->value));
    }

    std::vector<Value *> args;
    for (auto const &a : ast->args) {
        a->accept(this);
        args.push_back(last_value);
    }
    last_value = builder.CreateCall(callee, args, "calltmp");
}

void CodeGenerator::visit_ASTIf(ASTIf *ast) {
    debug("CodeGenerator::visit_ASTIf");

    // IF
    ast->if_clause.expr->accept(this);

    // Create blocks, insert then block
    auto        funct = builder.GetInsertBlock()->getParent();
    BasicBlock *then_block = BasicBlock::Create(context, "then", funct);
    BasicBlock *else_block = BasicBlock::Create(context, "else");
    std::vector<BasicBlock *> elsif_blocks;
    int                       i = 0;
    std::for_each(
        begin(ast->elsif_clause), end(ast->elsif_clause), [&](auto const &e) {
            auto e_block =
                BasicBlock::Create(context, fmt::format("elsif{}", i++));
            elsif_blocks.push_back(e_block);
        });
    BasicBlock *merge_block = BasicBlock::Create(context, "ifcont");

    if (!ast->elsif_clause.empty()) {
        builder.CreateCondBr(last_value, then_block, elsif_blocks[0]);
    } else {
        builder.CreateCondBr(last_value, then_block, else_block);
    }

    // Emit then value.
    builder.SetInsertPoint(then_block);

    std::for_each(begin(ast->if_clause.stats), end(ast->if_clause.stats),
                  [this](auto const &s) { s->accept(this); });
    builder.CreateBr(merge_block);

    // Codegen of THEN can change the current block, update then_block
    then_block = builder.GetInsertBlock();

    i = 0;
    for (auto const &e : ast->elsif_clause) {
        funct->getBasicBlockList().push_back(elsif_blocks[i]);
        builder.SetInsertPoint(elsif_blocks[i]);

        // do expr
        e.expr->accept(this);
        BasicBlock *t_block = BasicBlock::Create(context, "then", funct);

        if (&e != &ast->elsif_clause.back()) {
            builder.CreateCondBr(last_value, t_block, elsif_blocks[i + 1]);
        } else {
            // last ELSIF block - branch to else_block
            builder.CreateCondBr(last_value, t_block, else_block);
        }

        // THEN
        builder.SetInsertPoint(t_block);
        std::for_each(begin(e.stats), end(e.stats),
                      [this](auto const &s) { s->accept(this); });
        builder.CreateBr(merge_block);
        elsif_blocks[i] = builder.GetInsertBlock();
        i++;
    }

    // Emit ELSE block.
    funct->getBasicBlockList().push_back(else_block);
    builder.SetInsertPoint(else_block);

    if (ast->else_clause) {
        auto elses = *ast->else_clause;
        std::for_each(begin(elses), end(elses),
                      [this](auto const &s) { s->accept(this); });
    }

    builder.CreateBr(merge_block);
    // codegen of ELSE can change the current block, update else_block
    else_block = builder.GetInsertBlock();

    // Emit merge block.
    funct->getBasicBlockList().push_back(merge_block);
    builder.SetInsertPoint(merge_block);
}

void CodeGenerator::visit_ASTFor(ASTFor *ast) {
    debug("CodeGenerator::visit_ASTFor");

    // do start expr
    ast->start->accept(this);
    auto start_value = last_value;

    // Make the new basic block for the loop header, inserting after current
    // block.
    auto        funct = builder.GetInsertBlock()->getParent();
    BasicBlock *preheader = builder.GetInsertBlock();
    BasicBlock *loop = BasicBlock::Create(context, "loop", funct);

    auto former_symboltable = current_symboltable;
    current_symboltable =
        std::make_shared<SymbolTable<Value *>>(former_symboltable);
    auto index = createEntryBlockAlloca(funct, ast->ident->value,
                                        std::string(*types.IntType));
    builder.CreateStore(last_value, index);
    current_symboltable->put(ast->ident->value, index);

    // Insert an explicit fall through from the current block to the Loop.
    // Start insertion in LoopBB.
    builder.CreateBr(loop);
    builder.SetInsertPoint(loop);

    // DO
    std::for_each(begin(ast->stats), end(ast->stats),
                  [this](auto const &s) { s->accept(this); });

    // Emit the step value.
    Value *step;
    if (ast->by) {
        (*ast->by)->accept(this);
        step = last_value;
    } else {
        step = ConstantInt::get(context, APInt(64, 1, false));
    }
    auto   tmp = builder.CreateLoad(index);
    Value *nextVar = builder.CreateAdd(tmp, step, "nextvar");
    builder.CreateStore(nextVar, index);

    // Compute the end condition.
    ast->end->accept(this);
    auto end_value = last_value;

    // Convert condition to a bool
    Value *endCond = builder.CreateICmpSLE(nextVar, end_value, "loopcond");

    // Create the "after loop" block and insert it.
    BasicBlock *loopEnd = builder.GetInsertBlock();
    BasicBlock *after = BasicBlock::Create(context, "afterloop", funct);

    // Insert the conditional branch into the end of Loop.
    builder.CreateCondBr(endCond, loop, after);

    // Any new code will be inserted in AfterBB.
    builder.SetInsertPoint(after);

    current_symboltable = former_symboltable;
}

void CodeGenerator::visit_ASTBlock(ASTBlock *ast) {
    debug("CodeGenerator::visit_ASTBlock");

    // BEGIN
    std::for_each(begin(ast->stats), end(ast->stats),
                  [this](auto const &s) { s->accept(this); });
}

void CodeGenerator::visit_ASTWhile(ASTWhile *ast) {
    debug("CodeGenerator::visit_ASTWhile");

    // Create blocks, insert then block
    auto        funct = builder.GetInsertBlock()->getParent();
    BasicBlock *while_block = BasicBlock::Create(context, "while", funct);
    BasicBlock *loop = BasicBlock::Create(context, "loop");
    BasicBlock *end_block = BasicBlock::Create(context, "end");

    builder.CreateBr(while_block);
    builder.SetInsertPoint(while_block);

    // Expr
    ast->expr->accept(this);
    builder.CreateCondBr(last_value, loop, end_block);

    // DO
    funct->getBasicBlockList().push_back(loop);
    builder.SetInsertPoint(loop);

    std::for_each(begin(ast->stats), end(ast->stats),
                  [this](auto const &s) { s->accept(this); });
    builder.CreateBr(while_block);

    // END
    funct->getBasicBlockList().push_back(end_block);
    builder.SetInsertPoint(end_block);
}

void CodeGenerator::visit_ASTExpr(ASTExpr *ast) {
    ast->expr->accept(this);

    if (ast->relation) {
        auto L = last_value;
        (*ast->relation_expr)->accept(this);
        auto R = last_value;
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

void CodeGenerator::visit_ASTSimpleExpr(ASTSimpleExpr *ast) {
    ast->term->accept(this);
    Value *L = last_value;
    // if initial sign exists and is negative, negate the integer
    if (ast->first_sign && ast->first_sign.value() == TokenType::dash) {
        L = builder.CreateSub(ConstantInt::get(context, APInt(64, 0, true)), L,
                              "negtmp");
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
            throw CodeGenException("ASTSimpleExpr with sign" + string(t.first));
        }
        L = last_value;
    }
}

void CodeGenerator::visit_ASTTerm(ASTTerm *ast) {
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
            throw CodeGenException("ASTTerm with sign" + string(t.first));
        }

        L = last_value;
    }
}

void CodeGenerator::visit_ASTFactor(ASTFactor *ast) {
    debug("CodeGenerator::visit_ASTFactor");
    // Visit the appropriate variant
    std::visit(overloaded{[this](auto arg) {
                              debug("visit_ASTFactor: wrong ");
                              arg->accept(this);
                          },
                          [this, ast](std::shared_ptr<ASTFactor> const &arg) {
                              debug("visit_ASTFactor: not ");
                              if (ast->is_not) {
                                  debug("visit_ASTFactor: not do");
                                  arg->accept(this);
                                  last_value = builder.CreateNot(last_value);
                              }
                          }},
               ast->factor);
} // namespace ax

void CodeGenerator::visit_ASTIdentifier(ASTIdentifier *ast) {
    if (auto res = current_symboltable->find(ast->value)) {
        last_value = builder.CreateLoad(*res, ast->value);
        return;
    }
    throw CodeGenException(fmt::format("identifier {} unknown", ast->value));
}

void CodeGenerator::visit_ASTInteger(ASTInteger *ast) {
    last_value = ConstantInt::get(context, APInt(64, ast->value, true));
}

void CodeGenerator::visit_ASTBool(ASTBool *ast) {
    last_value = ConstantInt::get(
        context, APInt(1, static_cast<uint64_t>(ast->value), true));
};

/**
 * @brief Create an alloca instruction in the entry block of the function.
 * This is used for mutable variables etc.
 *
 * @param function
 * @param name
 * @return AllocaInst*
 */
AllocaInst *CodeGenerator::createEntryBlockAlloca(Function *         function,
                                                  std::string const &name,
                                                  std::string const &type) {
    IRBuilder<> TmpB(&function->getEntryBlock(),
                     function->getEntryBlock().begin());
    auto        t = getType(type);
    return TmpB.CreateAlloca(t, nullptr, name);
}

llvm::Type *CodeGenerator::getType(std::string const &t) {
    if (auto type = types.find(t); *type) {
        return (*type)->get_llvm();
    }
    throw CodeGenException("Type not found: " + t, 0);
}

void CodeGenerator::init(std::string const &module_name) {
    module = std::make_unique<Module>(module_name, context);
    module->setSourceFileName(module_name);
}

void CodeGenerator::print_code() {
    debug("CodeGenerator::print_code");
    auto            f = filename + file_ext_llvmri;
    std::error_code EC;
    raw_fd_ostream  out_file(f, EC, sys::fs::OF_None);

    if (EC) {
        throw CodeGenException("Could not open file: " + EC.message(), 0);
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
    auto        target = TargetRegistry::lookupTarget(targetTriple, error);

    // Print an error and exit if we couldn't find the requested target.
    // This generally occurs if we've forgotten to initialise the
    // TargetRegistry or we have a bogus target triple.
    if (target == nullptr) {
        throw CodeGenException(error, 0);
    }

    // Use generic CPU without features
    auto CPU = "generic";
    auto features = "";

    TargetOptions opt;
    auto          RM = Optional<Reloc::Model>();
    auto          targetMachine =
        target->createTargetMachine(targetTriple, CPU, features, opt, RM);

    module->setDataLayout(targetMachine->createDataLayout());
    module->setTargetTriple(targetTriple);

    auto            f = filename + file_ext_obj;
    std::error_code EC;
    raw_fd_ostream  dest_file(f, EC, sys::fs::OF_None);

    if (EC) {
        throw CodeGenException("Could not open file: " + EC.message(), 0);
    }

    legacy::PassManager pass;
    auto                file_type = LLVMTargetMachine::CGFT_ObjectFile;

    if (targetMachine->addPassesToEmitFile(pass, dest_file, nullptr,
                                           file_type)) {
        throw CodeGenException("TargetMachine can't emit a file of this type",
                               0);
    }
    pass.run(*module);
    dest_file.flush();
}

} // namespace ax