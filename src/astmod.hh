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

    void negate() { value = -value; };

    long value;
};

struct Expr_addition {
    TokenType                   sign;
    std::shared_ptr<ASTInteger> integer;
};

/**
 * @brief expr -> ('+' | '-' )? INTEGER ( ('+' | '-' ) INTEGER)*
 *
 */
class ASTExpr : public ASTBase {
  public:
    ~ASTExpr(){};

    void accept(ASTVisitor *v) { v->visit_ASTExpr(this); };

    std::optional<TokenType>    first_sign;
    std::shared_ptr<ASTInteger> integer;
    std::vector<Expr_addition>  rest;
};

class ASTModule : public ASTBase {
  public:
    ~ASTModule(){};
    void accept(ASTVisitor *v) { v->visit_ASTModule(this); };

    std::string                           name;
    std::vector<std::shared_ptr<ASTExpr>> exprs;
};
} // namespace ax