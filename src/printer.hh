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
    explicit ASTPrinter(std::ostream &ostream) : os(ostream){};

    void print(std::shared_ptr<ASTModule> const &ast) {
        visit_ASTModule(ast.get());
    };

    void visit_ASTModule(ASTModule *ast) override;
    void visit_ASTDeclaration(ASTDeclaration *ast) override;
    void visit_ASTConst(ASTConst *ast) override;
    void visit_ASTVar(ASTVar *ast) override;
    void visit_ASTProcedure(ASTProcedure *ast) override;
    void visit_ASTAssignment(ASTAssignment *ast) override;
    void visit_ASTReturn(ASTReturn *ast) override;
    void visit_ASTCall(ASTCall *ast) override;
    void visit_ASTExpr(ASTExpr *ast) override;
    void visit_ASTTerm(ASTTerm *ast) override;
    void visit_ASTFactor(ASTFactor *ast) override;
    void visit_ASTInteger(ASTInteger *ast) override;
    void visit_ASTIdentifier(ASTIdentifier *ast) override;

  private:
    std::ostream &os;
};

} // namespace ax
