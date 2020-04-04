//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <iostream>

#include "ast.hh"
#include "astvisitor.hh"
#include "error.hh"
#include "symboltable.hh"
#include "typetable.hh"

namespace ax {

class Inspector : public ASTVisitor {

  public:
    explicit Inspector(std::shared_ptr<SymbolTable<TypePtr>> const &s,
                       TypeTable &t, ErrorManager &e);

    void visit_ASTModule(ASTModule *ast) override;
    void visit_ASTConst(ASTConst *ast) override;
    void visit_ASTTypeDec(ASTTypeDec *ast) override;
    void visit_ASTVar(ASTVar *ast) override;
    void visit_ASTProcedure(ASTProcedure *ast) override;
    void visit_ASTAssignment(ASTAssignment *ast) override;
    void visit_ASTReturn(ASTReturn *ast) override;
    void visit_ASTCall(ASTCall *ast) override;
    void visit_ASTIf(ASTIf *ast) override;
    void visit_ASTFor(ASTFor *ast) override;
    void visit_ASTWhile(ASTWhile *ast) override;
    void visit_ASTRepeat(ASTRepeat *ast) override;
    void visit_ASTLoop(ASTLoop *ast) override;
    void visit_ASTBlock(ASTBlock *ast) override;
    void visit_ASTSimpleExpr(ASTSimpleExpr *ast) override;
    void visit_ASTExpr(ASTExpr *ast) override;
    void visit_ASTTerm(ASTTerm *ast) override;
    void visit_ASTFactor(ASTFactor *ast) override;
    void visit_ASTDesignator(ASTDesignator *ast) override;
    void visit_ASTType(ASTType *ast) override;
    void visit_ASTArray(ASTArray *ast) override;
    void visit_ASTRecord(ASTRecord *ast) override;
    void visit_ASTIdentifier(ASTIdentifier *ast) override;
    void visit_ASTInteger(ASTInteger *ast) override;
    void visit_ASTBool(ASTBool *ast) override;

    void check(std::shared_ptr<ASTModule> const &ast) {
        visit_ASTModule(ast.get());
    };

  private:
    std::shared_ptr<SymbolTable<TypePtr>> top_symboltable;
    std::shared_ptr<SymbolTable<TypePtr>> current_symboltable;

    std::shared_ptr<SymbolTable<TypePtr>> top_consts;
    std::shared_ptr<SymbolTable<TypePtr>> current_consts;

    TypeTable &   types;
    ErrorManager &errors;

    bool          is_const{false};
    TypePtr       last_type{nullptr};
    ASTProcedure *last_proc{nullptr};
};

} // namespace ax
