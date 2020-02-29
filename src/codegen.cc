//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "codegen.hh"

#include <iostream>

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

using namespace llvm::sys;

inline const std::string file_ext_llvmri{".ll"};
inline const std::string file_ext_obj{".o"};

CodeGenerator::CodeGenerator(Options &o)
    : options(o), symboltable(nullptr), filename("output"), builder(context),
      last_value(nullptr){};

void CodeGenerator::visit_ASTModule(ASTModule *ast) {
    // Set up code generation
    init(ast->name);

    // Top level consts
    // if (ast->decs->cnst) {
    //     doTopConsts(ast->decs->cnst.get());
    // }

    // Do procedures which generate functions first
    doProcedures(ast->procedures);

    // Set up the module as a function
    // Make the function type:  (void) : int
    std::vector<Type *> proto;
    FunctionType *      ft =
        FunctionType::get(Type::getInt64Ty(context), proto, false);

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
    visit_ASTDeclaration(ast->decs.get());

    // Go through the expressions
    for (auto const &x : ast->stats) {
        x->accept(this);
    }

    // Put in return statement at end of function, in case it is missing.
    // builder.CreateRet(last_value);

    // Validate the generated code, checking for consistency.
    verifyFunction(*f);

    // change the filename to generate module.obj
    filename = ast->name;
    if (!options.only_ll) {
        generate_objectcode();
    }
    print_code();
}

void CodeGenerator::doProcedures(
    std::vector<std::shared_ptr<ASTProcedure>> const &procs) {
    for (auto const &proc : procs) {
        visit_ASTProcedure(proc.get());
    }
}

void CodeGenerator::visit_ASTDeclaration(ASTDeclaration *ast) {
    if (ast->cnst) {
        visit_ASTConst(ast->cnst.get());
    }
    if (ast->var) {
        visit_ASTVar(ast->var.get());
    }
}

void CodeGenerator::doTopConsts(ASTConst *ast) {
    for (auto const &c : ast->consts) {
        module->getOrInsertGlobal(c.ident->value, builder.getInt64Ty());
        GlobalVariable *gVar = module->getNamedGlobal(c.ident->value);
        gVar->setLinkage(GlobalValue::LinkageTypes::InternalLinkage);
        gVar->setInitializer(ConstantInt::get(context, APInt(64, 0, true)));
        gVar->setAlignment(8);
        gVar->setConstant(true);

        symboltable.put(c.ident->value, gVar);
    }
}

void CodeGenerator::visit_ASTConst(ASTConst *ast) {
    for (auto const &c : ast->consts) {
        visit_ASTExpr(c.expr.get());
        auto val = last_value;

        auto name = c.ident->value;
        debug("create const: {}", name);

        // Create variable for module

        auto function = builder.GetInsertBlock()->getParent();
        auto alloc = createEntryBlockAlloca(function, name);
        builder.CreateStore(val, alloc);

        symboltable.put(name, alloc);

        // GlobalVariable *gVar = module->getNamedGlobal(name);
        // builder.CreateStore(val, gVar);
    }
    debug("finish const");
}

void CodeGenerator::visit_ASTVar(ASTVar *ast) {
    for (auto const &c : ast->vars) {

        // Create variable for module
        auto name = c.ident->value;
        debug("create var: {}", name);

        auto function = builder.GetInsertBlock()->getParent();
        auto alloc = createEntryBlockAlloca(function, name);
        builder.CreateStore(ConstantInt::get(context, APInt(64, 0, true)),
                            alloc);
        alloc->setName(name);
        debug("set name: {}", name);
        symboltable.put(name, alloc);
    }
    debug("finish var");
}

void CodeGenerator::visit_ASTProcedure(ASTProcedure *ast) {

    // Make the function type:  (void) : void
    // Change later when implement parameters
    std::vector<Type *> proto;

    FunctionType *ft =
        FunctionType::get(Type::getVoidTy(context), proto, false);

    Function *f = Function::Create(ft, Function::ExternalLinkage, ast->name,
                                   module.get());

    // Create a new basic block to start insertion into.
    BasicBlock *block = BasicBlock::Create(context, "entry", f);
    builder.SetInsertPoint(block);

    // Do declarations
    visit_ASTDeclaration(ast->decs.get());

    // Go through the expressions
    for (auto const &x : ast->stats) {
        x->accept(this);
    }

    // Put in return statement at end of function, in case it is missing.
    // builder.CreateRet(last_value);

    // Validate the generated code, checking for consistency.
    verifyFunction(*f);
}

