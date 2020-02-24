//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <iostream>

#include "ast.hh"
#include "astvisitor.hh"

namespace ax {

class ASTPrinter : ASTVisitor {

  public:
    ASTPrinter(std::ostream &ostream) : os(ostream){};

    void print(std::shared_ptr<ASTModule> ast) { visit_ASTModule(ast.get()); };

    void visit_ASTModule(ASTModule *ast);
    void visit_ASTExpr(ASTExpr *ast);
    void visit_ASTTerm(ASTTerm *ast);
    void visit_ASTFactor(ASTFactor *ast);
    void visit_ASTInteger(ASTInteger *ast);

  private:
    std::ostream &os;
};

} // namespace ax
