//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "codegen.hh"

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

#include "error.hh"

namespace ax {

using namespace llvm;
using namespace llvm::sys;

void CodeGenerator::visit_ASTModule(ASTModule *) {
    // Set up code generation
    init();

    generate_objectcode();
}

void CodeGenerator::visit_ASTExpr(ASTExpr *) {}

void CodeGenerator::visit_ASTInteger(ASTInteger *) {}

void CodeGenerator::init() {}

void CodeGenerator::generate_objectcode() {

    // Define the target triple
    auto TargetTriple = sys::getDefaultTargetTriple();

    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();

    // Get a target
    std::string error;
    auto        Target = TargetRegistry::lookupTarget(TargetTriple, error);

    // Print an error and exit if we couldn't find the requested target.
    // This generally occurs if we've forgotten to initialise the
    // TargetRegistry or we have a bogus target triple.
    if (!Target) {
        throw CodeGenException(error, 0);
    }

    // Use generic CPU without features
    auto CPU = "generic";
    auto Features = "";

    TargetOptions opt;
    auto          RM = Optional<Reloc::Model>();
    auto          TargetMachine =
        Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

    auto            Filename = "output.o";
    std::error_code EC;
    raw_fd_ostream  dest_file(Filename, EC, sys::fs::OF_None);

    if (EC) {
        throw CodeGenException("Could not open file: " + EC.message(), 0);
    }

    legacy::PassManager pass;
    auto                FileType = LLVMTargetMachine::CGFT_ObjectFile;

    if (TargetMachine->addPassesToEmitFile(pass, dest_file, nullptr,
                                           FileType)) {
        throw CodeGenException("TargetMachine can't emit a file of this type",
                               0);
    }
    // pass.run(*TheModule);
    dest_file.flush();
}

} // namespace ax