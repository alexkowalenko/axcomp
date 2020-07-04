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
#include "lexerUTF8.hh"
#include "location.hh"
#include "symbol.hh"
#include "token.hh"
#include "type.hh"

namespace ax {

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

    [[nodiscard]] TypePtr get_type() const { return type_info; };
    void                  set_type(TypePtr const &t) { type_info = t; }

    explicit operator std::string();

  private:
    Location location;
    TypePtr  type_info{nullptr}; // store information about the type
};
using ASTBasePtr = std::shared_ptr<ASTBase>;

template <class T, typename... Rest> auto make(Rest... rest) {
    return std::make_shared<T>(rest...);
}

////////////////
// Basic Objects

class ASTNil : public ASTBase, public std::enable_shared_from_this<ASTNil> {
  public:
    ~ASTNil() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTNil(shared_from_this()); };
};
using ASTNilPtr = std::shared_ptr<ASTNil>;

class ASTInteger : public ASTBase, public std::enable_shared_from_this<ASTInteger> {
  public:
    ~ASTInteger() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTInteger(shared_from_this()); };

    Int  value{0};
    bool hex{false};
};
using ASTIntegerPtr = std::shared_ptr<ASTInteger>;

class ASTReal : public ASTBase, public std::enable_shared_from_this<ASTReal> {
  public:
    ~ASTReal() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTReal(shared_from_this()); };

    Real        value{0.0};
    std::string style;
};
using ASTRealPtr = std::shared_ptr<ASTReal>;

class ASTBool : public ASTBase, public std::enable_shared_from_this<ASTBool> {
  public:
    ~ASTBool() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTBool(shared_from_this()); };

    Bool value{false};
};
using ASTBoolPtr = std::shared_ptr<ASTBool>;

/**
 * @brief digit {hexDigit} "X" | "’" char "’"
 *
 */
class ASTChar : public ASTBase, public std::enable_shared_from_this<ASTChar> {
  public:
    ~ASTChar() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTChar(shared_from_this()); };

    std::string str() const {
        std::string s{};
        Character32::add_string(s, value);
        return s;
    }

    Char value = 0;
    bool hex{false};
};
using ASTBCharPtr = std::shared_ptr<ASTChar>;

class ASTString : public ASTBase, public std::enable_shared_from_this<ASTString> {
  public:
    ~ASTString() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTString(shared_from_this()); };

    std::string str() const { return delim + value + delim; }

    std::string value;
    char        delim{'"'};
};
using ASTStringPtr = std::shared_ptr<ASTString>;

/**
 * @brief "{"" [ element {, element}] "}""
 *
 * element = expr [".." expr]
 *
 */

class ASTSet : public ASTBase, public std::enable_shared_from_this<ASTSet> {
  public:
    ~ASTSet() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTSet(shared_from_this()); };

    std::vector<std::variant<ASTSimpleExprPtr, ASTRangePtr>> values;
};
using ASTSetPtr = std::shared_ptr<ASTSet>;

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
 * @brief Qualident | arrayType | recordType | pointerType
 *
 */

class ASTType : public ASTBase, public std::enable_shared_from_this<ASTType> {
  public:
    ~ASTType() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTType(shared_from_this()); };

    std::variant<ASTQualidentPtr, ASTArrayPtr, ASTRecordPtr, ASTPointerTypePtr> type;
};
using ASTTypePtr = std::shared_ptr<ASTType>;

/**
 * @brief  "ARRAY" [ expr ( , expr ) ] "OF" type
 *
 */

class ASTArray : public ASTBase, public std::enable_shared_from_this<ASTArray> {
  public:
    ~ASTArray() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTArray(shared_from_this()); };

    std::vector<ASTIntegerPtr> dimensions;
    ASTTypePtr                 type;
};
using ASTArrayPtr = std::shared_ptr<ASTArray>;

/**
 * @brief "RECORD" "(" qualident ")" fieldList ( ";" fieldList )* "END"
 *
 */

using VarDec = std::pair<ASTIdentifierPtr, ASTTypePtr>;

class ASTRecord : public ASTBase, public std::enable_shared_from_this<ASTRecord> {
  public:
    ~ASTRecord() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTRecord(shared_from_this()); };

    ASTQualidentPtr     base = nullptr;
    std::vector<VarDec> fields;
};
using ASTRecordPtr = std::shared_ptr<ASTRecord>;

/**
 * @brief "POINTER" "TO" type
 *
 */

