//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "ast.hh"
#include "astvisitor.hh"
#include "token.hh"

namespace ax {

class ASTDeclaration;

////////////////
// Basic Objects

class ASTInteger : public ASTBase {
  public:
    ~ASTInteger() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTInteger(this); };

    long value = 0;
};

class ASTIdentifier : public ASTBase {
  public:
    ~ASTIdentifier() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTIdentifier(this); };

    std::string value;
};

/////////////////////
// Expression Objects

/**
 * @brief factor -> IDENT | INTEGER | '(' expr ')'
 *
 */
class ASTFactor : public ASTBase {
  public:
    ~ASTFactor() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTFactor(this); };

    std::variant<std::shared_ptr<ASTIdentifier>, std::shared_ptr<ASTInteger>,
                 std::shared_ptr<ASTExpr>, std::shared_ptr<ASTCall>>
        factor;
};

/**
 * @brief term -> factor ( ( '*' | 'DIV' | 'MOD' ) factor)*
 *
 */
class ASTTerm : public ASTBase {
  public:
    ~ASTTerm() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTTerm(this); };

    using Term_mult = std::pair<TokenType, std::shared_ptr<ASTFactor>>;

    std::shared_ptr<ASTFactor> factor;
    std::vector<Term_mult>     rest;
};

/**
 * @brief expr -> ('+' | '-' )? term ( ('+' | '-' ) term)*
 *
 */
class ASTExpr : public ASTBase {
  public:
    ~ASTExpr() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTExpr(this); };

    using Expr_add = std::pair<TokenType, std::shared_ptr<ASTTerm>>;

    std::optional<TokenType> first_sign;
    std::shared_ptr<ASTTerm> term;
    std::vector<Expr_add>    rest;
};

////////////////////
// Statement objects

class ASTStatement : public ASTBase {};

/**
 * @brief ident ":=" expr
 *
 */
class ASTAssignment : public ASTStatement {
  public:
    ~ASTAssignment() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTAssignment(this); };

    std::shared_ptr<ASTIdentifier> ident;
    std::shared_ptr<ASTExpr>       expr;
};

/**
 * @brief RETURN [expr]
 *
 */
class ASTReturn : public ASTStatement {
  public:
    ~ASTReturn() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTReturn(this); };

    std::shared_ptr<ASTExpr> expr;
};

/**
 * @brief IDENT "(" expr ( "," expr )* ")"
 *
 */
class ASTCall : public ASTStatement {
  public:
    ~ASTCall() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTCall(this); };

    std::shared_ptr<ASTIdentifier>        name;
    std::vector<std::shared_ptr<ASTExpr>> args;
};

//////////////////////
// Declaration objects

using VarDec = std::pair<std::shared_ptr<ASTIdentifier>, std::string>;

class ASTProcedure : public ASTBase {
  public:
    ~ASTProcedure() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTProcedure(this); };

    std::string                                name;
    bool                                       is_external = true;
    std::string                                return_type;
    std::vector<VarDec>                        params;
    std::shared_ptr<ASTDeclaration>            decs;
    std::vector<std::shared_ptr<ASTStatement>> stats;
};

/**
 * @brief "VAR" (IDENT ":" type ";")*
 *
 */
class ASTVar : public ASTBase {
  public:
    ~ASTVar() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTVar(this); };

    std::vector<VarDec> vars;
};

/**
 * @brief "CONST" (IDENT "=" INTEGER ";")*
 *
 */
class ASTConst : public ASTBase {
  public:
    ~ASTConst() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTConst(this); };

    using ConstDec =
        std::pair<std::shared_ptr<ASTIdentifier>, std::shared_ptr<ASTInteger>>;
    std::vector<ConstDec> consts;
};

class ASTDeclaration : public ASTBase {
  public:
    ~ASTDeclaration() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTDeclaration(this); };

    std::shared_ptr<ASTConst> cnst;
    std::shared_ptr<ASTVar>   var;
};

class ASTModule : public ASTBase {
  public:
    ~ASTModule() override = default;
    void accept(ASTVisitor *v) override { v->visit_ASTModule(this); };

    std::string                                name;
    std::shared_ptr<ASTDeclaration>            decs;
    std::vector<std::shared_ptr<ASTProcedure>> procedures;
    std::vector<std::shared_ptr<ASTStatement>> stats;
};
} // namespace ax