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
    explicit Inspector(SymbolTable<Symbol> &s, TypeTable &t)
        : symbols(s), types(t){};

    void visit_ASTModule(ASTModule *ast) override;

    void visit_ASTVar(ASTVar *ast) override;
    void visit_ASTProcedure(ASTProcedure *ast) override;
    void visit_ASTReturn(ASTReturn *ast) override;

    void check(std::shared_ptr<ASTModule> const &ast) {
        visit_ASTModule(ast.get());
    };

  private:
    SymbolTable<Symbol> &symbols;
    TypeTable &          types;

    bool has_return = false;
};

} // namespace ax
