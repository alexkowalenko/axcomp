//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "codegen.hh"

#include <exception>
#include <format>
#include <memory>

#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>

#include "ast.hh"
#include "astvisitor.hh"
#include "builtin.hh"
#include "error.hh"
#include "symbol.hh"
#include "symboltable.hh"
#include "token.hh"
#include "type.hh"
#include "typetable.hh"

namespace ax {

// this has to be a #define for it work properly
#define DEBUG_TYPE "codegen"

template <typename S, typename... Args> static void debug(const S &format, const Args &...msg) {
    LLVM_DEBUG(llvm::dbgs() << DEBUG_TYPE << ' '
                            << std::vformat(format, std::make_format_args(msg...))
                            << '\n'); // NOLINT
}

using namespace llvm::sys;

constexpr auto file_ext_llvmri{".ll"};
constexpr auto file_ext_obj{".o"};

CodeGenerator::CodeGenerator(Options &o, SymbolFrameTable &s, TypeTable &t, Importer &i)
    : options{o}, symboltable{s}, types{t}, importer{i}, filename("main"), builder(context) {
    debug("CodeGenerator::CodeGenerator");
    TypeTable::setTypes(context);
}

void CodeGenerator::visit(ASTModule const &ast) {
    // Set up code generation
    module_name = ast->name;
    init();

    // Set up builtins
    setup_builtins();

    if (ast->import) {
        visit(ast->import);
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
        function_name = ASTQualident_::make_coded_id(ast->name, "main");
    }
    Function *f = Function::Create(ft, Function::ExternalLinkage, function_name, module.get());

    // Create a new basic block to start insertion into.
    BasicBlock *block = BasicBlock::Create(context, "entry", f);
    builder.SetInsertPoint(block);

    // Do declarations - vars
    top_level = true; // we have done the procedures
    visit(ast->decs);

    for (auto const &stat : ast->stats) {
        stat->accept(this);
    }
    block = builder.GetInsertBlock();
    if (block->back().getOpcode() != llvm::Instruction::Ret) {
        builder.CreateRet(TypeTable::IntType->get_init());
    }

    // Validate the generated code, checking for consistency.
    verifyFunction(*f);

    // change the filename to generate module.obj
    filename = ast->name;
}

void CodeGenerator::visit(ASTImport const &ast) {
    debug("ASTImport");

    for (auto const &import : ast->imports) {
        SymbolFrameTable symbols;
        const auto       found = importer.find_module(import.first->value, symbols, types);
        assert(found); // NOLINT

        debug("ASTImport do {0}", import.first->value);

        for (auto &symbol : symbols) {
            auto name = std::string(symbol.first);
            auto type = symbol.second->type;
            debug("ASTImport get {0} : {1}", name, type->get_name());

            if (type->is_referencable()) {
                GlobalVariable *gVar = generate_global(name, type->get_llvm());
                debug("ASTImport var {0}", name);
                symboltable.set_value(name, gVar, Attr::global);
            } else if (type->id == TypeId::procedure) {
                debug("ASTImport proc {0}", name);
                if (auto res = symboltable.find(name); res->is(Attr::used)) {
                    auto *funcType = (FunctionType *)type->get_llvm();

                    auto *func = Function::Create(
                        funcType, Function::LinkageTypes::ExternalLinkage, name, module.get());
                    verifyFunction(*func);
                    symboltable.set_value(name, func);
                    debug("ASTImport proc {0} set ", name);
                }
            } else {
                assert(false);
            }
        }
    }
}

void CodeGenerator::doTopDecs(ASTDeclaration const &ast) {
    if (ast->cnst) {
        doTopConsts(ast->cnst);
    }
    if (ast->var) {
        doTopVars(ast->var);
    }
}

void CodeGenerator::doTopVars(ASTVar const &ast) {
    debug("doTopVars");
    for (auto const &c : ast->vars) {
        debug("doTopVars {0}: {1}", c.first->value, c.second->get_type()->get_name());

        llvm::Type     *type = getType(c.second);
        auto            var_name = gen_module_id(c.first->value);
        GlobalVariable *gVar = generate_global(var_name, type);

        GlobalValue::LinkageTypes linkage = GlobalValue::LinkageTypes::InternalLinkage;
        if (c.first->is(Attr::global)) {
            linkage = GlobalValue::LinkageTypes::ExternalLinkage;
        }
        gVar->setLinkage(linkage);
        auto *init = getType_init(c.second);
        gVar->setInitializer(init);
        symboltable.set_value(c.first->value, gVar);
    }
}

void CodeGenerator::doTopConsts(ASTConst const &ast) {
    debug("doTopConsts");
    for (auto const &c : ast->consts) {
        debug("doTopConsts type: {0}", c.type->get_type()->get_name());
        auto           *type = getType(c.type);
        auto            const_name = gen_module_id(c.ident->value);
        GlobalVariable *gVar = generate_global(const_name, type);

        visit(c.value);
        GlobalValue::LinkageTypes linkage = GlobalValue::LinkageTypes::InternalLinkage;
        if (c.ident->is(Attr::global)) {
            linkage = GlobalValue::LinkageTypes::ExternalLinkage;
        }
        gVar->setLinkage(linkage);
        if (isa<Constant>(last_value)) {
            gVar->setInitializer(dyn_cast<Constant>(last_value));
        } else {
            throw CodeGenException(ast->get_location(), "Expression based CONSTs not supported.");
        }
        gVar->setConstant(true);
        symboltable.set_value(c.ident->value, gVar);
    }
}

void CodeGenerator::doProcedures(std::vector<ASTProc> const &procs) {
    for (auto const &proc : procs) {
        proc->accept(this);
    }
}

void CodeGenerator::visit(ASTDeclaration const &ast) {
    if (ast->cnst && !top_level) {
        visit(ast->cnst);
    }
    if (ast->var && !top_level) {
        visit(ast->var);
    }
}

void CodeGenerator::visit(ASTConst const &ast) {
    debug("ASTConst");
    for (auto const &c : ast->consts) {
        visit(c.value);
        auto *val = last_value;

        auto name = c.ident->value; // consts within procedures don't need
                                    // to be renamed
        debug("create const: {}", name);

        // Create const
        auto *function = builder.GetInsertBlock()->getParent();
        auto *alloc = createEntryBlockAlloca(function, name, c.type);
        builder.CreateStore(val, alloc);
        symboltable.set_value(name, alloc);
    }
}

void CodeGenerator::visit(ASTVar const &ast) {
    debug("ASTVar");
    for (auto const &c : ast->vars) {

        // Create variable
        auto name = c.first->value;
        debug("create var: {}", name);

        auto       *function = builder.GetInsertBlock()->getParent();
        auto        type = c.second;
        AllocaInst *alloc = createEntryBlockAlloca(function, name, type);
        auto       *init = getType_init(type);
        builder.CreateStore(init, alloc);

        alloc->setName(name);
        debug("set name: {0}", name);
        symboltable.set_value(name, alloc);
    }
    debug("finish var");
}

void CodeGenerator::visit(ASTProcedure const &ast) {
    debug("ASTProcedure {0}", ast->name->value);

    nested_procs.push_back(ast->name->value);
    auto sym = symboltable.find(ast->name->value);
    auto funct_type = std::dynamic_pointer_cast<ProcedureType>(sym->type);

    // Make the function arguments
    std::vector<llvm::Type *>                              proto;
    std::vector<std::pair<int, llvm::Attribute::AttrKind>> argAttr;
    auto                                                   index{0};

    // Do receiver first
    if (ast->receiver.first) {
        auto [name, typeName] = ast->receiver;
        auto *type = typeName->get_type()->get_llvm();
        if (name->is(Attr::var)) {
            type = llvm::PointerType::get(context, 0);
        }
        proto.push_back(type);
    }

    for (auto const &[var, t_type] : ast->params) {
        auto             *type = getType(t_type);
        llvm::AttrBuilder attrs(context);
        index++;
        if (index == 1 && sym->is(Attr::closure)) {
            debug("ASTProcedure {0} is closure function", ast->name->value);
            argAttr.emplace_back(index, llvm::Attribute::Nest);
            proto.push_back(llvm::PointerType::get(context, 0));
            continue;
        }
        if (var->is(Attr::var)) {
            debug(" CodeGenerator::visit VAR {0}", var->value);
            type = llvm::PointerType::get(context, 0);

        } else {
            // Todo: Switch on later
            // argAttr.emplace_back(index, llvm::Attribute::ByVal);
        }
        proto.push_back(type);
    };

    llvm::Type *returnType = nullptr;
    if (ast->return_type == nullptr) {
        returnType = llvm::Type::getVoidTy(context);
    } else {
        returnType = getType(ast->return_type);
    }

    auto                      proc_name = gen_module_id(get_nested_name());
    FunctionType             *ft = FunctionType::get(returnType, proto, false);
    GlobalValue::LinkageTypes linkage = Function::InternalLinkage;
    if (ast->name->is(Attr::global)) {
        linkage = Function::ExternalLinkage;
    }
    Function *f = module->getFunction(proc_name);
    if (!f) {
        f = Function::Create(ft, linkage, proc_name, module.get());
    }
    for (auto const &[_, attr] : argAttr) {
        f->addFnAttr(attr);
    }

    // Create a new basic block to start insertion into.
    BasicBlock *block = BasicBlock::Create(context, "entry", f);
    builder.SetInsertPoint(block);

    // Push new frame
    debug("ASTProcedure push frame {0}", get_nested_name());
    symboltable.push_frame(ast->name->value);

    // Set parameter names

    if (ast->receiver.first) {
        auto [name, typeName] = ast->receiver;
        auto *type = typeName->get_type()->get_llvm();
        if (name->is(Attr::var)) {
            type = llvm::PointerType::get(context, 0);
        }
        proto.push_back(type);
    }

    unsigned i = 0;
    bool     do_receiver{ast->receiver.first != nullptr};

    for (auto &arg : f->args()) {
        debug("ASTProcedure process parameter {0}", i);
        if (i == 0 && sym->is(Attr::closure)) {
            // skip closure variable
            i++;
            continue;
        }

        std::string param_name;
        AllocaInst *alloca{nullptr};
        Attr        attr = Attr::null;
        if (i == 0 && do_receiver) {
            auto [name, typeName] = ast->receiver;
            param_name = name->value;
            auto *type = typeName->get_type()->get_llvm();
            if (name->is(Attr::var)) {
                type = llvm::PointerType::get(context, 0);
                attr = Attr::var;
            }
            arg.setName(name->value);
            alloca = createEntryBlockAlloca(f, name->value, type);
            i--; // go back
            do_receiver = false;
        } else {
            param_name = ast->params[i].first->value;
            auto type_name = ast->params[i].second;
            attr = Attr::null;
            if (ast->params[i].first->is(Attr::var)) {
                attr = Attr::var;
            }
            alloca = createEntryBlockAlloca(f, param_name, type_name, attr == Attr::var);
        }
        arg.setName(param_name);

        // Store the initial value into the alloca.
        builder.CreateStore(&arg, alloca);

        // put also into symbol table
        symboltable.set_value(param_name, alloca, attr);
        debug("put {0} into symboltable", param_name);
        i++;
    };

    // Do declarations
    visit(ast->decs);

    // Do closure variables
    if (sym->is(Attr::closure)) {
        // Set closure variables
        // from lacsap PrototypeAST::CreateArgumentAlloca
        debug("ASTProcedure set up closure variables", ast->name->value);

        std::vector<llvm::Value *> ind = {TypeTable::IntType->make_value(0), nullptr};
        unsigned                   i = 0;
        auto                      *cls_arg = f->arg_begin();
        for (auto const &[cls_var, cls_type] :
             std::dynamic_pointer_cast<ProcedureType>(sym->type)->free_vars) {
            ind[1] = TypeTable::IntType->make_value(i);

            llvm::Value *a = builder.CreateGEP(cls_arg->getType(), &*cls_arg, ind, cls_var);
            a = builder.CreateLoad(a->getType(), a, cls_var);

            // put into symbol table
            // * this is meant to shadow the variable in the outer scope otherwise the
            // outer scope
            // * variable changes.
            // debug("ASTProcedure set_value {0} : {1}", cls_arg->getName(),
            // a->getName());
            auto sym = mkSym(cls_type);
            sym->value = a;
            symboltable.put(cls_var, sym);
            i++;
        }
    }

    for (auto const &proc : ast->procedures) {
        proc->accept(this);
    }

    builder.SetInsertPoint(block);

    // for recursion
    debug("ASTProcedure set function {0}", ast->name->value);
    symboltable.set_value(ast->name->value, f);

    for (auto const &stat : ast->stats) {
        stat->accept(this);
    }
    block = builder.GetInsertBlock();
    if (block->back().getOpcode() != llvm::Instruction::Ret) {
        if (ast->return_type == nullptr) {
            builder.CreateRetVoid();
        } else {
            builder.CreateRet(last_value);
        }
    }
    symboltable.pop_frame();
    debug("ASTProcedure pop frame {0}", get_nested_name());
    // set function in outer scope, incase function name identical in outer and
    // inner scope
    symboltable.set_value(ast->name->value, f);
    nested_procs.pop_back();
    // Validate the generated code, checking for consistency.
    verifyFunction(*f);
}

void CodeGenerator::visit(ASTProcedureForward const &ast) {

    auto  res = symboltable.find(ast->name->value);
    auto *funcType = dyn_cast<FunctionType>(res->type->get_llvm());

    auto  proc_name = gen_module_id(ast->name->value);
    auto *func = Function::Create(funcType, Function::LinkageTypes::InternalLinkage, proc_name,
                                  module.get());
    verifyFunction(*func);
    symboltable.set_value(ast->name->value, func);
}

void CodeGenerator::visit(ASTAssignment const &ast) {
    debug("ASTAssignment {0}", std::string(*(ast->ident)));
    visit(ast->expr);
    auto *val = last_value;

    bool var = false;
    if (auto res = symboltable.find(ast->ident->ident->id->value); res) {
        if (res->is(Attr::var)) {
            var = true;
        }
    }
    debug("ASTAssignment VAR {0}", var);
    if (var) {
        // Handle VAR assignment
        is_var = true; // Set change in visitPtr to notify
                       // this is a write of a VAR variable
        visitPtr(ast->ident, false);
    } else {
        visitPtr(ast->ident, true);
    }
    // debug("ASTAssignment store {0} in {1}", val->getName(),
    // last_value->getName());

    // Do LLIR Type adjustments for the final assignment
    // LLVM has opaque pointer types
    // if (auto *ty = dyn_cast<llvm::PointerType>(last_value->getType()); ty) {
    //     auto *base_type = ty->getElementType(); // extract base type
    //     if (val->getType() != base_type) {
    //         val = builder.CreateBitCast(val, base_type);
    //     }
    // }
    builder.CreateStore(val, last_value);
    is_var = false;
}

void CodeGenerator::visit(ASTReturn const &ast) {
    if (ast->expr) {
        visit(ast->expr);
        if (top_level && last_value->getType() != llvm::Type::getInt64Ty(context)) {
            // in the main module - return i64 value
            last_value = builder.CreateZExt(last_value, llvm::Type::getInt64Ty(context));
        }
        builder.CreateRet(last_value);
    } else {
        builder.CreateRetVoid();
    }
}

void CodeGenerator::visit(ASTExit const &ast) {
    debug("ASTExit");
    if (last_end) {
        builder.CreateBr(last_end);
    } else {
        throw CodeGenException(ast->get_location(), "EXIT: no enclosing loop.");
    }
}

std::tuple<std::shared_ptr<ProcedureType>, std::string, bool>
CodeGenerator::do_find_proc(ASTCall const &ast) {

    // Look up the name in the global module table.

    auto name = ast->name->ident->make_coded_id();
    auto res = symboltable.find(name);
    bool bound_proc{false};

    auto typeFunction = std::dynamic_pointer_cast<ProcedureType>(res->type);
    if (!typeFunction) {
        // check bound procedure
        auto field = ast->name->first_field();
        name = field->value;
        res = symboltable.find(name);
        typeFunction = std::dynamic_pointer_cast<ProcedureType>(res->type);
        bound_proc = true;
    }
    return std::make_tuple(typeFunction, name, bound_proc);
}

std::vector<Value *> CodeGenerator::do_arguments(ASTCall const &ast) {

    auto [typeFunction, _, bound_proc] = do_find_proc(ast);

    std::vector<Value *> args;
    if (bound_proc) {
        auto varname = ast->name->ident->id->value;
        debug("do_arguments receiver {0}", varname);
        auto var = symboltable.find(varname);
        // llvm::dbgs() << *var->value << '\n';
        last_value = var->value;
        debug("do_arguments receiver {0} send {1}", string(typeFunction->receiver),
              string(ast->name->ident->id->get_type()));
        if (typeFunction->receiver_type != Attr::var) {
            last_value = builder.CreateLoad(last_value->getType(), last_value);
        }

        args.push_back(last_value);

        // llvm::dbgs() << *last_value << '\n';
    }
    auto i = 0;
    for (auto const &a : ast->args) {
        if (typeFunction->params[i].second == Attr::var) {
            debug("ASTCall VAR parameter");
            // Var Parameter

            // Get Identifier, get pointer set to last_value
            // visitPtr
            // This works, since the Inspector checks that VAR arguments are
            // only identifiers
            auto ptr = a->expr->term->factor->factor;
            auto p2 = std::get<ASTDesignator>(ptr)->ident;

            debug("ASTCall identifier {0}", p2->id->value);
            visitPtr(p2->id, true);
        } else {
            // Reference Parameter

            // Check for STRING1 to CHAR conversion
            do_strchar_conv = a->get_type()->id == TypeId::str1 &&
                              (typeFunction->params[i].first &&
                               typeFunction->params[i].first->id == TypeId::chr);
            visit(a);
        }
        args.push_back(last_value);
        i++;
    }
    return args;
}

void CodeGenerator::visit(ASTCall const &ast) {
    auto name = ast->name->ident->make_coded_id();
    debug("ASTCall: {0}", name);
    auto res = symboltable.find(name);

    if (res->is(Attr::compile_function)) {
        debug("ASTCall: compile {0}", name);
        auto &f = Builtin::compile_functions[name];
        last_value = f(this, ast);
        return;
    }
    debug("ASTCall: global {0}", name);
    auto args = do_arguments(ast);
    assert(res->value && "did not find function to call!"); // NOLINT
    auto [callee_type, fname, _] = do_find_proc(ast);
    res = symboltable.find(fname);
    callee_type = std::dynamic_pointer_cast<ProcedureType>(res->type);
    auto *callee = llvm::dyn_cast<Function>(res->value);
    if (res->is(Attr::closure)) {
        debug("ASTCall: {0} call closure", name);

        // From lacsap ClosureAST::CodeGen

        auto                      *cls_ty = callee_type->get_closure_struct()->get_llvm();
        std::vector<llvm::Value *> ind = {TypeTable::IntType->make_value(0), nullptr};
        auto                      *current_func = builder.GetInsertBlock()->getParent();
        llvm::Value *closure = createEntryBlockAlloca(current_func, "closure_struct", cls_ty);
        int          index = 0;
        for (auto const &[name, f_ty] : callee_type->free_vars) {
            auto         r = symboltable.find(name);
            llvm::Value *v = r->value;
            ind[1] = TypeTable::IntType->make_value(index);
            llvm::Value *ptr = builder.CreateGEP(closure->getType(), closure, ind, "cls");
            // debug("ASTCall closure call {0} : {1}", v->getName(), name);
            builder.CreateStore(v, ptr);
            index++;
        }
        args.insert(args.begin(), closure);
    }
    auto *inst = builder.CreateCall(callee, args);
    if (res->is(Attr::closure)) {
        inst->addAttributeAtIndex(1, llvm::Attribute::Nest);
    }
    last_value = inst;
}

void CodeGenerator::visit(ASTIf const &ast) {
    debug("ASTIf");

    // IF
    visit(ast->if_clause.expr);

    // Create blocks, insert then block
    auto                     *funct = builder.GetInsertBlock()->getParent();
    BasicBlock               *then_block = BasicBlock::Create(context, "then", funct);
    BasicBlock               *else_block = BasicBlock::Create(context, "else");
    std::vector<BasicBlock *> elsif_blocks;
    if (!ast->elsif_clause.empty()) {
        for (size_t idx = 0; idx < ast->elsif_clause.size(); ++idx) {
            auto *e_block = BasicBlock::Create(context, std::format("elsif{0}", idx));
            elsif_blocks.push_back(e_block);
        }
    }
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

    for (auto const &stmt : ast->if_clause.stats) {
        stmt->accept(this);
    }
    then_block = builder.GetInsertBlock(); // necessary for correct generation of code
    ejectBranch(ast->if_clause.stats, then_block, merge_block);

    int i = 0;
    for (auto const &e : ast->elsif_clause) {

        // ELSEIF
        funct->insert(funct->end(), elsif_blocks[i]);
        builder.SetInsertPoint(elsif_blocks[i]);

        // do expr
        visit(e.expr);
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
        for (auto const &stmt : e.stats) {
            stmt->accept(this);
        }
        t_block = builder.GetInsertBlock(); // necessary for correct generation of code
        ejectBranch(e.stats, t_block, merge_block);
        i++;
    }

    // Emit ELSE block.

    if (ast->else_clause) {
        funct->insert(funct->end(), else_block);
        builder.SetInsertPoint(else_block);
        auto elses = *ast->else_clause;
        for (auto const &stmt : elses) {
            stmt->accept(this);
        }
        else_block = builder.GetInsertBlock(); // necessary for correct generation of code
        ejectBranch(elses, else_block, merge_block);
    }

    // codegen of ELSE can change the current block, update else_block
    else_block = builder.GetInsertBlock(); // NOLINT

    // Emit merge block.
    funct->insert(funct->end(), merge_block);
    builder.SetInsertPoint(merge_block);
}

void CodeGenerator::visit(ASTCase const &ast) {
    debug("ASTCase");

    // Create blocks, insert then block
    auto                     *funct = builder.GetInsertBlock()->getParent();
    BasicBlock               *else_block = nullptr;
    std::vector<BasicBlock *> element_blocks;
    for (size_t idx = 0; idx < ast->elements.size(); ++idx) {
        auto *block = BasicBlock::Create(context, std::format("case.element{0}", idx));
        element_blocks.push_back(block);
    }

    if (!ast->else_stats.empty()) {
        else_block = BasicBlock::Create(context, "case.else");
    }
    BasicBlock *range_block = BasicBlock::Create(context, "case.range");
    BasicBlock *end_block = BasicBlock::Create(context, "case.end");

    // CASE
    visit(ast->expr);
    auto *case_value = last_value;

    std::vector<std::pair<ASTRange, int>> range_list;

    // CASE elements
    auto *switch_inst = builder.CreateSwitch(case_value, range_block);

    int i = 0;
    for (auto const &element : ast->elements) {
        debug("ASTCase {0}", i);
        for (auto &expr : element->exprs) {
            if (std::holds_alternative<ASTSimpleExpr>(expr)) {
                debug("ASTCase {0} expr", i);
                std::get<ASTSimpleExpr>(expr)->accept(this);
                assert(llvm::dyn_cast<llvm::ConstantInt>(last_value));
                switch_inst->addCase(llvm::dyn_cast<llvm::ConstantInt>(last_value),
                                     element_blocks[i]);
            } else if (std::holds_alternative<ASTRange>(expr)) {
                debug("ASTCase {0} range", i);
                auto range = std::get<ASTRange>(expr);
                range_list.push_back({range, i});
            }
        }
        i++;
    }

    funct->insert(funct->end(), range_block);
    builder.SetInsertPoint(range_block);

    // Go through Range values
    if (!range_list.empty()) {
        std::vector<BasicBlock *> range_blocks;
        for (size_t idx = 0; idx < range_list.size(); ++idx) {
            auto *block = BasicBlock::Create(context, std::format("case.range{0}", idx));
            range_blocks.push_back(block);
        }
        i = 0;
        builder.CreateBr(range_blocks[0]);
        for (auto &p : range_list) {
            funct->insert(funct->end(), range_blocks[i]);
            builder.SetInsertPoint(range_blocks[i]);
            visit_value(p.first, case_value);

            BasicBlock *next = nullptr;
            if (i == range_list.size() - 1) {
                if (!ast->else_stats.empty()) {
                    next = else_block;
                } else {
                    next = end_block;
                }
            } else {
                next = range_blocks[i + 1];
            }
            builder.CreateCondBr(last_value, element_blocks[p.second], next);
            i++;
        }
    } else {
        BasicBlock *next = nullptr;
        if (!ast->else_stats.empty()) {
            next = else_block;
        } else {
            next = end_block;
        }
        builder.CreateBr(next);
    }

    i = 0;
    for (auto const &element : ast->elements) {
        debug("ASTCase element {0}", i);

        funct->insert(funct->end(), element_blocks[i]);
        builder.SetInsertPoint(element_blocks[i]);
        for (auto const &stmt : element->stats) {
            stmt->accept(this);
        }
        // check if last instruction is branch (EXIT)
        if (element_blocks[i]->back().getOpcode() != llvm::Instruction::Br) {
            builder.CreateBr(end_block);
        }
        element_blocks[i] = builder.GetInsertBlock(); // necessary for correct generation of code
        i++;
    }

    // ELSE
    if (!ast->else_stats.empty()) {
        funct->insert(funct->end(), else_block);
        builder.SetInsertPoint(else_block);
        for (auto const &stmt : ast->else_stats) {
            stmt->accept(this);
        }

        builder.CreateBr(end_block);
        else_block = builder.GetInsertBlock(); // necessary for correct generation
                                               // of code NOLINT
    }

    // END
    funct->insert(funct->end(), end_block);
    builder.SetInsertPoint(end_block);
    end_block = builder.GetInsertBlock(); // necessary for correct generation of code NOLINT
}

void CodeGenerator::visit(ASTFor const &ast) {
    debug("ASTFor");

    // do start expr
    visit(ast->start);
    auto *start_value = last_value;

    // Make the new basic block for the loop header, inserting after current
    // block.
    auto       *funct = builder.GetInsertBlock()->getParent();
    BasicBlock *loop = BasicBlock::Create(context, "loop", funct);
    BasicBlock *forpos = BasicBlock::Create(context, "forpos", funct);
    BasicBlock *forneg = BasicBlock::Create(context, "forneg", funct);
    BasicBlock *after = BasicBlock::Create(context, "afterloop", funct);
    last_end = after;

    debug("index: {}", ast->ident->value);
    auto *index = symboltable.find(ast->ident->value)->value;
    builder.CreateStore(start_value, index);

    // Insert an explicit fall through from the current block to the Loop.
    // Start insertion in LoopBB.
    builder.CreateBr(loop);
    builder.SetInsertPoint(loop);

    // DO
    for (auto const &stat : ast->stats) {
        stat->accept(this);
    }

    // Emit the step value.
    Value *step = nullptr;
    if (ast->by) {
        visit(ast->by);
        step = last_value;
    } else {
        step = TypeTable::IntType->make_value(1);
    }
    auto  *tmp = builder.CreateLoad(TypeTable::IntType->get_llvm(), index, "tmp");
    Value *nextVar = builder.CreateAdd(tmp, step, "nextvar");
    builder.CreateStore(nextVar, index);

    debug("Compute the end condition.");
    visit(ast->end);
    auto *end_value = last_value;

    debug("check step");
    auto *cond = builder.CreateICmpSGE(step, TypeTable::IntType->get_init());
    builder.CreateCondBr(cond, forpos, forneg);

    debug("Step is positive");
    if (llvm::isCurrentDebugType(DEBUG_TYPE) && options.debug) {
        llvm::dbgs() << "index: ";
        index->getType()->print(llvm::dbgs());
        llvm::dbgs() << "\ntmp: ";
        tmp->getType()->print(llvm::dbgs());
        llvm::dbgs() << "\nnextVar: ";
        nextVar->getType()->print(llvm::dbgs());
        llvm::dbgs() << "\nend_value: ";
        end_value->getType()->print(llvm::dbgs());
    }

    builder.SetInsertPoint(forpos);
    auto *nextVar_value = builder.CreateLoad(TypeTable::IntType->get_llvm(), index, "nextvar");
    if (llvm::isCurrentDebugType(DEBUG_TYPE) && options.debug) {
        llvm::dbgs() << "\nnextVar_value: ";
        nextVar_value->getType()->print(llvm::dbgs());
        llvm::dbgs() << "\n";
    }

    Value *endCond = builder.CreateICmpSLE(nextVar_value, end_value, "loopcond");

    debug("Insert the conditional branch into the end of Loop.");
    builder.CreateCondBr(endCond, loop, after);

    // Step is negative
    builder.SetInsertPoint(forneg);
    nextVar_value = builder.CreateLoad(TypeTable::IntType->get_llvm(), index, "nextvar");
    endCond = builder.CreateICmpSGE(nextVar_value, end_value, "loopcond");

    // Insert the conditional branch into the end of Loop.
    builder.CreateCondBr(endCond, loop, after);

    // Any new code will be inserted in AfterBB.
    builder.SetInsertPoint(after);

    // debug("ASTFor after:{0}", last_end);
}

void CodeGenerator::visit(ASTWhile const &ast) {
    debug("ASTWhile");

    // Create blocks
    auto       *funct = builder.GetInsertBlock()->getParent();
    BasicBlock *while_block = BasicBlock::Create(context, "while", funct);
    BasicBlock *loop = BasicBlock::Create(context, "loop");
    BasicBlock *end_block = BasicBlock::Create(context, "end");
    last_end = end_block;

    builder.CreateBr(while_block);
    builder.SetInsertPoint(while_block);

    // Expr
    visit(ast->expr);
    builder.CreateCondBr(last_value, loop, end_block);
    while_block = builder.GetInsertBlock();

    // DO
    funct->insert(funct->end(), loop);
    builder.SetInsertPoint(loop);

    for (auto const &stat : ast->stats) {
        stat->accept(this);
    }

    loop = builder.GetInsertBlock(); // necessary for correct generation of code
    ejectBranch(ast->stats, loop, while_block);

    // END
    funct->insert(funct->end(), end_block);
    builder.SetInsertPoint(end_block);
}

void CodeGenerator::visit(ASTRepeat const &ast) {
    debug("ASTRepeat");

    // Create blocks
    auto       *funct = builder.GetInsertBlock()->getParent();
    BasicBlock *repeat_block = BasicBlock::Create(context, "repeat", funct);
    BasicBlock *end_block = BasicBlock::Create(context, "end");
    last_end = end_block;

    // REPEAT
    builder.CreateBr(repeat_block);
    builder.SetInsertPoint(repeat_block);

    for (auto const &stat : ast->stats) {
        stat->accept(this);
    }

    // Expr
    visit(ast->expr);
    builder.CreateCondBr(last_value, end_block, repeat_block);
    repeat_block = builder.GetInsertBlock(); // necessary for correct generation of code NOLINT

    // END
    funct->insert(funct->end(), end_block);
    builder.SetInsertPoint(end_block);
}

void CodeGenerator::visit(ASTLoop const &ast) {
    debug("ASTLoop");

    // Create blocks
    auto       *funct = builder.GetInsertBlock()->getParent();
    BasicBlock *loop_block = BasicBlock::Create(context, "loop", funct);
    BasicBlock *end_block = BasicBlock::Create(context, "end");
    last_end = end_block;

    // LOOP
    builder.CreateBr(loop_block);
    builder.SetInsertPoint(loop_block);

    for (auto const &stat : ast->stats) {
        stat->accept(this);
    }
    builder.CreateBr(loop_block);
    loop_block = builder.GetInsertBlock(); // necessary for correct generation of code NOLINT

    // END
    funct->insert(funct->end(), end_block);
    builder.SetInsertPoint(end_block);
    // end_block = builder.GetInsertBlock();
}

void CodeGenerator::visit(ASTBlock const &ast) {
    debug("ASTBlock");

    // Create blocks
    auto       *funct = builder.GetInsertBlock()->getParent();
    BasicBlock *begin_block = BasicBlock::Create(context, "begin", funct);
    BasicBlock *end_block = BasicBlock::Create(context, "end");
    last_end = end_block;

    // BEGIN
    builder.CreateBr(begin_block);
    builder.SetInsertPoint(begin_block);

    // BEGIN
    for (auto const &stat : ast->stats) {
        stat->accept(this);
    }

    builder.CreateBr(end_block);
    begin_block = builder.GetInsertBlock(); // necessary for correct generation of code NOLINT

    // END
    funct->insert(funct->end(), end_block);
    builder.SetInsertPoint(end_block);
}

void CodeGenerator::visit(ASTExpr const &ast) {
    debug("ASTExpr");
    visit(ast->expr);

    if (ast->relation) {
        auto *L = last_value;
        visit(ast->relation_expr);
        auto *R = last_value;
        if (L->getType() == TypeTable::StrType->get_llvm() &&
            R->getType() == TypeTable::StrType->get_llvm()) {
            // String comparisons
            last_value = call_function("Strings_Compare", TypeTable::IntType->get_llvm(), {L, R});
            switch (*ast->relation) {
            case TokenType::equals:
                last_value = builder.CreateNot(last_value);
                break;
            case TokenType::hash:
                // Correct
                break;
            case TokenType::less:
                last_value = builder.CreateICmpSLT(
                    last_value, TypeTable::IntType->get_init()); // last_value > 0
                break;
            case TokenType::leq:
                last_value = builder.CreateICmpSLE(last_value, TypeTable::IntType->get_init());
                break;
            case TokenType::greater:
                last_value = builder.CreateICmpSGT(last_value, TypeTable::IntType->get_init());
                break;
            case TokenType::gteq:
                last_value = builder.CreateICmpSGE(last_value, TypeTable::IntType->get_init());
                break;
            default:;
            }
        } else if (ast->expr->get_type()->id == TypeId::pointer &&
                   ast->relation_expr->get_type()->id == TypeId::null) {
            // Pointer comparisons
            R = builder.CreateBitCast(R, L->getType());
            switch (*ast->relation) {
            case TokenType::equals:
                last_value = builder.CreateICmpEQ(L, R);
                break;
            case TokenType::hash:
                last_value = builder.CreateICmpNE(L, R);
                break;
            default:;
            }
        } else if (ast->relation_expr->get_type() == TypeTable::SetType) {
            // SET comparisons
            debug("ASTExpr set comprisons");
            switch (*ast->relation) {
            case TokenType::in: {
                auto *index = builder.CreateShl(TypeTable::IntType->make_value(1), L);
                last_value = builder.CreateAnd(R, index);
                last_value = builder.CreateICmpUGT(last_value, TypeTable::IntType->get_init());
                break;
            }
            case TokenType::equals:
                last_value = builder.CreateICmpEQ(L, R);
                break;
            case TokenType::hash:
                last_value = builder.CreateICmpNE(L, R);
                break;
            default:;
            }

        } else if (TypeTable::is_int_instruct(L->getType()) &&
                   TypeTable::is_int_instruct(R->getType())) {
            // Do integer versions
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
        } else {
            // Promote any integers to floats
            if (L->getType() == TypeTable::IntType->get_llvm()) {
                L = builder.CreateSIToFP(L, TypeTable::RealType->get_llvm());
            }
            if (R->getType() == TypeTable::IntType->get_llvm()) {
                R = builder.CreateSIToFP(R, TypeTable::RealType->get_llvm());
            }
            // do float equivalent
            switch (*ast->relation) {
            case TokenType::equals:
                last_value = builder.CreateFCmpOEQ(L, R);
                break;
            case TokenType::hash:
                last_value = builder.CreateFCmpONE(L, R);
                break;
            case TokenType::less:
                last_value = builder.CreateFCmpOLT(L, R);
                break;
            case TokenType::leq:
                last_value = builder.CreateFCmpOLE(L, R);
                break;
            case TokenType::greater:
                last_value = builder.CreateFCmpOGT(L, R);
                break;
            case TokenType::gteq:
                last_value = builder.CreateFCmpOGE(L, R);
                break;
            default:;
            }
        }
    }
}

void CodeGenerator::visit(ASTSimpleExpr const &ast) {
    debug("ASTSimpleExpr");
    visit(ast->term);
    Value *L = last_value;
    // if initial sign exists and is negative, negate the integer
    if (ast->first_sign && ast->first_sign.value() == TokenType::dash) {
        if (TypeTable::is_int_instruct(L->getType())) {
            L = builder.CreateSub(TypeTable::IntType->get_init(), L, "negtmp");
        } else {
            L = builder.CreateFSub(TypeTable::RealType->get_init(), L, "negtmp");
        }
        last_value = L;
    }

    BasicBlock *or_end_block = BasicBlock::Create(context, "or_end");
    std::vector<std::pair<BasicBlock *, Value *>> next_blocks;
    bool                                          use_end{false};

    for (auto const &[op, right] : ast->rest) {

        if (op == TokenType::or_k && builder.GetInsertBlock()) {
            // Lazy evaluation of OR, only when we are in a block
            next_blocks.emplace_back(builder.GetInsertBlock(), last_value);
            auto       *funct = builder.GetInsertBlock()->getParent();
            BasicBlock *or_next_block = BasicBlock::Create(context, "or_next", funct);
            builder.CreateCondBr(last_value, or_end_block, or_next_block);
            use_end = true;
            builder.SetInsertPoint(or_next_block);
            visit(right);
            L = last_value;
            continue;
        }

        visit(right);
        Value *R = last_value;
#if 0
        llvm::dbgs() << "L and R ";
        L->getType()->print(llvm::dbgs());
        llvm::dbgs() << " ";
        R->getType()->print(llvm::dbgs());
        llvm::dbgs() << '\n';
#endif
        if (right->get_type() == TypeTable::SetType) {
            // SET operations
            switch (op) {
            case TokenType::plus:
                last_value = builder.CreateOr(L, R, "setunion");
                break;
            case TokenType::dash:
                last_value = builder.CreateNot(R, "setdiff");
                last_value = builder.CreateAnd(L, last_value, "setdiff");
                break;
            default:
                throw CodeGenException(ast->get_location(),
                                       "ASTSimpleExpr with sign" + string(op));
            }
        } else if (L->getType() == TypeTable::StrType->get_llvm() ||
                   R->getType() == TypeTable::StrType->get_llvm()) {
            // STRING operations
            if (L->getType() == TypeTable::StrType->get_llvm() &&
                R->getType() == TypeTable::StrType->get_llvm()) {
                last_value =
                    call_function("Strings_Concat", TypeTable::StrType->get_llvm(), {L, R});
            } else if (R->getType() == TypeTable::CharType->get_llvm()) {
                last_value =
                    call_function("Strings_ConcatChar", TypeTable::StrType->get_llvm(), {L, R});
            } else {
                last_value =
                    call_function("Strings_AppendChar", TypeTable::StrType->get_llvm(), {L, R});
            }
        } else if (TypeTable::is_int_instruct(L->getType()) &&
                   TypeTable::is_int_instruct(R->getType())) {
            // INTEGER operations
            switch (op) {
            case TokenType::plus:
                last_value = builder.CreateAdd(L, R, "addtmp");
                break;
            case TokenType::dash:
                last_value = builder.CreateSub(L, R, "subtmp");
                break;
            case TokenType::or_k: // leave in for CONST calculations
                last_value = builder.CreateOr(L, R, "subtmp");
                break;
            default:
                throw CodeGenException(ast->get_location(),
                                       "ASTSimpleExpr with sign" + string(op));
            }
        } else {
            // Do float calculations
            // Promote any integers to floats
            if (L->getType() == TypeTable::IntType->get_llvm()) {
                L = builder.CreateSIToFP(L, TypeTable::RealType->get_llvm());
            }
            if (R->getType() == TypeTable::IntType->get_llvm()) {
                R = builder.CreateSIToFP(R, TypeTable::RealType->get_llvm());
            }
            switch (op) {
            case TokenType::plus:
                last_value = builder.CreateFAdd(L, R, "addtmp");
                break;
            case TokenType::dash:
                last_value = builder.CreateFSub(L, R, "subtmp");
                break;
            default:
                throw CodeGenException(ast->get_location(),
                                       "ASTSimpleExpr float with sign" + string(op));
            }
        }
        L = last_value;
    }
    if (use_end) {
        next_blocks.emplace_back(builder.GetInsertBlock(), last_value);
        builder.CreateBr(or_end_block);
        auto *funct = builder.GetInsertBlock()->getParent();
        funct->insert(funct->end(), or_end_block);
        builder.SetInsertPoint(or_end_block);
        auto *phi_node =
            builder.CreatePHI(TypeTable::BoolType->get_llvm(), next_blocks.size(), "or");
        for (auto const &[block, value] : next_blocks) {
            phi_node->addIncoming(value, block);
        };
        last_value = phi_node;
    }
}

void CodeGenerator::visit(ASTTerm const &ast) {
    debug("ASTTerm {0}", std::string(*ast));
    visit(ast->factor);
    Value *L = last_value;

    BasicBlock *end_block = BasicBlock::Create(context, "and_end");
    std::vector<std::pair<BasicBlock *, Value *>> next_blocks;
    bool                                          use_end{false};

    for (auto const &[op, right] : ast->rest) {

        if (op == TokenType::ampersand && builder.GetInsertBlock()) {
            // Lazy evaluation of & AND, only when we are in a block
            next_blocks.emplace_back(builder.GetInsertBlock(), last_value);
            auto       *funct = builder.GetInsertBlock()->getParent();
            BasicBlock *next_block = BasicBlock::Create(context, "and_next", funct);
            builder.CreateCondBr(last_value, next_block, end_block);
            use_end = true;
            builder.SetInsertPoint(next_block);
            visit(right);
            L = last_value;
            continue;
        }

        visit(right);
        Value *R = last_value;
#if 0
        llvm::dbgs() << "L and R ";
        L->getType()->print(llvm::dbgs());
        llvm::dbgs() << " ";
        R->getType()->print(llvm::dbgs());
        llvm::dbgs() << '\n';
#endif
        // debug("ASTTerm type L {0} R {1}", L->getType(), R->getType());
        if (right->get_type() == TypeTable::SetType) {
            // SET operations
            switch (op) {
            case TokenType::asterisk:
                last_value = builder.CreateAnd(L, R, "setintersect");
                break;
            case TokenType::slash: {
                // (x-y) + (y-x)
                auto *a = builder.CreateNot(R, "setsdiff");
                a = builder.CreateAnd(L, a, "setsdiff");
                auto *b = builder.CreateNot(L, "setsdiff");
                b = builder.CreateAnd(R, b, "setsdiff");
                last_value = builder.CreateOr(a, b, "setsdiff");
                break;
            }
            default:
                throw CodeGenException(ast->get_location(), "ASTTerm with sign" + string(op));
            }
        } else if (TypeTable::is_int_instruct(L->getType()) &&
                   TypeTable::is_int_instruct(R->getType())) {
            // Do integer calculations
            switch (op) {
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
                throw CodeGenException(ast->get_location(), "ASTTerm with sign" + string(op));
            }
        } else {
            // Do float calculations
            // Promote any integers to floats
            if (L->getType() == TypeTable::IntType->get_llvm()) {
                L = builder.CreateSIToFP(L, TypeTable::RealType->get_llvm());
            }
            if (R->getType() == TypeTable::IntType->get_llvm()) {
                R = builder.CreateSIToFP(R, TypeTable::RealType->get_llvm());
            }
            switch (op) {
            case TokenType::asterisk:
                last_value = builder.CreateFMul(L, R, "multmp");
                break;
            case TokenType::slash:
                last_value = builder.CreateFDiv(L, R, "divtmp");
                break;
            default:
                throw CodeGenException(ast->get_location(),
                                       "ASTTerm float with sign " + string(op));
            }
        }
        L = last_value;
    }
    if (use_end) {
        next_blocks.emplace_back(builder.GetInsertBlock(), last_value);
        builder.CreateBr(end_block);
        auto *funct = builder.GetInsertBlock()->getParent();
        funct->insert(funct->end(), end_block);
        builder.SetInsertPoint(end_block);
        auto *phi_node =
            builder.CreatePHI(TypeTable::BoolType->get_llvm(), next_blocks.size(), "and");
        for (auto const &[block, value] : next_blocks) {
            phi_node->addIncoming(value, block);
        };
        last_value = phi_node;
    }
}

void CodeGenerator::visit(ASTFactor const &ast) {
    // debug("ASTFactor {0}", std::string(*ast));
    // Visit the appropriate variant
    std::visit(overloaded{[this](auto arg) { arg->accept(this); },
                          [this, ast](ASTDesignator const &arg) { visitPtr(arg, false); },
                          [this, ast](ASTFactor const &arg) {
                              debug("visit: not ");
                              if (ast->is_not) {
                                  debug("visit: not do");
                                  arg->accept(this);
                                  last_value = builder.CreateNot(last_value);
                              }
                          }},
               ast->factor);
}

void CodeGenerator::visit_value(ASTRange const &ast, Value *case_value) {
    debug("ASTRange");
    visit(ast->first);
    auto *low = builder.CreateICmpSLE(last_value, case_value);
    visit(ast->last);
    auto *high = builder.CreateICmpSLE(case_value, last_value);
    last_value = builder.CreateAnd(low, high);
}

void CodeGenerator::get_index(ASTDesignator const &ast) {
    debug("get_index: {}", std::string(*ast));
    visitPtr(ast->ident, true);
    auto                *arg_ptr = last_value;
    std::vector<Value *> index{TypeTable::IntType->make_value(0)};

    for (auto const &s : ast->selectors) {

        std::visit(
            overloaded{[this, &index](ArrayRef const &s) {
                           for (auto iter = s.rbegin(); iter != s.rend(); ++iter) {
                               (*iter)->accept(this);
                               debug("GEP index is Int: {0}",
                                     last_value->getType()->isIntegerTy());
                               index.push_back(last_value);
                           }
                       },
                       [this, &index](FieldRef const &s) {
                           // calculate index
                           // extract the field index
                           debug("get_index record index {0} for {1}", s.second, s.first->value);
                           assert(s.second >= 0); // NOLINT

                           // record indexes are 32 bit integers
                           auto *idx = ConstantInt::get(llvm::Type::getInt32Ty(context), s.second);
                           index.push_back(idx);
                       },
                       [this, &arg_ptr](PointerRef /* unused */) {
                           arg_ptr = builder.CreateLoad(arg_ptr->getType(), arg_ptr);
                       }},
            s);
    }
    // debug("GEP is Ptr: {0}", arg_ptr->getType()->isPointerTy());
    // arg_ptr->getType()->print(llvm::dbgs());

    assert(arg_ptr->getType()->isPointerTy()); // NOLINT
    if (ast->ident->id->is(Attr::ptr) || ast->ident->get_type()->id == TypeId::openarray ||
        is_var) {
        debug("Create load");
#if 0
        if (ast->ident->get_type()->id == TypeId::openarray) {
            debug("openarray");
            auto array_type = dynamic_cast<OpenArrayType *>(ast->ident->get_type().get());
            arg_ptr = builder.CreateLoad(array_type->get_llvm(), arg_ptr);
            debug("arg_ptr ");
            arg_ptr->print(llvm::dbgs());
            llvm::dbgs() << "\n";
            ast->ident->get_type()->get_llvm()->print(llvm::dbgs());
            llvm::dbgs() << "\n";
            last_value =
                builder.CreateGEP(array_type->base_type->get_llvm(), arg_ptr, index, "idx");
            return;
        }
#endif
        arg_ptr = builder.CreateLoad(arg_ptr->getType(), arg_ptr);
    }
    debug("get_index: GEP number of indices: {0}", index.size());
    debug("get_index: basetype: {0}", std::string(*ast->ident->get_type()));
    last_value = builder.CreateGEP(ast->ident->get_type()->get_llvm(), arg_ptr, index, "idx");
}

/**
 * @brief Used to fetch values
 *
 * @param ast
 */
void CodeGenerator::visitPtr(ASTDesignator const &ast, bool ptr) {
    debug("ASTDesignator {0} {1}", std::string(*ast), ptr);

    visitPtr(ast->ident, ptr);
    // Check if has selectors
    if (ast->selectors.empty()) {
        return;
    }

    // Array structure
    get_index(ast);
    // debug("ASTDesignator: ptr:{0} is_var:{1}", ptr, is_var);
    if (!ptr && !is_var) {
        debug("ASTDesignator: load");
        last_value = builder.CreateLoad(ast->get_type()->get_llvm(), last_value, "idx");
    }
}

void CodeGenerator::visitPtr(ASTQualident const &ast, bool ptr) {
    debug("ASTQualident");
    if (!ast->qual.empty()) {
        // modify the AST
        ast->id->value = ast->make_coded_id();
    }
    visitPtr(ast->id, ptr);
}

void CodeGenerator::visitPtr(ASTIdentifier const &ast, bool ptr) {
    debug("ASTIdentifierPtr {0} {1}", ast->value, ptr);
    if (auto res = symboltable.find(ast->value); res) {
        last_value = res->value;
        if (res->is(Attr::var)) {
            debug("ASTIdentifierPtr VAR ");
            last_value = builder.CreateLoad(last_value->getType(), last_value, ast->value);
        }
        if (is_var) {
            // This is a write of a VAR variable, preseve this value
            last_value = res->value;
        }
        if (!ptr) {
            debug("ASTIdentifierPtr !ptr type: {}", res->type->get_name());
            auto type = types.resolve(res->type->get_name());
            last_value = builder.CreateLoad(type->get_llvm(), last_value, ast->value);
        }
        return;
    }
}

void CodeGenerator::visit(ASTSet const &ast) {
    Value *set_value = TypeTable::SetType->get_init();
    auto  *one = TypeTable::IntType->make_value(1);
    for (auto const &exp : ast->values) {
        std::visit(overloaded{[this, &set_value, ast, one](ASTSimpleExpr const &exp) {
                                  debug("ASTSet exp");
                                  exp->accept(this);
                                  auto *index = builder.CreateShl(one, last_value);
                                  set_value = builder.CreateOr(set_value, index);
                              },
                              [this, &set_value, ast, one](ASTRange const &exp) {
                                  debug("ASTSet range");
                                  exp->first->accept(this);
                                  auto *first = last_value;
                                  exp->last->accept(this);
                                  auto *last = last_value;
                                  set_value =
                                      call_function("Set_range", TypeTable::SetType->get_llvm(),
                                                    {set_value, first, last});
                              }},
                   exp);
    }
    last_value = set_value;
}

void CodeGenerator::visit(ASTInteger const &ast) {
    last_value = TypeTable::IntType->make_value(ast->value);
}

void CodeGenerator::visit(ASTReal const &ast) {
    last_value = TypeTable::RealType->make_value(ast->value);
}

void CodeGenerator::visit(ASTCharPtr const &ast) {
    last_value = TypeTable::CharType->make_value(ast->value);
}

void CodeGenerator::visit(ASTString const &ast) {
    debug("ASTString {0}", ast->value);

    if (do_strchar_conv) {
        auto char_val = ast->value[0];
        last_value = TypeTable::CharType->make_value(char_val);
        return;
    }

    GlobalVariable *var = nullptr;
    if (auto res = global_strings.find(ast->value); res != global_strings.end()) {
        var = res->second;
    } else {
        std::string name = std::format("STRING_{0}", string_const++);
        var = generate_global(name, TypeTable::StrType->make_type(ast->value));
        var->setInitializer(TypeTable::StrType->make_value(ast->value));
        var->setLinkage(GlobalValue::LinkageTypes::PrivateLinkage);
        global_strings[ast->value] = var;
    }
    last_value = builder.CreateBitCast(var, TypeTable::StrType->make_type_ptr());
}

void CodeGenerator::visit(ASTBool const &ast) {
    last_value = TypeTable::BoolType->make_value(ast->value);
}

void CodeGenerator::visit(ASTNil const & /*not used*/) {
    last_value = TypeTable::VoidType->get_init();
}

std::string CodeGenerator::gen_module_id(std::string const &id) const {
    return ASTQualident_::make_coded_id(module_name, id);
}

/**
 * @brief like AST_Call but arguments already compiled, used from builtins to
 * call other functions, like LEN(STRING)
 *
 * @param name
 * @param ret
 * @param args
 * @return Value*
 */
Value *CodeGenerator::call_function(std::string const &name, llvm::Type *ret,
                                    std::vector<Value *> const &args) {
    debug("call_function {0}", name);

    auto *f = module->getFunction(name);
    if (!f) {

        debug("call_function generate {0}", name);
        std::vector<llvm::Type *> proto;
        for (auto const &arg : args) {
            proto.push_back(arg->getType());
        }
        auto *ft = FunctionType::get(ret, proto, false);
        f = Function::Create(ft, Function::LinkageTypes::ExternalLinkage, name, module.get());
    }
    debug("call_function call {0}", name);
    return builder.CreateCall(f, args);
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
                                                  ASTType type, bool var) {
    AllocaInst *res = nullptr;
    IRBuilder<> TmpB(&function->getEntryBlock(), function->getEntryBlock().begin());

    std::visit(
        [&](auto /*unused*/) {
            llvm::Type *t = getType(type);
            if (var) {
                t = llvm::PointerType::get(context, 0);
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

Type CodeGenerator::resolve_type(ASTType const &t) {
    // debug("resolve_type {0}", std::string(*t));
    Type result;
    std::visit(overloaded{[this, &result](ASTQualident const &type) {
                              result = types.resolve(type->id->value);

                              // should be a resloved type this far down
                              assert(result && "Type not found"); // NOLINT
                          },
                          [t, &result, this](auto /* not used*/) {
                              result = types.resolve(t->get_type()->get_name());
                          }},
               t->type);
    // debug("resolve_type to {0}", std::string(*result));
    return result;
}

llvm::Type *CodeGenerator::getType(ASTType const &t) {
    // debug("getType {0}", std::string(*t));
    return resolve_type(t)->get_llvm();
}

Constant *CodeGenerator::getType_init(ASTType const &t) {
    // debug("getType_init {0}", std::string(*t));
    return resolve_type(t)->get_init();
}

void CodeGenerator::setup_builtins() {
    debug("setup_builtins");

    for (auto const &f : Builtin::global_functions) {
        // debug("function: {0} ", f.first);

        if (auto res = symboltable.find(f.first);
            res->is(Attr::used) && res->is(Attr::global_function)) {
            auto *funcType = dyn_cast<FunctionType>(res->type->get_llvm());

            auto *func = Function::Create(funcType, Function::LinkageTypes::ExternalLinkage,
                                          f.first, module.get());
            verifyFunction(*func);
            symboltable.set_value(f.first, func);
        }
    }
}

/**
 * @brief eject a BR instruction in a block if the statements where empty, or
 * the previous is not a BR or RET instruction
 *
 */
void CodeGenerator::ejectBranch(std::vector<ASTStatement> const &stats, BasicBlock *block,
                                BasicBlock *where) {
    debug("ejectBranch");
    if (stats.empty() || (block->back().getOpcode() != llvm::Instruction::Br &&
                          block->back().getOpcode() != llvm::Instruction::Ret)) {
        // not terminator (branch) put in EXIT
        debug("ejectBranch BR");
        builder.CreateBr(where);
    }
}

std::string CodeGenerator::get_nested_name() {
    const auto *insert{""};
    std::string result{};
    for (auto const &name : nested_procs) {
        result += insert + name;
        insert = "_";
    }
    return result;
}

GlobalVariable *CodeGenerator::generate_global(std::string const &name, llvm::Type *t) {
    module->getOrInsertGlobal(name, t);
    return module->getNamedGlobal(name);
}

FunctionCallee CodeGenerator::generate_function(std::string const &name, llvm::Type *ret,
                                                llvm::ArrayRef<llvm::Type *> const &params) {
    llvm::FunctionType *ft = llvm::FunctionType::get(ret, params, false);
    return module->getOrInsertFunction(name, ft);
}

void CodeGenerator::init() {
    module = std::make_unique<Module>(module_name, context);
    module->setSourceFileName(options.file_name);
}

void CodeGenerator::optimize() {
    llvm::PassBuilder passBuilder;

    llvm::LoopAnalysisManager     loopAnalysisManager; // * add constructor true for debug info
    llvm::FunctionAnalysisManager functionAnalysisManager;
    llvm::CGSCCAnalysisManager    cGSCCAnalysisManager;
    llvm::ModuleAnalysisManager   moduleAnalysisManager;

    passBuilder.registerModuleAnalyses(moduleAnalysisManager);
    passBuilder.registerCGSCCAnalyses(cGSCCAnalysisManager);
    passBuilder.registerFunctionAnalyses(functionAnalysisManager);
    passBuilder.registerLoopAnalyses(loopAnalysisManager);

    passBuilder.crossRegisterProxies(loopAnalysisManager, functionAnalysisManager,
                                     cGSCCAnalysisManager, moduleAnalysisManager);

    auto opt_level{OptimizationLevel::O0};
    switch (options.optimise) {
    case 1:
        opt_level = OptimizationLevel::O1;
        break;
    case 2:
        opt_level = OptimizationLevel::O2;
        break;
    case 3:
        opt_level = OptimizationLevel::O3;
        break;
    }

    llvm::ModulePassManager modulePassManager =
        passBuilder.buildPerModuleDefaultPipeline(opt_level);
    modulePassManager.run(*module, moduleAnalysisManager);
}

void CodeGenerator::generate_llcode() {
    debug("generate_llcode");
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
    debug("generate_objectcode");

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

    TargetOptions const opt;
    auto                RM = std::optional<Reloc::Model>();

    llvm::Triple const triple(targetTriple);
    auto              *targetMachine = target->createTargetMachine(triple, CPU, features, opt, RM);

    module->setDataLayout(targetMachine->createDataLayout());
    module->setTargetTriple(triple);

    auto            f = filename + file_ext_obj;
    std::error_code EC;
    raw_fd_ostream  dest_file(f, EC, sys::fs::OF_None);

    if (EC) {
        throw CodeGenException("Could not open file: " + EC.message());
    }

    legacy::PassManager pass;
    auto                file_type = llvm::CodeGenFileType::ObjectFile;

    if (targetMachine->addPassesToEmitFile(pass, dest_file, nullptr, file_type)) {
        throw CodeGenException("TargetMachine can't emit a file of this type");
    }
    pass.run(*module);
    dest_file.flush();
}

} // namespace ax
