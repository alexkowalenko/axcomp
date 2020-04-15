//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <iostream>

#include "ast.hh"
#include "printer.hh"

namespace ax {

class DefPrinter : ASTPrinter {

  public:
    explicit DefPrinter(std::ostream &ostream) : ASTPrinter(ostream){};

    void print(ASTModulePtr const &ast) { visit_ASTModule(ast.get()); };

  private:
    void visit_ASTModule(ASTModule *ast) override;

    void visit_ASTConst(ASTConst *ast) override;
    void visit_ASTTypeDec(ASTTypeDec *ast) override;
    void visit_ASTVar(ASTVar *ast) override;
    void visit_ASTProcedure(ASTProcedure *ast) override;
};

} // namespace ax
