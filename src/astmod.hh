//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "ast.hh"
#include "astvisitor.hh"
#include "token.hh"

namespace ax {

class ASTInteger : public ASTBase {
  public:
    ~ASTInteger(){};

    void accept(ASTVisitor *v) { v->visit_ASTInteger(this); };

    long value;
};

struct Term_mult {
    TokenType                   sign;
    std::shared_ptr<ASTInteger> integer;
};

/**
 * @brief term -> INTEGER ( ( '*' | 'DIV' | 'MOD' ) INTEGER)*
 *
 */
class ASTTerm : public ASTBase {
  public:
    ~ASTTerm(){};

    void accept(ASTVisitor *v) { v->visit_ASTTerm(this); };

    std::shared_ptr<ASTInteger> integer;
    std::vector<Term_mult>      rest;
};

struct Expr_add {
    TokenType                sign;
    std::shared_ptr<ASTTerm> term;
};

/**
 * @brief expr -> ('+' | '-' )? term ( ('+' | '-' ) term)*
 *
 */
class ASTExpr : public ASTBase {
  public:
    ~ASTExpr(){};

    void accept(ASTVisitor *v) { v->visit_ASTExpr(this); };

    std::optional<TokenType> first_sign;
    std::shared_ptr<ASTTerm> term;
    std::vector<Expr_add>    rest;
};

class ASTModule : public ASTBase {
  public:
    ~ASTModule(){};
    void accept(ASTVisitor *v) { v->visit_ASTModule(this); };

    std::string                           name;
    std::vector<std::shared_ptr<ASTExpr>> exprs;
};
} // namespace ax