void CodeGenerator::visit_ASTAssignment(ASTAssignment *ast) {
    visit_ASTExpr(ast->expr.get());
    auto val = last_value;

    auto var = symboltable.find(ast->ident->value);
    if (!var) {
        throw CodeGenException(
            fmt::format("identifier: {} not found." + ast->ident->value));
    }
    debug("CodeGenerator::visit_ASTAssignment value: {}", val->getName().str());
    builder.CreateStore(val, var.value());
}

void CodeGenerator::visit_ASTReturn(ASTReturn *ast) {
    visit_ASTExpr(ast->expr.get());
    builder.CreateRet(last_value);
}

void CodeGenerator::visit_ASTExpr(ASTExpr *expr) {

    visit_ASTTerm(expr->term.get());
    Value *L = last_value;
    // if initial sign exists and is negative, negate the integer
    if (expr->first_sign && expr->first_sign.value() == TokenType::dash) {
        L = builder.CreateSub(ConstantInt::get(context, APInt(64, 0, true)), L,
                              "negtmp");
        last_value = L;
    }

    for (auto t : expr->rest) {
        visit_ASTTerm(t.term.get());
        Value *R = last_value;
        switch (t.sign) {
        case TokenType::plus:
            last_value = builder.CreateAdd(L, R, "addtmp");
            break;
        case TokenType::dash:
            last_value = builder.CreateSub(L, R, "subtmp");
            break;
        default:
            throw CodeGenException("ASTExpr with sign" + string(t.sign));
        }

        L = last_value;
    }
}

void CodeGenerator::visit_ASTTerm(ASTTerm *ast) {
    visit_ASTFactor(ast->factor.get());
    Value *L = last_value;
    for (auto t : ast->rest) {
        visit_ASTFactor(t.factor.get());
        Value *R = last_value;
        switch (t.sign) {
        case TokenType::asterisk:
            last_value = builder.CreateMul(L, R, "multmp");
            break;
        case TokenType::div:
            last_value = builder.CreateSDiv(L, R, "divtmp");
            break;
        case TokenType::mod:
            last_value = builder.CreateSRem(L, R, "modtmp");
            break;
        default:
            throw CodeGenException("ASTTerm with sign" + string(t.sign));
        }

        L = last_value;
    }
}

void CodeGenerator::visit_ASTFactor(ASTFactor *ast) {
    if (ast->integer) {
        visit_ASTInteger(ast->integer.get());
    } else if (ast->expr) {
        visit_ASTExpr(ast->expr.get());
    } else if (ast->identifier) {
        visit_ASTIdentifier(ast->identifier.get());
    }
}

void CodeGenerator::visit_ASTInteger(ASTInteger *ast) {
    last_value = ConstantInt::get(context, APInt(64, ast->value, true));
}

void CodeGenerator::visit_ASTIdentifier(ASTIdentifier *ast) {
    if (auto res = symboltable.find(ast->value)) {
        last_value = builder.CreateLoad(*res, ast->value);
        return;
    }
    throw CodeGenException(fmt::format("identifier {} unknown", ast->value));
}

/**
 * @brief Create an alloca instruction in the entry block of the function.  This
 * is used for mutable variables etc.
 *
 * @param function
 * @param name
 * @return AllocaInst*
 */
AllocaInst *CodeGenerator::createEntryBlockAlloca(Function *   function,
                                                  std::string &name) {
    IRBuilder<> TmpB(&function->getEntryBlock(),
                     function->getEntryBlock().begin());
    return TmpB.CreateAlloca(Type::getInt64Ty(context), nullptr, name);
}

void CodeGenerator::init(std::string const &module_name) {
    module = std::make_unique<Module>(module_name, context);
    module->setSourceFileName(module_name);
}

void CodeGenerator::print_code() {
    auto            f = filename + file_ext_llvmri;
    std::error_code EC;
    raw_fd_ostream  out_file(f, EC, sys::fs::OF_None);

    if (EC) {
        throw CodeGenException("Could not open file: " + EC.message(), 0);
    }
    module->print(out_file, nullptr);
}

void CodeGenerator::generate_objectcode() {
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