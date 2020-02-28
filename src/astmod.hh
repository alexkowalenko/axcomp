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

class ASTDeclaration;

////////////////
// Basic Objects

class ASTInteger : public ASTBase {
  public:
    ~ASTInteger() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTInteger(this); };

    long value;
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
    ASTFactor() : identifier(nullptr), integer(nullptr), expr(nullptr){};
    ~ASTFactor() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTFactor(this); };

    std::shared_ptr<ASTIdentifier> identifier;
    std::shared_ptr<ASTInteger>    integer;
    std::shared_ptr<ASTExpr>       expr;
};

struct Term_mult {
    TokenType                  sign;
    std::shared_ptr<ASTFactor> factor;
};

/**
 * @brief term -> factor ( ( '*' | 'DIV' | 'MOD' ) factor)*
 *
 */
class ASTTerm : public ASTBase {
  public:
    ~ASTTerm() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTTerm(this); };

    std::shared_ptr<ASTFactor> factor;
    std::vector<Term_mult>     rest;
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
    ~ASTExpr() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTExpr(this); };

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

    std::shared_ptr<ASTIdentifier> indent;
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

//////////////////////
// Declaration objects

class ASTProcedure : public ASTBase {
  public:
    ~ASTProcedure() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTProcedure(this); };

    std::string name;
    bool        is_external;
    // std::vector<Params>
    std::shared_ptr<ASTDeclaration>            decs;
    std::vector<std::shared_ptr<ASTStatement>> stats;
};

struct VarDec {
    std::shared_ptr<ASTIdentifier> indent;
    std::string                    type;
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

struct ConstDec {
    std::shared_ptr<ASTIdentifier> indent;
    std::shared_ptr<ASTExpr>       expr;
};

/**
 * @brief "CONST" (IDENT "=" expr ";")*
 *
 */
class ASTConst : public ASTBase {
  public:
    ~ASTConst() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTConst(this); };

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