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

    void print(ASTModule const &ast) { ast->accept(this); };

  private:
    void visit(ASTModule const &ast) override;

    void visit(ASTConst const &ast) override;
    void visit(ASTTypeDec const &ast) override;
    void visit(ASTVar const &ast) override;
    void visit(ASTProcedure const &ast) override;
};

} // namespace ax