class ASTPointerType : public ASTBase, public std::enable_shared_from_this<ASTPointerType> {
  public:
    ~ASTPointerType() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTPointerType(shared_from_this()); };

    ASTTypePtr reference;
};
using ASTPointerTypePtr = std::shared_ptr<ASTPointerType>;

/////////////////////
// Expression Objects

/**
 * @brief IDENT selector
 *
 * selector = ( '[' exprList ']' | '.' IDENT | "^")*
 *
 * exprList = simpleExpr {"," simpleExpr}.
 *
 */

using FieldRef = std::pair<ASTIdentifierPtr, int>;
using ArrayRef = std::vector<ASTSimpleExprPtr>;
using PointerRef = bool;

class ASTDesignator : public ASTBase, public std::enable_shared_from_this<ASTDesignator> {
  public:
    ~ASTDesignator() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTDesignator(shared_from_this()); };

    ASTQualidentPtr                                           ident;
    std::vector<std::variant<ArrayRef, FieldRef, PointerRef>> selectors;
};
using ASTDesignatorPtr = std::shared_ptr<ASTDesignator>;

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
class ASTFactor : public ASTBase, public std::enable_shared_from_this<ASTFactor> {
  public:
    ~ASTFactor() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTFactor(shared_from_this()); };

    std::variant<ASTDesignatorPtr, ASTIntegerPtr, ASTRealPtr, ASTExprPtr, ASTCallPtr, ASTBoolPtr,
                 ASTCharPtr, ASTStringPtr, ASTSetPtr, ASTNilPtr, ASTFactorPtr>
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
 * @brief range -> simpleExpr .. simpleExpr
 *
 */
class ASTRange : public ASTBase, public std::enable_shared_from_this<ASTRange> {
  public:
    ~ASTRange() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTRange(shared_from_this()); };

    ASTSimpleExprPtr first;
    ASTSimpleExprPtr last;
};
using ASTRangePtr = std::shared_ptr<ASTRange>;

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

    ASTSimpleExprPtr         expr;
    std::optional<TokenType> relation;
    ASTSimpleExprPtr         relation_expr{nullptr};
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
 * @brief
 *
 */
class ASTCaseElement : public ASTStatement, public std::enable_shared_from_this<ASTCaseElement> {
  public:
    ~ASTCaseElement() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTCaseElement(shared_from_this()); };

    std::vector<std::variant<ASTSimpleExprPtr, ASTRangePtr>> exprs;
    std::vector<ASTStatementPtr>                             stats;
};
using ASTCaseElementPtr = std::shared_ptr<ASTCaseElement>;

/**
 * @brief  CASE Expression OF Case {"|" Case} [ELSE StatementSequence] END.
 *
 */
class ASTCase : public ASTStatement, public std::enable_shared_from_this<ASTCase> {
  public:
    ~ASTCase() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTCase(shared_from_this()); };

    ASTSimpleExprPtr               expr;
    std::vector<ASTCaseElementPtr> elements;
    std::vector<ASTStatementPtr>   else_stats;
};
using ASTCasePtr = std::shared_ptr<ASTCase>;

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
    ASTExprPtr                   by{nullptr};
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

using RecVar = std::pair<ASTIdentifierPtr, ASTIdentifierPtr>;

class ASTProc : public ASTBase {
  public:
    ASTIdentifierPtr    name;
    ASTTypePtr          return_type{nullptr};
    RecVar              receiver{nullptr, nullptr};
    std::vector<VarDec> params;
};
using ASTProcPtr = std::shared_ptr<ASTProc>;

class ASTProcedureForward : public ASTProc,
                            public std::enable_shared_from_this<ASTProcedureForward> {
  public:
    ~ASTProcedureForward() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTProcedureForward(shared_from_this()); };
};
using ASTProcedureForwardPtr = std::shared_ptr<ASTProcedureForward>;

class ASTProcedure : public ASTProc, public std::enable_shared_from_this<ASTProcedure> {
  public:
    ~ASTProcedure() override = default;

    void accept(ASTVisitor *v) override {
        v->visit_ASTProcedure(ASTProcedure::shared_from_this());
    };

    ASTDeclarationPtr            decs;
    std::vector<ASTProcPtr>      procedures;
    std::vector<ASTStatementPtr> stats;

    std::vector<std::pair<std::string, Attrs>> free_variables;
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
    std::vector<ASTProcPtr>      procedures;
    std::vector<ASTStatementPtr> stats;
};
using ASTModulePtr = std::shared_ptr<ASTModule>;

template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

} // namespace ax
