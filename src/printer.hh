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

    void visit_ASTModule(ASTModule *);
    void visit_ASTDeclaration(ASTDeclaration *);
    void visit_ASTConst(ASTConst *);
    void visit_ASTVar(ASTVar *);
    void visit_ASTProcedure(ASTProcedure *);
    void visit_ASTAssignment(ASTAssignment *);
    void visit_ASTReturn(ASTReturn *);
    void visit_ASTExpr(ASTExpr *);
    void visit_ASTTerm(ASTTerm *);
    void visit_ASTFactor(ASTFactor *);
    void visit_ASTInteger(ASTInteger *);
    void visit_ASTIdentifier(ASTIdentifier *);

  private:
    std::ostream &os;
};

} // namespace ax
