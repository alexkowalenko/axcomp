//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <memory>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include "astvisitor.hh"

using namespace llvm;

namespace ax {

class CodeGenerator : ASTVisitor {
  public:
    CodeGenerator();

    void generate(std::shared_ptr<ASTModule> ast) {
        visit_ASTModule(ast.get());
    };

    void visit_ASTModule(ASTModule *ast);
    void visit_ASTExpr(ASTExpr *ast);
    void visit_ASTInteger(ASTInteger *ast);

  private:
    void init();
    void generate_objectcode();
    void print_code();

    std::string             filename;
    LLVMContext             context;
    IRBuilder<>             builder;
    std::unique_ptr<Module> module;

    Value *last_value; // holds last value of compilation
};

} // namespace ax