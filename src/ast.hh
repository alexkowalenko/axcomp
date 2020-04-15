//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <cstddef>
#include <memory>
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

template <class T, typename... Rest> auto make(Rest... rest) {
    return std::make_shared<T>(rest...);
}

////////////////
// Basic Objects

class ASTInteger : public ASTBase, public std::enable_shared_from_this<ASTInteger> {
  public:
    ~ASTInteger() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTInteger(shared_from_this()); };

    long value{0};
};
using ASTIntegerPtr = std::shared_ptr<ASTInteger>;

class ASTBool : public ASTBase, public std::enable_shared_from_this<ASTBool> {
  public:
    ~ASTBool() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTBool(shared_from_this()); };

    bool value{false};
};
using ASTBoolPtr = std::shared_ptr<ASTBool>;

class ASTIdentifier : public ASTBase, public std::enable_shared_from_this<ASTIdentifier> {
  public:
    ASTIdentifier() = default;
    explicit ASTIdentifier(std::string n) : value(std::move(n)){};
    ~ASTIdentifier() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTIdentifier(shared_from_this()); };

    [[nodiscard]] bool is(Attr attr) const { return attrs.contains(attr); }
    void               set(Attr attr) { attrs.set(attr); }

    explicit virtual operator std::string() { return value; };

    std::string value;
    Attrs       attrs;
};
using ASTIdentifierPtr = std::shared_ptr<ASTIdentifier>;

/**
 * @brief Qualident = [ident "."] ident.
 *
 */
class ASTQualident : public ASTBase, public std::enable_shared_from_this<ASTQualident> {
  public:
    ASTQualident() = default;
    explicit ASTQualident(std::string &n) { id = make<ASTIdentifier>(n); };
    ~ASTQualident() override = default;

    ASTQualident(ASTQualident const &o) = default;
    ASTQualident &operator=(ASTQualident const &other) = default;

    void accept(ASTVisitor *v) override { v->visit_ASTQualident(shared_from_this()); };

    std::string                    qual;
    std::shared_ptr<ASTIdentifier> id = nullptr;

    static std::string make_coded_id(std::string const &q, std::string const &i) {
        return q + "_" + i;
    }
    std::string make_coded_id() const {
        return qual.empty() ? id->value : make_coded_id(qual, id->value);
    }

    explicit operator std::string() const {
        return qual.empty() ? id->value : qual + "." + id->value;
    };
};
using ASTQualidentPtr = std::shared_ptr<ASTQualident>;

/**
 * @brief INDENT | arrayType
 *
 */

class ASTType : public ASTBase, public std::enable_shared_from_this<ASTType> {
  public:
    ~ASTType() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTType(shared_from_this()); };

    std::variant<ASTQualidentPtr, ASTArrayPtr, ASTRecordPtr> type;
    TypePtr type_info = nullptr; // store information about the type
};
using ASTTypePtr = std::shared_ptr<ASTType>;

/**
 * @brief  "ARRAY" "[" expr "]" "OF" type
 *
 */

class ASTArray : public ASTBase, public std::enable_shared_from_this<ASTArray> {
  public:
    ~ASTArray() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTArray(shared_from_this()); };

    ASTIntegerPtr size;
    ASTTypePtr    type;
};
using ASTArrayPtr = std::shared_ptr<ASTArray>;

/**
 * @brief "RECORD" fieldList ( ";" fieldList )* "END"
 *
 */

using VarDec = std::pair<ASTIdentifierPtr, ASTTypePtr>;

class ASTRecord : public ASTBase, public std::enable_shared_from_this<ASTRecord> {
  public:
    ~ASTRecord() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTRecord(shared_from_this()); };

    std::vector<VarDec> fields;
};
using ASTRecordPtr = std::shared_ptr<ASTRecord>;

/////////////////////
// Expression Objects

/**
 * @brief IDENT selector
 *
 * selector = ( '[' expr ']' | '.' IDENT )*
 *
 */

using FieldRef = std::pair<ASTIdentifierPtr, int>;

class ASTDesignator : public ASTBase, public std::enable_shared_from_this<ASTDesignator> {
  public:
    ~ASTDesignator() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTDesignator(shared_from_this()); };

    ASTQualidentPtr                                 ident;
    std::vector<std::variant<ASTExprPtr, FieldRef>> selectors;
};
using ASTDesignatorPtr = std::shared_ptr<ASTDesignator>;

/**
 * @brief factor -> designator
 *                  | procedureCall
 *                  | INTEGER
 *                  | "TRUE" | "FALSE"
 *                  | '('expr ')'
 *                  | "~" factor
 *
 */
class ASTFactor : public ASTBase, public std::enable_shared_from_this<ASTFactor> {
  public:
    ~ASTFactor() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTFactor(shared_from_this()); };

    std::variant<ASTDesignatorPtr, ASTIntegerPtr, ASTExprPtr, ASTCallPtr, ASTBoolPtr, ASTFactorPtr>
         factor;
    bool is_not = false;
};
using ASTFactorPtr = std::shared_ptr<ASTFactor>;

