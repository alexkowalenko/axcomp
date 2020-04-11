//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <optional>
#include <unordered_set>
#include <utility>
#include <variant>

#include "astvisitor.hh"
#include "attr.hh"
#include "location.hh"
#include "token.hh"
#include "type.hh"

namespace ax {

class ASTVisitor;

class ASTBase {
  public:
    ASTBase() = default;
    virtual ~ASTBase() = default;

    ASTBase(ASTBase const &) = default;
    ASTBase &operator=(ASTBase const &) = default;

    ASTBase(ASTBase &&) = default;
    ASTBase &operator=(ASTBase &&) = default;

    virtual void accept(ASTVisitor *v) = 0;

    void            set_location(Location const &l) { location = l; };
    Location const &get_location() { return location; };

    explicit operator std::string();

  private:
    Location location;
};

using ASTBasePtr = std::shared_ptr<ASTBase>;

////////////////
// Basic Objects

class ASTInteger : public ASTBase {
  public:
    ~ASTInteger() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTInteger(this); };

    long value{0};
};

class ASTBool : public ASTBase {
  public:
    ~ASTBool() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTBool(this); };

    bool value{false};
};

class ASTIdentifier : public ASTBase {
  public:
    ASTIdentifier() = default;
    explicit ASTIdentifier(std::string n) : value(std::move(n)){};
    ~ASTIdentifier() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTIdentifier(this); };

    [[nodiscard]] bool is(Attr attr) const { return attrs.contains(attr); }
    void               set(Attr attr) { attrs.set(attr); }

    explicit virtual operator std::string() { return value; };

    std::string value;
    Attrs       attrs;
};

/**
 * @brief Qualident = [ident "."] ident.
 *
 */
class ASTQualident : public ASTIdentifier {
  public:
    ASTQualident() = default;
    explicit ASTQualident(std::string n) : ASTIdentifier(std::move(n)){};
    ~ASTQualident() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTQualident(this); };

    std::string qual;

    explicit operator std::string() override {
        return qual.empty() ? value : qual + "." + value;
    };
};

/**
 * @brief INDENT | arrayType
 *
 */

class ASTType : public ASTBase {
  public:
    ~ASTType() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTType(this); };

    std::variant<std::shared_ptr<ASTQualident>, std::shared_ptr<ASTArray>,
                 std::shared_ptr<ASTRecord>>
            type;
    TypePtr type_info = nullptr; // store information about the type
};

/**
 * @brief  "ARRAY" "[" expr "]" "OF" type
 *
 */

class ASTArray : public ASTBase {
  public:
    ~ASTArray() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTArray(this); };

    std::shared_ptr<ASTInteger> size;
    std::shared_ptr<ASTType>    type;
};

/**
 * @brief "RECORD" fieldList ( ";" fieldList )* "END"
 *
 */

using VarDec =
    std::pair<std::shared_ptr<ASTIdentifier>, std::shared_ptr<ASTType>>;

class ASTRecord : public ASTBase {
  public:
    ~ASTRecord() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTRecord(this); };

    std::vector<VarDec> fields;
};

/////////////////////
// Expression Objects

/**
 * @brief IDENT selector
 *
 * selector = ( '[' expr ']' | '.' IDENT )*
 *
 */

using FieldRef = std::pair<std::shared_ptr<ASTIdentifier>, int>;

class ASTDesignator : public ASTBase {
  public:
    ~ASTDesignator() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTDesignator(this); };

    std::shared_ptr<ASTQualident>                                 ident;
    std::vector<std::variant<std::shared_ptr<ASTExpr>, FieldRef>> selectors;
};

/**
 * @brief factor -> designator
 *                  | procedureCall
 *                  | INTEGER
 *                  | "TRUE" | "FALSE"
 *                  | '('expr ')'
 *                  | "~" factor
 *
 */
class ASTFactor : public ASTBase {
  public:
    ~ASTFactor() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTFactor(this); };

    std::variant<std::shared_ptr<ASTDesignator>, std::shared_ptr<ASTInteger>,
                 std::shared_ptr<ASTExpr>, std::shared_ptr<ASTCall>,
                 std::shared_ptr<ASTBool>, std::shared_ptr<ASTFactor>>
         factor;
    bool is_not = false;
};

/**
 * @brief term -> factor ( ( '*' | 'DIV' | 'MOD' | "&" ) factor)*
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
 * @brief expr -> ('+' | '-' )? term ( ('+' | '-' | "OR") term)*
 *
 */
class ASTSimpleExpr : public ASTBase {
  public:
    ~ASTSimpleExpr() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTSimpleExpr(this); };

    using Expr_add = std::pair<TokenType, std::shared_ptr<ASTTerm>>;

    std::optional<TokenType> first_sign;
    std::shared_ptr<ASTTerm> term;
    std::vector<Expr_add>    rest;
};

/**
 * @brief expr = simpleExpr [ relation simpleExpr]
 *
 * relation = "=" | "#" | "<" | "<=" | ">" | ">="
 *
 */
