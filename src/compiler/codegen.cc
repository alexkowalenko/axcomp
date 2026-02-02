//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "codegen.hh"

#include <exception>
#include <format>
#include <memory>
#include <ranges>

#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
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

#include "ast/all.hh"
#include "astvisitor.hh"
#include "builtin.hh"
#include "error.hh"
#include "symbol.hh"
#include "symboltable.hh"
#include "token.hh"
#include "types/all.hh"
#include "typetable.hh"

namespace ax {

namespace {
// this has to be a #define for it work properly
#define DEBUG_TYPE "codegen"

template <typename S, typename... Args> void debug(const S &format, const Args &...msg) {
    LLVM_DEBUG(llvm::dbgs() << DEBUG_TYPE << ' '
                            << std::vformat(format, std::make_format_args(msg...)) << '\n');
}

using namespace llvm::sys;

constexpr auto file_ext_llvmri{".ll"};
constexpr auto file_ext_obj{".o"};

constexpr bool is_string_type(Type const &type) {
    return (type->id == TypeId::STRING || type->id == TypeId::STR1);
}

constexpr bool is_char_type(Type const &type) {
    return type->id == TypeId::CHAR;
}

} // namespace

CodeGenerator::CodeGenerator(Options &o, SymbolFrameTable &s, TypeTable &t, Importer &i)
    : options{o}, symboltable{s}, types{t}, importer{i}, filename("main"), builder(context) {
    debug("CodeGenerator::CodeGenerator");
    TypeTable::setTypes(context);
}

Value *CodeGenerator::eval_expr(ASTExpr const &expr) {
    expr->accept(this);
    return last_value;
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
    constexpr std::vector<llvm::Type *> proto;
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

    for (const auto &key : ast->imports | std::views::keys) {
        SymbolFrameTable symbols;
        const auto       found = importer.find_module(key->value, symbols, types);
        assert(found);

        debug("ASTImport do {0}", key->value);

        for (const auto &[name, snd] : symbols) {
            const auto type = snd->type;
            debug("ASTImport get {0} : {1}", name, type->get_name());

            if (type->is_referencable()) {
                GlobalVariable *gVar = generate_global(name, type->get_llvm());
                debug("ASTImport var {0}", name);
                symboltable.set_value(name, gVar, Attr::global);
            } else if (type->id == TypeId::PROCEDURE) {
                debug("ASTImport proc {0}", name);
                if (const auto res = symboltable.find(name); res->is(Attr::used)) {
                    auto *funcType = static_cast<FunctionType *>(type->get_llvm());

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

void CodeGenerator::doTopVars(ASTVar const &ast) const {
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
    llvm::Type                                            *closure_type = nullptr;

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
        auto                   *type = getType(t_type);
        llvm::AttrBuilder const attrs(context);
        index++;
        if (index == 1 && sym->is(Attr::closure)) {
            debug("ASTProcedure {0} is closure function", ast->name->value);
            if (!closure_type) {
                closure_type = std::dynamic_pointer_cast<ProcedureType>(sym->type)
                                   ->get_closure_struct()
                                   ->get_llvm();
            }
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
    for (const auto &attr : argAttr | std::views::values) {
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
        auto        attr = Attr::null;
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

        // put also into the symbol table
        symboltable.set_value(param_name, alloca, attr);
        debug("put {0} into symbol table", param_name);
        i++;
    };

    // Do declarations
    visit(ast->decs);

    // Do closure variables
    if (sym->is(Attr::closure)) {
        // Set closure variables
        debug("ASTProcedure set up closure variables {}", ast->name->value);

        std::vector<llvm::Value *> ind = {TypeTable::IntType->make_value(0), nullptr};
        unsigned                   i = 0;
        auto                      *cls_arg = f->arg_begin();
        if (!closure_type) {
            closure_type = std::dynamic_pointer_cast<ProcedureType>(sym->type)
                               ->get_closure_struct()
                               ->get_llvm();
        }
        for (auto const &[cls_var, cls_type] :
             std::dynamic_pointer_cast<ProcedureType>(sym->type)->free_vars) {
            ind[1] = TypeTable::IntType->make_value(i);

            LLVM_DEBUG({
                llvm::dbgs() << "closure callee GEP cls_var=" << cls_var << " cls_arg_type=";
                cls_arg->getType()->print(llvm::dbgs());
                llvm::dbgs() << " closure_type=";
                closure_type->print(llvm::dbgs());
                llvm::dbgs() << " idx_type=";
                ind[1]->getType()->print(llvm::dbgs());
                llvm::dbgs() << "\n";
            });
            llvm::Value *a = builder.CreateGEP(closure_type, &*cls_arg, ind, cls_var);
            a = builder.CreateLoad(a->getType(), a, cls_var);

            // put into symbol table
            // this is meant to shadow the variable in the outer scope, otherwise the
            // outer scope variable changes.
            debug("ASTProcedure set_value {} : {}, type: {}", cls_arg->getName(), a->getName(),
                  cls_type->get_name());

            auto closure_sym = mkSym(cls_type);
            closure_sym->value = a;
            symboltable.put(cls_var, closure_sym);
            i++;
        }
    }

    for (auto const &proc : ast->procedures) {
        proc->accept(this);
    }

    builder.SetInsertPoint(block);

    // for recursion
    debug("ASTProcedure set function {}", ast->name->value);
    symboltable.set_value(ast->name->value, f);

    // Clear the last_end stack
    while (!last_end.empty()) {
        last_end.pop();
    }

    // Work through the statements
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
    // set function in outer scope, incase function name identical in outer and inner scope
    symboltable.set_value(ast->name->value, f);
    nested_procs.pop_back();
    // Validate the generated code, checking for consistency.
    verifyFunction(*f);
}

void CodeGenerator::visit(ASTProcedureForward const &ast) {

    const auto res = symboltable.find(ast->name->value);
    auto      *funcType = dyn_cast<FunctionType>(res->type->get_llvm());

    const auto proc_name = gen_module_id(ast->name->value);
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
    if (const auto res = symboltable.find(ast->ident->ident->id->value); res) {
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
    // debug("ASTAssignment store {0} in {1}", val->getName(), last_value->getName());

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
    if (!last_end.empty()) {
        builder.CreateBr(last_end.top());
    } else {
        throw CodeGenException(ast->get_location(), "EXIT: no enclosing loop.");
    }
}

std::tuple<std::shared_ptr<ProcedureType>, std::string, bool>
CodeGenerator::do_find_proc(ASTCall const &ast) const {
    // Look up the name in the global module table.

    auto name = ast->name->ident->make_coded_id();
    auto res = symboltable.find(name);
    bool bound_proc{false};

    auto typeFunction = std::dynamic_pointer_cast<ProcedureType>(res->type);
    if (!typeFunction) {
        // check bound procedure
        const auto field = ast->name->first_field();
        name = field->value;
        res = symboltable.find(name);
        typeFunction = std::dynamic_pointer_cast<ProcedureType>(res->type);
        bound_proc = true;
    }
    return {typeFunction, name, bound_proc};
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
            auto *recv_type = typeFunction->receiver->get_llvm();
            last_value = builder.CreateLoad(recv_type, last_value);
        }

        args.push_back(last_value);
    }
    auto i = 0;
    for (auto const &a : ast->args) {
        const bool is_var_param = i < static_cast<int>(typeFunction->params.size()) &&
                                  typeFunction->params[i].second == Attr::var;
        if (is_var_param) {
            debug("ASTCall VAR parameter");
            // Var Parameter

            // Get Identifier, get pointer set to last_value  visitPtr
            // This works, since the Inspector checks that VAR arguments are
            // only identifiers
            auto ptr = a->expr->term->factor->factor;
            auto p2 = std::get<ASTDesignator>(ptr)->ident;

            debug("ASTCall identifier {0}", p2->id->value);
            visitPtr(p2->id, true);
            if (typeFunction->params[i].first &&
                typeFunction->params[i].first->id == TypeId::OPENARRAY && a->get_type() &&
                (a->get_type()->id == TypeId::STRING || a->get_type()->id == TypeId::STR1)) {
                if (llvm::isa<AllocaInst>(last_value) || llvm::isa<GlobalVariable>(last_value)) {
                    last_value = builder.CreateLoad(last_value->getType(), last_value);
                }
            }
        } else {
            // Reference Parameter

            // Check for STRING1 to CHAR conversion
            do_strchar_conv = a->get_type()->id == TypeId::STR1 &&
                              (typeFunction->params[i].first &&
                               typeFunction->params[i].first->id == TypeId::CHAR);
            visit(a);
        }
        args.push_back(last_value);
        i++;
    }
    return args;
}

void CodeGenerator::visit(ASTCall const &ast) {
    const auto name = ast->name->ident->make_coded_id();
    debug("ASTCall: {0}", name);
    auto res = symboltable.find(name);

    if (res->is(Attr::compile_function)) {
        debug("ASTCall: compile {0}", name);
        const auto &f = Builtin::compile_functions[name];
        last_value = f(this, ast);
        return;
    }
    debug("ASTCall: global {0}", name);
    auto args = do_arguments(ast);
    assert(res->value && "did not find function to call!");
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
        for (const auto &free_var_name : callee_type->free_vars | std::views::keys) {
            const auto   r = symboltable.find(free_var_name);
            llvm::Value *v = r->value;
            ind[1] = TypeTable::IntType->make_value(index);
            LLVM_DEBUG({
                llvm::dbgs() << "closure caller GEP name=" << free_var_name << " closure_type=";
                cls_ty->print(llvm::dbgs());
                llvm::dbgs() << " closure_value_type=";
                closure->getType()->print(llvm::dbgs());
                llvm::dbgs() << " idx_type=";
                ind[1]->getType()->print(llvm::dbgs());
                llvm::dbgs() << "\n";
            });
            llvm::Value *ptr = builder.CreateGEP(cls_ty, closure, ind, "cls");
            // debug("ASTCall closure call {0} : {1}", v->getName(), name);
            builder.CreateStore(v, ptr);
            index++;
        }
        args.insert(args.begin(), closure);
    }
    auto *inst = builder.CreateCall(callee, args);
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
        const auto elses = *ast->else_clause;
        for (auto const &stmt : elses) {
            stmt->accept(this);
        }
        else_block = builder.GetInsertBlock(); // necessary for correct generation of code
        ejectBranch(elses, else_block, merge_block);
    }

    // codegen of ELSE can change the current block, update else_block
    else_block = builder.GetInsertBlock();

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
                range_list.emplace_back(range, i);
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
        else_block = builder.GetInsertBlock(); // necessary for correct generation of code
    }

    // END
    funct->insert(funct->end(), end_block);
    builder.SetInsertPoint(end_block);
    end_block = builder.GetInsertBlock(); // necessary for correct generation of code
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
    last_end.push(after);

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
    last_end.pop();

    // debug("ASTFor after:{0}", last_end);
}

void CodeGenerator::visit(ASTWhile const &ast) {
    debug("ASTWhile");

    // Create blocks
    auto       *funct = builder.GetInsertBlock()->getParent();
    BasicBlock *while_block = BasicBlock::Create(context, "while", funct);
    BasicBlock *loop = BasicBlock::Create(context, "loop");
    BasicBlock *end_block = BasicBlock::Create(context, "end");
    last_end.push(end_block);
    auto *cond_block = while_block;

    builder.CreateBr(while_block);
    builder.SetInsertPoint(while_block);

    // WHILE Expr
    visit(ast->expr);
    builder.CreateCondBr(last_value, loop, end_block);

    // DO
    funct->insert(funct->end(), loop);
    builder.SetInsertPoint(loop);

    for (auto const &stat : ast->stats) {
        stat->accept(this);
    }

    loop = builder.GetInsertBlock(); // necessary for correct generation of code
    ejectBranch(ast->stats, loop, cond_block);

    // END
    funct->insert(funct->end(), end_block);
    builder.SetInsertPoint(end_block);
    last_end.pop();
}

void CodeGenerator::visit(ASTRepeat const &ast) {
    debug("ASTRepeat");

    // Create blocks
    auto       *funct = builder.GetInsertBlock()->getParent();
    BasicBlock *repeat_block = BasicBlock::Create(context, "repeat", funct);
    BasicBlock *end_block = BasicBlock::Create(context, "end");
    last_end.push(end_block);

    // REPEAT
    builder.CreateBr(repeat_block);
    builder.SetInsertPoint(repeat_block);

    for (auto const &stat : ast->stats) {
        stat->accept(this);
    }

    // Expr
    visit(ast->expr);
    builder.CreateCondBr(last_value, end_block, repeat_block);
    repeat_block = builder.GetInsertBlock(); // necessary for correct generation of code

    // END
    funct->insert(funct->end(), end_block);
    builder.SetInsertPoint(end_block);
    last_end.pop();
}

void CodeGenerator::visit(ASTLoop const &ast) {
    debug("ASTLoop");

    // Create blocks
    auto       *funct = builder.GetInsertBlock()->getParent();
    BasicBlock *loop_block = BasicBlock::Create(context, "loop", funct);
    BasicBlock *end_block = BasicBlock::Create(context, "end");
    last_end.push(end_block);

    // LOOP
    builder.CreateBr(loop_block);
    builder.SetInsertPoint(loop_block);

    for (auto const &stat : ast->stats) {
        stat->accept(this);
    }
    builder.CreateBr(loop_block);
    loop_block = builder.GetInsertBlock(); // necessary for correct generation of code

    // END
    funct->insert(funct->end(), end_block);
    builder.SetInsertPoint(end_block);
    last_end.pop();
}

void CodeGenerator::visit(ASTBlock const &ast) {
    debug("ASTBlock");

    // Create blocks
    auto       *funct = builder.GetInsertBlock()->getParent();
    BasicBlock *begin_block = BasicBlock::Create(context, "begin", funct);
    BasicBlock *end_block = BasicBlock::Create(context, "end");
    last_end.push(end_block);

    // BEGIN
    builder.CreateBr(begin_block);
    builder.SetInsertPoint(begin_block);

    for (auto const &stat : ast->stats) {
        stat->accept(this);
    }

    builder.CreateBr(end_block);
    begin_block = builder.GetInsertBlock(); // necessary for correct generation of code
    // END
    funct->insert(funct->end(), end_block);
    builder.SetInsertPoint(end_block);
    last_end.push(end_block);
}

void CodeGenerator::visit(ASTExpr const &ast) {
    debug("ASTExpr");
    visit(ast->expr);

    if (ast->relation) {
        auto *L = last_value;
        visit(ast->relation_expr);
        auto      *R = last_value;
        auto       lhs_type = ast->expr->get_type();
        auto       rhs_type = ast->relation_expr->get_type();
        const bool lhs_is_string = is_string_type(lhs_type);
        const bool rhs_is_string = is_string_type(rhs_type);
        if (lhs_is_string && rhs_is_string) {
            // String comparisons
            last_value = call_function("Strings_Compare", TypeTable::IntType->get_llvm(), {L, R});
            switch (*ast->relation) {
            case TokenType::EQUALS:
                last_value = builder.CreateNot(last_value);
                break;
            case TokenType::HASH:
                // Correct
                break;
            case TokenType::LESS:
                last_value = builder.CreateICmpSLT(
                    last_value, TypeTable::IntType->get_init()); // last_value > 0
                break;
            case TokenType::LEQ:
                last_value = builder.CreateICmpSLE(last_value, TypeTable::IntType->get_init());
                break;
            case TokenType::GREATER:
                last_value = builder.CreateICmpSGT(last_value, TypeTable::IntType->get_init());
                break;
            case TokenType::GTEQ:
                last_value = builder.CreateICmpSGE(last_value, TypeTable::IntType->get_init());
                break;
            default:;
            }
        } else if ((lhs_type && lhs_type->id == TypeId::POINTER && rhs_type &&
                    rhs_type->id == TypeId::VOID) ||
                   (lhs_type && lhs_type->id == TypeId::VOID && rhs_type &&
                    rhs_type->id == TypeId::POINTER)) {
            // Pointer comparisons
            if (lhs_type->id == TypeId::VOID) {
                std::swap(L, R);
            }
            R = builder.CreateBitCast(R, L->getType());
            switch (*ast->relation) {
            case TokenType::EQUALS:
                last_value = builder.CreateICmpEQ(L, R);
                break;
            case TokenType::HASH:
                last_value = builder.CreateICmpNE(L, R);
                break;
            default:;
            }
        } else if (ast->relation_expr->get_type() == TypeTable::SetType) {
            // SET comparisons
            debug("ASTExpr set comparisons");
            switch (*ast->relation) {
            case TokenType::IN: {
                auto *index = builder.CreateShl(TypeTable::IntType->make_value(1), L);
                last_value = builder.CreateAnd(R, index);
                last_value = builder.CreateICmpUGT(last_value, TypeTable::IntType->get_init());
                break;
            }
            case TokenType::EQUALS:
                last_value = builder.CreateICmpEQ(L, R);
                break;
            case TokenType::HASH:
                last_value = builder.CreateICmpNE(L, R);
                break;
            default:;
            }

        } else if (TypeTable::is_int_instruct(L->getType()) &&
                   TypeTable::is_int_instruct(R->getType())) {
            // Do integer versions
            switch (*ast->relation) {
            case TokenType::EQUALS:
                last_value = builder.CreateICmpEQ(L, R);
                break;
            case TokenType::HASH:
                last_value = builder.CreateICmpNE(L, R);
                break;
            case TokenType::LESS:
                last_value = builder.CreateICmpSLT(L, R);
                break;
            case TokenType::LEQ:
                last_value = builder.CreateICmpSLE(L, R);
                break;
            case TokenType::GREATER:
                last_value = builder.CreateICmpSGT(L, R);
                break;
            case TokenType::GTEQ:
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
            case TokenType::EQUALS:
                last_value = builder.CreateFCmpOEQ(L, R);
                break;
            case TokenType::HASH:
                last_value = builder.CreateFCmpONE(L, R);
                break;
            case TokenType::LESS:
                last_value = builder.CreateFCmpOLT(L, R);
                break;
            case TokenType::LEQ:
                last_value = builder.CreateFCmpOLE(L, R);
                break;
            case TokenType::GREATER:
                last_value = builder.CreateFCmpOGT(L, R);
                break;
            case TokenType::GTEQ:
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
    auto   current_type = ast->term->get_type();
    // if the initial sign exists and is negative, negate the integer
    if (ast->first_sign && ast->first_sign.value() == TokenType::DASH) {
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

        if (op == TokenType::OR && builder.GetInsertBlock()) {
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
        Value     *R = last_value;
        const auto right_type = right->get_type();
        const bool left_is_string = is_string_type(current_type);
        const bool right_is_string = is_string_type(right_type);
        const bool left_is_char = is_char_type(current_type);
        const bool right_is_char = is_char_type(right_type);
        LLVM_DEBUG({
            llvm::dbgs() << "L and R ";
            L->getType()->print(llvm::dbgs());
            llvm::dbgs() << " ";
            R->getType()->print(llvm::dbgs());
            llvm::dbgs() << '\n';
        });
        if (right_type == TypeTable::SetType) {
            // SET operations
            switch (op) {
            case TokenType::PLUS:
                last_value = builder.CreateOr(L, R, "setunion");
                break;
            case TokenType::DASH:
                last_value = builder.CreateNot(R, "setdiff");
                last_value = builder.CreateAnd(L, last_value, "setdiff");
                break;
            default:
                throw CodeGenException(ast->get_location(),
                                       "ASTSimpleExpr with sign" + string(op));
            }
            current_type = TypeTable::SetType;
        } else if (left_is_string || right_is_string) {
            // STRING operations
            if (left_is_string && right_is_string) {
                last_value =
                    call_function("Strings_Concat", TypeTable::StrType->get_llvm(), {L, R});
            } else if (left_is_string && right_is_char) {
                last_value =
                    call_function("Strings_ConcatChar", TypeTable::StrType->get_llvm(), {L, R});
            } else if (left_is_char && right_is_string) {
                last_value =
                    call_function("Strings_AppendChar", TypeTable::StrType->get_llvm(), {L, R});
            } else {
                throw CodeGenException(ast->get_location(),
                                       "Invalid STRING operation " + string(op));
            }
            current_type = TypeTable::StrType;
        } else if (TypeTable::is_int_instruct(L->getType()) &&
                   TypeTable::is_int_instruct(R->getType())) {
            // INTEGER operations
            switch (op) {
            case TokenType::PLUS:
                last_value = builder.CreateAdd(L, R, "addtmp");
                break;
            case TokenType::DASH:
                last_value = builder.CreateSub(L, R, "subtmp");
                break;
            case TokenType::OR: // leave in for CONST calculations
                last_value = builder.CreateOr(L, R, "subtmp");
                break;
            default:
                throw CodeGenException(ast->get_location(),
                                       "ASTSimpleExpr with sign" + string(op));
            }
            current_type = TypeTable::IntType;
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
            case TokenType::PLUS:
                last_value = builder.CreateFAdd(L, R, "addtmp");
                break;
            case TokenType::DASH:
                last_value = builder.CreateFSub(L, R, "subtmp");
                break;
            default:
                throw CodeGenException(ast->get_location(),
                                       "ASTSimpleExpr float with sign" + string(op));
            }
            current_type = TypeTable::RealType;
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

        if (op == TokenType::AMPERSAND && builder.GetInsertBlock()) {
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
        LLVM_DEBUG({
            llvm::dbgs() << "L and R ";
            L->getType()->print(llvm::dbgs());
            llvm::dbgs() << " ";
            R->getType()->print(llvm::dbgs());
            llvm::dbgs() << '\n';
        });
        // debug("ASTTerm type L {0} R {1}", L->getType(), R->getType());
        if (right->get_type() == TypeTable::SetType) {
            // SET operations
            switch (op) {
            case TokenType::ASTÉRIX:
                last_value = builder.CreateAnd(L, R, "setintersect");
                break;
            case TokenType::SLASH: {
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
            case TokenType::ASTÉRIX:
                last_value = builder.CreateMul(L, R, "multmp");
                break;
            case TokenType::DIV:
                last_value = builder.CreateSDiv(L, R, "divtmp");
                break;
            case TokenType::MOD:
                last_value = builder.CreateSRem(L, R, "modtmp");
                break;
            case TokenType::AMPERSAND:
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
            case TokenType::ASTÉRIX:
                last_value = builder.CreateFMul(L, R, "multmp");
                break;
            case TokenType::SLASH:
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
    std::visit(overloaded{[this](auto arg) { visit(arg); },
                          [this, ast](ASTDesignator const &arg) { visitPtr(arg, false); },
                          [this, ast](ASTFactor const &arg) {
                              debug("visit: not ");
                              if (ast->is_not) {
                                  debug("visit: not do");
                                  visit(arg);
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
    auto *arg_ptr = last_value;

    auto ident_type = ast->ident->get_type();
    const bool is_string_type = ident_type->id == TypeId::STRING || ident_type->id == TypeId::STR1;
    bool       implicit_ptr_deref = false;

    auto current_type = ident_type;
    bool is_array = current_type->is_array();
    bool is_record = current_type->id == TypeId::RECORD;
    bool is_pointer = current_type->id == TypeId::POINTER;
    bool is_string = is_string_type;

    std::vector<Value *> index;
    if (ident_type->id != TypeId::OPENARRAY && !is_string_type) {
        auto *zero = ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
        index.push_back(zero);
    }

    for (auto const &s : ast->selectors) {
        std::visit(
            overloaded{[this, &index, &is_array, &is_string, &current_type](ArrayRef const &s) {
                           for (const auto &iter : std::ranges::reverse_view(s)) {
                               const auto was_var = is_var;
                               is_var = false;
                               visit(iter);
                               is_var = was_var;
                               debug("GEP index is Int: {0}",
                                     last_value->getType()->isIntegerTy());
                               if (last_value->getType() != llvm::Type::getInt32Ty(context)) {
                                   last_value = builder.CreateSExtOrTrunc(
                                       last_value, llvm::Type::getInt32Ty(context), "idx");
                               }
                               index.push_back(last_value);
                           }
                           if (is_array) {
                               if (const auto array_type =
                                       std::dynamic_pointer_cast<ArrayType>(current_type)) {
                                   current_type = array_type->base_type;
                               }
                           } else if (is_string) {
                               current_type = TypeTable::CharType;
                           }
                           is_array = current_type->is_array();
                           is_string = current_type->id == TypeId::STRING ||
                                       current_type->id == TypeId::STR1;
                       },
                       [this, &index, &arg_ptr, &current_type, &is_record, &is_pointer,
                        &implicit_ptr_deref](FieldRef const &s) {
                           if (!is_record && is_pointer) {
                               if (const auto ptr_type =
                                       std::dynamic_pointer_cast<PointerType>(current_type)) {
                                   if (ptr_type->get_reference()->id == TypeId::RECORD) {
                                       arg_ptr = builder.CreateLoad(arg_ptr->getType(), arg_ptr);
                                       current_type = ptr_type->get_reference();
                                       is_record = true;
                                       is_pointer = false;
                                       implicit_ptr_deref = true;
                                   }
                               }
                           }
                           // calculate index
                           // extract the field index
                           debug("get_index record index {0} for {1}", s.second, s.first->value);
                           assert(s.second >= 0);

                           // record indexes are 32 bit integers
                           auto *idx = ConstantInt::get(llvm::Type::getInt32Ty(context), s.second);
                           index.push_back(idx);
                       },
                       [this, &arg_ptr, &current_type, &is_record, &is_pointer, &is_array,
                        &is_string](PointerRef /* unused */) {
                           arg_ptr = builder.CreateLoad(arg_ptr->getType(), arg_ptr);
                           if (const auto ptr_type =
                                   std::dynamic_pointer_cast<PointerType>(current_type)) {
                               current_type = ptr_type->get_reference();
                               is_record = current_type->id == TypeId::RECORD;
                               is_array = current_type->is_array();
                               is_string = current_type->id == TypeId::STRING ||
                                           current_type->id == TypeId::STR1;
                               is_pointer = current_type->id == TypeId::POINTER;
                           }
                       }},
            s);
    }

    assert(arg_ptr->getType()->isPointerTy());
    if (ast->ident->get_type()->id == TypeId::OPENARRAY) {
        const auto *array_type = dynamic_cast<OpenArrayType *>(ast->ident->get_type().get());
        auto       *elem_type = array_type->base_type->get_llvm();
        auto       *elem_ptr_type = llvm::PointerType::get(context, 0);
        LLVM_DEBUG({
            llvm::dbgs() << "openarray get_index elem_type=";
            elem_type->print(llvm::dbgs());
            llvm::dbgs() << " elem_ptr_type=";
            elem_ptr_type->print(llvm::dbgs());
            llvm::dbgs() << " arg_ptr_type(before)=";
            arg_ptr->getType()->print(llvm::dbgs());
            llvm::dbgs() << "\n";
        });
        if (llvm::isa<AllocaInst>(arg_ptr) || llvm::isa<GlobalVariable>(arg_ptr)) {
            arg_ptr = builder.CreateLoad(elem_ptr_type, arg_ptr);
        }

        LLVM_DEBUG({
            llvm::dbgs() << "openarray get_index arg_ptr_type(after load)=";
            arg_ptr->getType()->print(llvm::dbgs());
            llvm::dbgs() << "\n";
            arg_ptr = builder.CreateBitCast(arg_ptr, elem_ptr_type, "openarray.elem");
            llvm::dbgs() << "openarray get_index arg_ptr_type(after cast)=";
            arg_ptr->getType()->print(llvm::dbgs());
            llvm::dbgs() << " index_count=" << index.size() << "\n";
        });
        last_value = builder.CreateGEP(elem_type, arg_ptr, index, "idx");
        return;
    }
    if (ast->ident->id->is(Attr::ptr) || is_string_type) {
        if (llvm::isa<AllocaInst>(arg_ptr) || llvm::isa<GlobalVariable>(arg_ptr)) {
            debug("Create load");
            arg_ptr = builder.CreateLoad(arg_ptr->getType(), arg_ptr);
        }
    }
    debug("get_index: GEP number of indices: {0}", index.size());
    debug("get_index: basetype: {0}", std::string(*ast->ident->get_type()));
    LLVM_DEBUG({
        llvm::dbgs() << "get_index non-openarray GEP base_type=";
        ast->ident->get_type()->get_llvm()->print(llvm::dbgs());
        llvm::dbgs() << " arg_ptr_type=";
        arg_ptr->getType()->print(llvm::dbgs());
        llvm::dbgs() << " index_count=" << index.size() << "\n";
        for (size_t idx = 0; idx < index.size(); ++idx) {
            llvm::dbgs() << "  idx[" << idx << "]=";
            index[idx]->getType()->print(llvm::dbgs());
            llvm::dbgs() << "\n";
        }
    });
    auto *gep_type = ast->ident->get_type()->get_llvm();
    if (is_string_type) {
        gep_type = TypeTable::CharType->get_llvm();
    } else if (implicit_ptr_deref) {
        if (auto ptr_type = std::dynamic_pointer_cast<PointerType>(ident_type)) {
            gep_type = ptr_type->get_reference()->get_llvm();
        }
    }
    last_value = builder.CreateGEP(gep_type, arg_ptr, index, "idx");
}

/**
 * @brief Used to fetch values
 *
 * @param ast
 * @param ptr
 */
void CodeGenerator::visitPtr(ASTDesignator const &ast, const bool ptr) {
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
        debug("ASTDesignator: load {}", ast->get_type()->get_name());
        const auto resolved_type = types.resolve(ast->get_type()->get_name());
        last_value = builder.CreateLoad(resolved_type->get_llvm(), last_value, "idx");
    }
}

void CodeGenerator::visitPtr(ASTQualident const &ast, const bool ptr) {
    debug("ASTQualident");
    if (!ast->qual.empty()) {
        // modify the AST
        ast->id->value = ast->make_coded_id();
    }
    visitPtr(ast->id, ptr);
}

void CodeGenerator::visitPtr(ASTIdentifier const &ast, const bool ptr) {
    debug("ASTIdentifierPtr {0} {1}", ast->value, ptr);
    if (const auto res = symboltable.find(ast->value); res) {
        if (!ptr && res->type->id == TypeId::ENUMERATION) {
            if (const auto enum_type = std::dynamic_pointer_cast<EnumType>(res->type)) {
                if (const auto ordinal = enum_type->get_ordinal(ast->value); ordinal) {
                    last_value = enum_type->make_value(*ordinal);
                    return;
                }
            }
        }

        last_value = res->value;

        if (res->is(Attr::var)) {
            // VAR parameters are stored as pointers in the symbol table.
            auto *ptr_value = builder.CreateLoad(last_value->getType(), last_value, ast->value);
            if (is_var || ptr) {
                last_value = ptr_value;
                return;
            }
            debug("ASTIdentifierPtr VAR load {}", res->type->get_name());
            const auto type = types.resolve(res->type->get_name());
            last_value = builder.CreateLoad(type->get_llvm(), ptr_value, ast->value);
            return;
        }

        if (is_var) {
            // This is a write of a non-VAR variable, preserve this value
            last_value = res->value;
            return;
        }

        if (!ptr) {
            debug("ASTIdentifierPtr !ptr type: {}", res->type->get_name());
            const auto type = types.resolve(res->type->get_name());
            last_value = builder.CreateLoad(type->get_llvm(), last_value, ast->value);
        }
        return;
    }
}

void CodeGenerator::visit(ASTSet const &ast) {
    Value *set_value = TypeTable::SetType->get_init();
    auto  *one = TypeTable::IntType->make_value(1);
    for (auto const &e : ast->values) {
        std::visit(overloaded{[this, &set_value, ast, one](ASTSimpleExpr const &exp) {
                                  debug("ASTSet exp");
                                  visit(exp);
                                  auto *index = builder.CreateShl(one, last_value);
                                  set_value = builder.CreateOr(set_value, index);
                              },
                              [this, &set_value, ast, one](ASTRange const &exp) {
                                  debug("ASTSet range");
                                  visit(exp->first);
                                  auto *first = last_value;
                                  visit(exp->last);
                                  auto *last = last_value;
                                  set_value =
                                      call_function("Set_range", TypeTable::SetType->get_llvm(),
                                                    {set_value, first, last});
                              }},
                   e);
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
        const auto char_val = ast->value[0];
        last_value = TypeTable::CharType->make_value(char_val);
        return;
    }

    GlobalVariable *var = nullptr;
    if (const auto res = global_strings.find(ast->value); res != global_strings.end()) {
        var = res->second;
    } else {
        const std::string name = std::format("STRING_{0}", string_const++);
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
 * @param type
 * @param var
 * @return AllocaInst*
 */
AllocaInst *CodeGenerator::createEntryBlockAlloca(Function *function, std::string const &name,
                                                  const ASTType &type, const bool var) {
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

Type CodeGenerator::resolve_type(ASTType const &t) const {
    // debug("resolve_type {0}", std::string(*t));
    Type result;
    std::visit(overloaded{[this, &result](ASTQualident const &type) {
                              result = types.resolve(type->id->value);
                              // should be a resolved type this far down
                              assert(result && "Type not found");
                          },
                          [t, &result, this](auto /* not used*/) {
                              result = types.resolve(t->get_type()->get_name());
                          }},
               t->type);
    // debug("resolve_type to {0}", std::string(*result));
    return result;
}

llvm::Type *CodeGenerator::getType(ASTType const &type) const {
    // debug("getType {0}", std::string(*t));
    return resolve_type(type)->get_llvm();
}

Constant *CodeGenerator::getType_init(ASTType const &type) const {
    // debug("getType_init {0}", std::string(*t));
    return resolve_type(type)->get_init();
}

void CodeGenerator::setup_builtins() const {
    debug("setup_builtins");

    for (const auto &key : Builtin::global_functions | std::views::keys) {
        if (const auto res = symboltable.find(key);
            res->is(Attr::used) && res->is(Attr::global_function)) {
            auto *funcType = dyn_cast<FunctionType>(res->type->get_llvm());

            auto *func = Function::Create(funcType, Function::LinkageTypes::ExternalLinkage, key,
                                          module.get());
            verifyFunction(*func);
            symboltable.set_value(key, func);
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

std::string CodeGenerator::get_nested_name() const {
    const auto *insert{""};
    std::string result{};
    for (auto const &name : nested_procs) {
        result += insert + name;
        insert = "_";
    }
    return result;
}

GlobalVariable *CodeGenerator::generate_global(std::string const &name, llvm::Type *t) const {
    module->getOrInsertGlobal(name, t);
    return module->getNamedGlobal(name);
}

FunctionCallee CodeGenerator::generate_function(std::string const &name, llvm::Type *return_type,
                                                llvm::ArrayRef<llvm::Type *> const &params) const {
    llvm::FunctionType *ft = llvm::FunctionType::get(return_type, params, false);
    return module->getOrInsertFunction(name, ft);
}

void CodeGenerator::init() {
    module = std::make_unique<Module>(module_name, context);
    module->setSourceFileName(options.file_name);
}

void CodeGenerator::optimize() const {
    PassBuilder passBuilder;

    LoopAnalysisManager     loopAnalysisManager; // * add constructor true for debug info
    FunctionAnalysisManager functionAnalysisManager;
    CGSCCAnalysisManager    cGSCCAnalysisManager;
    ModuleAnalysisManager   moduleAnalysisManager;

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
    default:;
    }

    ModulePassManager modulePassManager = passBuilder.buildPerModuleDefaultPipeline(opt_level);
    modulePassManager.run(*module, moduleAnalysisManager);
}

void CodeGenerator::generate_llcode() const {
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

void CodeGenerator::generate_objectcode() const {
    debug("generate_objectcode");

    // Define the target triple
    auto targetTriple = getDefaultTargetTriple();

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
    raw_fd_ostream  dest_file(f, EC, fs::OF_None);

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