/**
 * @brief term -> factor ( ( '*' | 'DIV' | 'MOD' | "&" ) factor)*
 *
 */
class ASTTerm : public ASTBase, public std::enable_shared_from_this<ASTTerm> {
  public:
    ~ASTTerm() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTTerm(shared_from_this()); };

    using Term_mult = std::pair<TokenType, ASTFactorPtr>;

    ASTFactorPtr           factor;
    std::vector<Term_mult> rest;
};
using ASTTermPtr = std::shared_ptr<ASTTerm>;

/**
 * @brief expr -> ('+' | '-' )? term ( ('+' | '-' | "OR") term)*
 *
 */
class ASTSimpleExpr : public ASTBase, public std::enable_shared_from_this<ASTSimpleExpr> {
  public:
    ~ASTSimpleExpr() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTSimpleExpr(shared_from_this()); };

    using Expr_add = std::pair<TokenType, ASTTermPtr>;

    std::optional<TokenType> first_sign;
    ASTTermPtr               term;
    std::vector<Expr_add>    rest;
};
using ASTSimpleExprPtr = std::shared_ptr<ASTSimpleExpr>;

/**
 * @brief expr = simpleExpr [ relation simpleExpr]
 *
 * relation = "=" | "#" | "<" | "<=" | ">" | ">="
 *
 */
class ASTExpr : public ASTBase, public std::enable_shared_from_this<ASTExpr> {
  public:
    ~ASTExpr() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTExpr(shared_from_this()); };

    ASTSimpleExprPtr                expr;
    std::optional<TokenType>        relation;
    std::optional<ASTSimpleExprPtr> relation_expr;
};
using ASTExprPtr = std::shared_ptr<ASTExpr>;

////////////////////
// Statement objects

class ASTStatement : public ASTBase {};
using ASTStatementPtr = std::shared_ptr<ASTStatement>;

/**
 * @brief designator ":=" expr
 *
 */
class ASTAssignment : public ASTStatement, public std::enable_shared_from_this<ASTAssignment> {
  public:
    ~ASTAssignment() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTAssignment(shared_from_this()); };

    ASTDesignatorPtr ident;
    ASTExprPtr       expr;
};
using ASTAssignmentPtr = std::shared_ptr<ASTAssignment>;

/**
 * @brief RETURN [expr]
 *
 */
class ASTReturn : public ASTStatement, public std::enable_shared_from_this<ASTReturn> {
  public:
    ~ASTReturn() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTReturn(shared_from_this()); };

    ASTExprPtr expr;
};
using ASTReturnPtr = std::shared_ptr<ASTReturn>;

/**
 * @brief EXIT
 *
 */
class ASTExit : public ASTStatement, public std::enable_shared_from_this<ASTExit> {
  public:
    ~ASTExit() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTExit(shared_from_this()); };

    ASTExprPtr expr;
};
using ASTExitPtr = std::shared_ptr<ASTExit>;

/**
 * @brief designator "(" expr ( "," expr )* ")"
 *
 */
class ASTCall : public ASTStatement, public std::enable_shared_from_this<ASTCall> {
  public:
    ~ASTCall() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTCall(shared_from_this()); };

    ASTDesignatorPtr        name;
    std::vector<ASTExprPtr> args;
};
using ASTCallPtr = std::shared_ptr<ASTCall>;

/**
 * @brief "IF" expression "THEN" statement_seq
 *
 * ( "ELSIF" expression "THEN" statement_seq )*
 *
 * [ "ELSE" statement_seq ] "END"
 *
 */
class ASTIf : public ASTStatement, public std::enable_shared_from_this<ASTIf> {
  public:
    ~ASTIf() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTIf(shared_from_this()); };

    struct IFClause {
        ASTExprPtr                   expr;
        std::vector<ASTStatementPtr> stats;
    };

    IFClause                                    if_clause;
    std::vector<IFClause>                       elsif_clause;
    std::optional<std::vector<ASTStatementPtr>> else_clause;
};
using ASTIfPtr = std::shared_ptr<ASTIf>;

/**
 * @brief "FOR" IDENT ":=" expr "TO" expr [ "BY" INTEGER ] "DO"
    statement_seq "END"
 *
 */
class ASTFor : public ASTStatement, public std::enable_shared_from_this<ASTFor> {
  public:
    ~ASTFor() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTFor(shared_from_this()); };

    ASTIdentifierPtr             ident;
    ASTExprPtr                   start;
    ASTExprPtr                   end;
    std::optional<ASTExprPtr>    by{std::nullopt};
    std::vector<ASTStatementPtr> stats;
};
using ASTForPtr = std::shared_ptr<ASTFor>;