class ASTExpr : public ASTBase {
  public:
    ~ASTExpr() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTExpr(this); };

    std::shared_ptr<ASTSimpleExpr>                expr;
    std::optional<TokenType>                      relation;
    std::optional<std::shared_ptr<ASTSimpleExpr>> relation_expr;
};

////////////////////
// Statement objects

class ASTStatement : public ASTBase {};

/**
 * @brief designator ":=" expr
 *
 */
class ASTAssignment : public ASTStatement {
  public:
    ~ASTAssignment() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTAssignment(this); };

    std::shared_ptr<ASTDesignator> ident;
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
 * @brief EXIT
 *
 */
class ASTExit : public ASTStatement {
  public:
    ~ASTExit() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTExit(this); };

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

/**
 * @brief "IF" expression "THEN" statement_seq
 *
 * ( "ELSIF" expression "THEN" statement_seq )*
 *
 * [ "ELSE" statement_seq ] "END"
 *
 */
class ASTIf : public ASTStatement {
  public:
    ~ASTIf() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTIf(this); };

    struct IFClause {
        std::shared_ptr<ASTExpr>                   expr;
        std::vector<std::shared_ptr<ASTStatement>> stats;
    };

    IFClause                                                  if_clause;
    std::vector<IFClause>                                     elsif_clause;
    std::optional<std::vector<std::shared_ptr<ASTStatement>>> else_clause;
};

/**
 * @brief "FOR" IDENT ":=" expr "TO" expr [ "BY" INTEGER ] "DO"
    statement_seq "END"
 *
 */
class ASTFor : public ASTStatement {
  public:
    ~ASTFor() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTFor(this); };

    std::shared_ptr<ASTIdentifier>             ident;
    std::shared_ptr<ASTExpr>                   start;
    std::shared_ptr<ASTExpr>                   end;
    std::optional<std::shared_ptr<ASTExpr>>    by{std::nullopt};
    std::vector<std::shared_ptr<ASTStatement>> stats;
};

/**
 * @brief "WHILE" expr "DO" statement_seq "END"
 *
 */
class ASTWhile : public ASTStatement {
  public:
    ~ASTWhile() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTWhile(this); };

    std::shared_ptr<ASTExpr>                   expr;
    std::vector<std::shared_ptr<ASTStatement>> stats;
};

/**
 * @brief "REPEAT" statement_seq "UNTIL" expr
 *
 */
class ASTRepeat : public ASTStatement {
  public:
    ~ASTRepeat() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTRepeat(this); };

    std::shared_ptr<ASTExpr>                   expr;
    std::vector<std::shared_ptr<ASTStatement>> stats;
};

/**
 * @brief "LOOP" statement_seq "END"
 *
 */
class ASTLoop : public ASTStatement {
  public:
    ~ASTLoop() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTLoop(this); };

    std::vector<std::shared_ptr<ASTStatement>> stats;
};

/**
 * @brief "BEGIN" statement_seq "END"
 *
 */
class ASTBlock : public ASTStatement {
  public:
    ~ASTBlock() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTBlock(this); };

    std::shared_ptr<ASTIdentifier>             ident;
    std::shared_ptr<ASTExpr>                   start;
    std::shared_ptr<ASTExpr>                   end;
    std::optional<std::shared_ptr<ASTExpr>>    by{std::nullopt};
    std::vector<std::shared_ptr<ASTStatement>> stats;
};

//////////////////////
// Declaration objects

class ASTProcedure : public ASTBase {
  public:
    ~ASTProcedure() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTProcedure(this); };

    std::shared_ptr<ASTIdentifier>             name;
    std::shared_ptr<ASTType>                   return_type{nullptr};
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
 * @brief "TYPE" (IDENT "=" type ";")*
 *
 */
class ASTTypeDec : public ASTBase {
  public:
    ~ASTTypeDec() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTTypeDec(this); };

    std::vector<VarDec> types;
};

struct ConstDec {
    std::shared_ptr<ASTIdentifier> ident;
    std::shared_ptr<ASTExpr>       value;
    std::shared_ptr<ASTType>       type;
};

/**
 * @brief "CONST" (IDENT "=" INTEGER ";")*
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

    std::shared_ptr<ASTTypeDec> type;
    std::shared_ptr<ASTConst>   cnst;
    std::shared_ptr<ASTVar>     var;
};

/**
 * @brief "IMPORT" Import {"," Import} ";".
 *
 * Import = = [ident ":="] ident.
 *
 */
class ASTImport : public ASTBase {
  public:
    ~ASTImport() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTImport(this); };

    using Pair = std::pair<std::shared_ptr<ASTIdentifier>,  // Module
                           std::shared_ptr<ASTIdentifier>>; // Alias

    std::vector<Pair> imports;
};

class ASTModule : public ASTBase {
  public:
    ~ASTModule() override = default;
    void accept(ASTVisitor *v) override { v->visit_ASTModule(this); };

    std::string                                name;
    std::shared_ptr<ASTImport>                 import;
    std::shared_ptr<ASTDeclaration>            decs;
    std::vector<std::shared_ptr<ASTProcedure>> procedures;
    std::vector<std::shared_ptr<ASTStatement>> stats;
};

template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

} // namespace ax
