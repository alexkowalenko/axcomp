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

class DefPrinter : public ASTPrinter {

  public:
    explicit DefPrinter(std::ostream &ostream) : ASTPrinter(ostream){};

    void print(std::shared_ptr<ASTModule> const &ast) {
        visit_ASTModule(ast.get());
    };

    void visit_ASTModule(ASTModule *ast);

    void visit_ASTConst(ASTConst *ast);
    void visit_ASTTypeDec(ASTTypeDec *ast);
    void visit_ASTVar(ASTVar *ast);
    void visit_ASTProcedure(ASTProcedure *ast);
};

} // namespace ax
