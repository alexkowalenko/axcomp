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

#include "astmod.hh"
#include "error.hh"

namespace ax {

using namespace llvm::sys;

inline const std::string file_ext_llvmri = ".ll";
inline const std::string file_ext_obj = ".o";

CodeGenerator::CodeGenerator()
    : filename("output"), builder(context), last_value(nullptr){};

void CodeGenerator::visit_ASTModule(ASTModule *ast) {
    // Set up code generation
    init(ast->name);

    // Set up the module as a function
    // Make the function type:  int(void)
    std::vector<Type *> proto;
    FunctionType *      ft =
        FunctionType::get(Type::getInt64Ty(context), proto, false);

    Function *f =
        Function::Create(ft, Function::ExternalLinkage, filename, module.get());

    // Create a new basic block to start insertion into.
    BasicBlock *block = BasicBlock::Create(context, "entry", f);
    builder.SetInsertPoint(block);

    // Go through the expressions
    for (auto x : ast->exprs) {
        visit_ASTExpr(x.get());
    }
    if (last_value) {
        builder.CreateRet(last_value);
    }
    // Validate the generated code, checking for consistency.
    verifyFunction(*f);

    // change the filename to generate module.obj
    filename = ast->name;
    generate_objectcode();
    print_code();
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
            throw CodeGenException("ASTExpr with sign" + to_string(t.sign));
        }

        L = last_value;
    }
}

void CodeGenerator::visit_ASTTerm(ASTTerm *ast) {
    visit_ASTInteger(ast->integer.get());
    Value *L = last_value;
    for (auto t : ast->rest) {
        visit_ASTInteger(t.integer.get());
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
            throw CodeGenException("ASTTerm with sign" + to_string(t.sign));
        }

        L = last_value;
    }
};

void CodeGenerator::visit_ASTInteger(ASTInteger *ast) {
    last_value = ConstantInt::get(context, APInt(64, ast->value, true));
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

    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();

    // Get the target
    std::string error;
    auto        target = TargetRegistry::lookupTarget(targetTriple, error);

    // Print an error and exit if we couldn't find the requested target.
    // This generally occurs if we've forgotten to initialise the
    // TargetRegistry or we have a bogus target triple.
    if (!target) {
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