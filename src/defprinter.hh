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

    void print(ASTModulePtr const &ast) { ast->accept(this); };

  private:
    void visit_ASTModule(ASTModulePtr ast) override;

    void visit_ASTConst(ASTConstPtr ast) override;
    void visit_ASTTypeDec(ASTTypeDecPtr ast) override;
    void visit_ASTVar(ASTVarPtr ast) override;
    void visit_ASTProcedure(ASTProcedurePtr ast) override;
};

} // namespace ax
