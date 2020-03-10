//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <iostream>

#include "ast.hh"
#include "astvisitor.hh"
#include "symbol.hh"
#include "symboltable.hh"
#include "typetable.hh"

namespace ax {

class Inspector : public ASTVisitor {

  public:
    explicit Inspector(std::shared_ptr<SymbolTable<Symbol>> const &s,
                       TypeTable &                                 t)
        : top_symboltable(s), current_symboltable(s), types(t){};

    void visit_ASTModule(ASTModule *ast) override;
    void visit_ASTConst(ASTConst *ast) override;
    void visit_ASTVar(ASTVar *ast) override;
    void visit_ASTProcedure(ASTProcedure *ast) override;
    void visit_ASTAssignment(ASTAssignment *ast) override;
    void visit_ASTReturn(ASTReturn *ast) override;
    void visit_ASTCall(ASTCall *ast) override;
    void visit_ASTIf(ASTIf *ast) override;
    void visit_ASTFor(ASTFor *ast) override;
    void visit_ASTSimpleExpr(ASTSimpleExpr *ast) override;
    void visit_ASTExpr(ASTExpr *ast) override;
    void visit_ASTTerm(ASTTerm *ast) override;
    void visit_ASTFactor(ASTFactor *ast) override;
    void visit_ASTIdentifier(ASTIdentifier *ast) override;
    void visit_ASTInteger(ASTInteger *ast) override;
    void visit_ASTBool(ASTBool *ast) override;

    void check(std::shared_ptr<ASTModule> const &ast) {
        visit_ASTModule(ast.get());
    };

  private:
    std::shared_ptr<SymbolTable<Symbol>> top_symboltable;
    std::shared_ptr<SymbolTable<Symbol>> current_symboltable;

    TypeTable &types;

    bool          has_return = false;
    TypePtr       last_type = nullptr;
    ASTProcedure *last_proc = nullptr;
};

} // namespace ax
