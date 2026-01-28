//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <optional>
#include <variant>
#include <vector>

#include "ast_type.hh"

namespace ax {

///////////////////////////////////////////////////////////////////////////////
// Expression Objects

/**
 * @brief IDENT selector
 *
 * selector = ( '[' exprList ']' | '.' IDENT | "^")*
 *
 * exprList = simpleExpr {"," simpleExpr}.
 *
 */

using FieldRef = std::pair<ASTIdentifier, int>;
using ArrayRef = std::vector<ASTSimpleExpr>;
using PointerRef = bool;

class ASTDesignator_ : public ASTBase_, public std::enable_shared_from_this<ASTDesignator_> {
  public:
    ~ASTDesignator_() override = default;

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

    ASTIdentifier first_field() const;

    ASTQualident                                              ident;
    std::vector<std::variant<ArrayRef, FieldRef, PointerRef>> selectors;
};
using ASTDesignator = std::shared_ptr<ASTDesignator_>;

/**
 * @brief factor -> designator
 *                  | procedureCall
 *                  | INTEGER
 *                  | REAL
 *                  | "TRUE" | "FALSE"
 *                  | character
 *                  | string
 *                  | '('expr ')'
 *                  | "~" factor
 *
 */
class ASTFactor_ : public ASTBase_, public std::enable_shared_from_this<ASTFactor_> {
  public:
    ~ASTFactor_() override = default;

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

    std::variant<ASTDesignator, ASTInteger, ASTReal, ASTExpr, ASTCall, ASTBool, ASTCharPtr,
                 ASTString, ASTSet, ASTNil, ASTFactor>
         factor;
    bool is_not = false;
};
using ASTFactor = std::shared_ptr<ASTFactor_>;

/**
 * @brief term -> factor ( ( '*' | 'DIV' | 'MOD' | "&" ) factor)*
 *
 */
class ASTTerm_ : public ASTBase_, public std::enable_shared_from_this<ASTTerm_> {
  public:
    ~ASTTerm_() override = default;

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

    using Term_mult = std::pair<TokenType, ASTFactor>;

    ASTFactor              factor;
    std::vector<Term_mult> rest;
};
using ASTTerm = std::shared_ptr<ASTTerm_>;

/**
 * @brief expr -> ('+' | '-' )? term ( ('+' | '-' | "OR") term)*
 *
 */
class ASTSimpleExpr_ : public ASTBase_, public std::enable_shared_from_this<ASTSimpleExpr_> {
  public:
    ~ASTSimpleExpr_() override = default;

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

    using Expr_add = std::pair<TokenType, ASTTerm>;

    std::optional<TokenType> first_sign;
    ASTTerm                  term;
    std::vector<Expr_add>    rest;
};
using ASTSimpleExpr = std::shared_ptr<ASTSimpleExpr_>;

/**
 * @brief range -> simpleExpr .. simpleExpr
 *
 */
class ASTRange_ : public ASTBase_, public std::enable_shared_from_this<ASTRange_> {
  public:
    ~ASTRange_() override = default;

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

    ASTSimpleExpr first;
    ASTSimpleExpr last;
};
using ASTRange = std::shared_ptr<ASTRange_>;

/**
 * @brief expr = simpleExpr [ relation simpleExpr]
 *
 * relation = "=" | "#" | "<" | "<=" | ">" | ">="
 *
 */
class ASTExpr_ : public ASTBase_, public std::enable_shared_from_this<ASTExpr_> {
  public:
    ~ASTExpr_() override = default;

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

    ASTSimpleExpr            expr;
    std::optional<TokenType> relation;
    ASTSimpleExpr            relation_expr{nullptr};
};
using ASTExpr = std::shared_ptr<ASTExpr_>;

} // namespace ax