/**
 * @brief "WHILE" expr "DO" statement_seq "END"
 *
 */
class ASTWhile : public ASTStatement, public std::enable_shared_from_this<ASTWhile> {
  public:
    ~ASTWhile() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTWhile(shared_from_this()); };

    ASTExprPtr                   expr;
    std::vector<ASTStatementPtr> stats;
};
using ASTWhilePtr = std::shared_ptr<ASTWhile>;

/**
 * @brief "REPEAT" statement_seq "UNTIL" expr
 *
 */
class ASTRepeat : public ASTStatement, public std::enable_shared_from_this<ASTRepeat> {
  public:
    ~ASTRepeat() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTRepeat(shared_from_this()); };

    ASTExprPtr                   expr;
    std::vector<ASTStatementPtr> stats;
};
using ASTRepeatPtr = std::shared_ptr<ASTRepeat>;

/**
 * @brief "LOOP" statement_seq "END"
 *
 */
class ASTLoop : public ASTStatement, public std::enable_shared_from_this<ASTLoop> {
  public:
    ~ASTLoop() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTLoop(shared_from_this()); };

    std::vector<ASTStatementPtr> stats;
};
using ASTLoopPtr = std::shared_ptr<ASTLoop>;

/**
 * @brief "BEGIN" statement_seq "END"
 *
 */
class ASTBlock : public ASTStatement, public std::enable_shared_from_this<ASTBlock> {
  public:
    ~ASTBlock() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTBlock(shared_from_this()); };

    std::vector<ASTStatementPtr> stats;
};
using ASTBlockPtr = std::shared_ptr<ASTBlock>;

//////////////////////
// Declaration objects

class ASTProcedure : public ASTBase, public std::enable_shared_from_this<ASTProcedure> {
  public:
    ~ASTProcedure() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTProcedure(shared_from_this()); };

    ASTIdentifierPtr             name;
    ASTTypePtr                   return_type{nullptr};
    std::vector<VarDec>          params;
    ASTDeclarationPtr            decs;
    std::vector<ASTStatementPtr> stats;
};
using ASTProcedurePtr = std::shared_ptr<ASTProcedure>;

/**
 * @brief "VAR" (IDENT ":" type ";")*
 *
 */
class ASTVar : public ASTBase, public std::enable_shared_from_this<ASTVar> {
  public:
    ~ASTVar() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTVar(shared_from_this()); };

    std::vector<VarDec> vars;
};
using ASTVarPtr = std::shared_ptr<ASTVar>;

/**
 * @brief "TYPE" (IDENT "=" type ";")*
 *
 */
class ASTTypeDec : public ASTBase, public std::enable_shared_from_this<ASTTypeDec> {
  public:
    ~ASTTypeDec() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTTypeDec(shared_from_this()); };

    std::vector<VarDec> types;
};
using ASTTypeDecPtr = std::shared_ptr<ASTTypeDec>;

struct ConstDec {
    ASTIdentifierPtr ident;
    ASTExprPtr       value;
    ASTTypePtr       type;
};

/**
 * @brief "CONST" (IDENT "=" INTEGER ";")*
 *
 */
class ASTConst : public ASTBase, public std::enable_shared_from_this<ASTConst> {
  public:
    ~ASTConst() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTConst(shared_from_this()); };

    std::vector<ConstDec> consts;
};
using ASTConstPtr = std::shared_ptr<ASTConst>;

class ASTDeclaration : public ASTBase, public std::enable_shared_from_this<ASTDeclaration> {
  public:
    ~ASTDeclaration() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTDeclaration(shared_from_this()); };

    ASTTypeDecPtr type;
    ASTConstPtr   cnst;
    ASTVarPtr     var;
};
using ASTDeclarationPtr = std::shared_ptr<ASTDeclaration>;

/**
 * @brief "IMPORT" Import {"," Import} ";".
 *
 * Import = = [ident ":="] ident.
 *
 */
class ASTImport : public ASTBase, public std::enable_shared_from_this<ASTImport> {
  public:
    ~ASTImport() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTImport(shared_from_this()); };

    using Pair = std::pair<ASTIdentifierPtr,  // Module
                           ASTIdentifierPtr>; // Alias

    std::vector<Pair> imports;
};
using ASTImportPtr = std::shared_ptr<ASTImport>;

class ASTModule : public ASTBase, public std::enable_shared_from_this<ASTModule> {
  public:
    ~ASTModule() override = default;
    void accept(ASTVisitor *v) override { v->visit_ASTModule(shared_from_this()); };

    std::string                  name;
    ASTImportPtr                 import;
    ASTDeclarationPtr            decs;
    std::vector<ASTProcedurePtr> procedures;
    std::vector<ASTStatementPtr> stats;
};
using ASTModulePtr = std::shared_ptr<ASTModule>;

template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

} // namespace ax
