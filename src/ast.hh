//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <cstddef>
#include <memory>
#include <optional>
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

class ASTBase_ {
  public:
    ASTBase_() = default;
    virtual ~ASTBase_() = default;

    ASTBase_(ASTBase_ const &) = default;
    ASTBase_ &operator=(ASTBase_ const &) = default;

    ASTBase_(ASTBase_ &&) = default;
    ASTBase_ &operator=(ASTBase_ &&) = default;

    virtual void accept(ASTVisitor *v) = 0;

    void            set_location(Location const &l) { location = l; };
    Location const &get_location() { return location; };

    [[nodiscard]] Type get_type() const { return type_info; };
    void               set_type(Type const &t) { type_info = t; }

    explicit operator std::string();

  private:
    Location location;
    Type     type_info{nullptr}; // store information about the type
};
using ASTBase = std::shared_ptr<ASTBase_>;

template <class T, typename... Rest> auto make(Rest... rest) {
    return std::make_shared<T>(rest...);
}

///////////////////////////////////////////////////////////////////////////////
// Basic Objects

class ASTNil_ : public ASTBase_, public std::enable_shared_from_this<ASTNil_> {
  public:
    ~ASTNil_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTNil(shared_from_this()); };
};
using ASTNil = std::shared_ptr<ASTNil_>;

class ASTInteger_ : public ASTBase_, public std::enable_shared_from_this<ASTInteger_> {
  public:
    ~ASTInteger_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTInteger(shared_from_this()); };

    Int  value{0};
    bool hex{false};
};
using ASTInteger = std::shared_ptr<ASTInteger_>;

class ASTReal_ : public ASTBase_, public std::enable_shared_from_this<ASTReal_> {
  public:
    ~ASTReal_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTReal(shared_from_this()); };

    Real        value{0.0};
    std::string style;
};
using ASTReal = std::shared_ptr<ASTReal_>;

class ASTBool_ : public ASTBase_, public std::enable_shared_from_this<ASTBool_> {
  public:
    ~ASTBool_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTBool(shared_from_this()); };

    Bool value{false};
};
using ASTBool = std::shared_ptr<ASTBool_>;

/**
 * @brief digit {hexDigit} "X" | "’" char "’"
 *
 */
class ASTChar_ : public ASTBase_, public std::enable_shared_from_this<ASTChar_> {
  public:
    ~ASTChar_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTChar(shared_from_this()); };

    std::string str() const {
        std::string s{};
        Character32::add_string(s, value);
        return s;
    }

    Char value = 0;
    bool hex{false};
};
using ASTChar = std::shared_ptr<ASTChar_>;

class ASTString_ : public ASTBase_, public std::enable_shared_from_this<ASTString_> {
  public:
    ~ASTString_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTString(shared_from_this()); };

    std::string str() const { return delim + value + delim; }

    std::string value;
    char        delim{'"'};
};
using ASTString = std::shared_ptr<ASTString_>;

/**
 * @brief "{"" [ element {, element}] "}""
 *
 * element = expr [".." expr]
 *
 */

class ASTSet_ : public ASTBase_, public std::enable_shared_from_this<ASTSet_> {
  public:
    ~ASTSet_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTSet(shared_from_this()); };

    std::vector<std::variant<ASTSimpleExpr, ASTRange>> values;
};
using ASTSet = std::shared_ptr<ASTSet_>;

class ASTIdentifier_ : public ASTBase_, public std::enable_shared_from_this<ASTIdentifier_> {
  public:
    ASTIdentifier_() = default;
    explicit ASTIdentifier_(std::string n) : value(std::move(n)){};
    ~ASTIdentifier_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTIdentifier(shared_from_this()); };

    [[nodiscard]] bool is(Attr attr) const { return attrs.contains(attr); }
    void               set(Attr attr) { attrs.set(attr); }

    explicit virtual operator std::string() { return value; };

    std::string value;
    Attrs       attrs;
};
using ASTIdentifier = std::shared_ptr<ASTIdentifier_>;

/**
 * @brief Qualident = [ident "."] ident.
 *
 */
class ASTQualident_ : public ASTBase_, public std::enable_shared_from_this<ASTQualident_> {
  public:
    ASTQualident_() = default;
    explicit ASTQualident_(std::string &n) { id = make<ASTIdentifier_>(n); };
    ~ASTQualident_() override = default;

    ASTQualident_(ASTQualident_ const &o) = default;
    ASTQualident_ &operator=(ASTQualident_ const &other) = default;

    void accept(ASTVisitor *v) override { v->visit_ASTQualident(shared_from_this()); };

    std::string                     qual;
    std::shared_ptr<ASTIdentifier_> id = nullptr;

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
using ASTQualident = std::shared_ptr<ASTQualident_>;

/**
 * @brief Qualident | arrayType | recordType | pointerType
 *
 */

class ASTType_ : public ASTBase_, public std::enable_shared_from_this<ASTType_> {
  public:
    ~ASTType_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTType(shared_from_this()); };

    std::variant<ASTQualident, ASTArray, ASTRecord, ASTPointerType> type;
};
using ASTType = std::shared_ptr<ASTType_>;

/**
 * @brief  "ARRAY" [ expr ( , expr ) ] "OF" type
 *
 */

class ASTArray_ : public ASTBase_, public std::enable_shared_from_this<ASTArray_> {
  public:
    ~ASTArray_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTArray(shared_from_this()); };

    std::vector<ASTInteger> dimensions;
    ASTType                 type;
};
using ASTArray = std::shared_ptr<ASTArray_>;

/**
 * @brief "RECORD" "(" qualident ")" fieldList ( ";" fieldList )* "END"
 *
 */

using VarDec = std::pair<ASTIdentifier, ASTType>;

class ASTRecord_ : public ASTBase_, public std::enable_shared_from_this<ASTRecord_> {
  public:
    ~ASTRecord_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTRecord(shared_from_this()); };

    ASTQualident        base = nullptr;
    std::vector<VarDec> fields;
};
using ASTRecord = std::shared_ptr<ASTRecord_>;

/**
 * @brief "POINTER" "TO" type
 *
 */

class ASTPointerType_ : public ASTBase_, public std::enable_shared_from_this<ASTPointerType_> {
  public:
    ~ASTPointerType_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTPointerType(shared_from_this()); };

    ASTType reference;
};
using ASTPointerType = std::shared_ptr<ASTPointerType_>;

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

    void accept(ASTVisitor *v) override { v->visit_ASTDesignator(shared_from_this()); };

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

    void accept(ASTVisitor *v) override { v->visit_ASTFactor(shared_from_this()); };

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

    void accept(ASTVisitor *v) override { v->visit_ASTTerm(shared_from_this()); };

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

    void accept(ASTVisitor *v) override { v->visit_ASTSimpleExpr(shared_from_this()); };

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

    void accept(ASTVisitor *v) override { v->visit_ASTRange(shared_from_this()); };

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

    void accept(ASTVisitor *v) override { v->visit_ASTExpr(shared_from_this()); };

    ASTSimpleExpr            expr;
    std::optional<TokenType> relation;
    ASTSimpleExpr            relation_expr{nullptr};
};
using ASTExpr = std::shared_ptr<ASTExpr_>;

////////////////////
// Statement objects

class ASTStatement_ : public ASTBase_ {};
using ASTStatement = std::shared_ptr<ASTStatement_>;

/**
 * @brief designator ":=" expr
 *
 */
class ASTAssignment_ : public ASTStatement_, public std::enable_shared_from_this<ASTAssignment_> {
  public:
    ~ASTAssignment_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTAssignment(shared_from_this()); };

    ASTDesignator ident;
    ASTExpr       expr;
};
using ASTAssignment = std::shared_ptr<ASTAssignment_>;

/**
 * @brief RETURN [expr]
 *
 */
class ASTReturn_ : public ASTStatement_, public std::enable_shared_from_this<ASTReturn_> {
  public:
    ~ASTReturn_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTReturn(shared_from_this()); };

    ASTExpr expr;
};
using ASTReturn = std::shared_ptr<ASTReturn_>;

/**
 * @brief EXIT
 *
 */
class ASTExit_ : public ASTStatement_, public std::enable_shared_from_this<ASTExit_> {
  public:
    ~ASTExit_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTExit(shared_from_this()); };

    ASTExpr expr;
};
using ASTExit = std::shared_ptr<ASTExit_>;

/**
 * @brief designator "(" expr ( "," expr )* ")"
 *
 */
class ASTCall_ : public ASTStatement_, public std::enable_shared_from_this<ASTCall_> {
  public:
    ~ASTCall_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTCall(shared_from_this()); };

    ASTDesignator        name;
    std::vector<ASTExpr> args;
};
using ASTCall = std::shared_ptr<ASTCall_>;

/**
 * @brief "IF" expression "THEN" statement_seq
 *
 * ( "ELSIF" expression "THEN" statement_seq )*
 *
 * [ "ELSE" statement_seq ] "END"
 *
 */
class ASTIf_ : public ASTStatement_, public std::enable_shared_from_this<ASTIf_> {
  public:
    ~ASTIf_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTIf(shared_from_this()); };

    struct IFClause {
        ASTExpr                   expr;
        std::vector<ASTStatement> stats;
    };

    IFClause                                 if_clause;
    std::vector<IFClause>                    elsif_clause;
    std::optional<std::vector<ASTStatement>> else_clause;
};
using ASTIf = std::shared_ptr<ASTIf_>;

/**
 * @brief
 *
 */
class ASTCaseElement_ : public ASTStatement_,
                        public std::enable_shared_from_this<ASTCaseElement_> {
  public:
    ~ASTCaseElement_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTCaseElement(shared_from_this()); };

    std::vector<std::variant<ASTSimpleExpr, ASTRange>> exprs;
    std::vector<ASTStatement>                          stats;
};
using ASTCaseElement = std::shared_ptr<ASTCaseElement_>;

/**
 * @brief  CASE Expression OF Case {"|" Case} [ELSE StatementSequence] END.
 *
 */
class ASTCase_ : public ASTStatement_, public std::enable_shared_from_this<ASTCase_> {
  public:
    ~ASTCase_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTCase(shared_from_this()); };

    ASTSimpleExpr               expr;
    std::vector<ASTCaseElement> elements;
    std::vector<ASTStatement>   else_stats;
};
using ASTCase = std::shared_ptr<ASTCase_>;

/**
 * @brief "FOR" IDENT ":=" expr "TO" expr [ "BY" INTEGER ] "DO"
    statement_seq "END"
 *
 */
class ASTFor_ : public ASTStatement_, public std::enable_shared_from_this<ASTFor_> {
  public:
    ~ASTFor_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTFor(shared_from_this()); };

    ASTIdentifier             ident;
    ASTExpr                   start;
    ASTExpr                   end;
    ASTExpr                   by{nullptr};
    std::vector<ASTStatement> stats;
};
using ASTFor = std::shared_ptr<ASTFor_>;

/**
 * @brief "WHILE" expr "DO" statement_seq "END"
 *
 */
class ASTWhile_ : public ASTStatement_, public std::enable_shared_from_this<ASTWhile_> {
  public:
    ~ASTWhile_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTWhile(shared_from_this()); };

    ASTExpr                   expr;
    std::vector<ASTStatement> stats;
};
using ASTWhile = std::shared_ptr<ASTWhile_>;

/**
 * @brief "REPEAT" statement_seq "UNTIL" expr
 *
 */
class ASTRepeat_ : public ASTStatement_, public std::enable_shared_from_this<ASTRepeat_> {
  public:
    ~ASTRepeat_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTRepeat(shared_from_this()); };

    ASTExpr                   expr;
    std::vector<ASTStatement> stats;
};
using ASTRepeat = std::shared_ptr<ASTRepeat_>;

/**
 * @brief "LOOP" statement_seq "END"
 *
 */
class ASTLoop_ : public ASTStatement_, public std::enable_shared_from_this<ASTLoop_> {
  public:
    ~ASTLoop_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTLoop(shared_from_this()); };

    std::vector<ASTStatement> stats;
};
using ASTLoop = std::shared_ptr<ASTLoop_>;

/**
 * @brief "BEGIN" statement_seq "END"
 *
 */
class ASTBlock_ : public ASTStatement_, public std::enable_shared_from_this<ASTBlock_> {
  public:
    ~ASTBlock_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTBlock(shared_from_this()); };

    std::vector<ASTStatement> stats;
};
using ASTBlock = std::shared_ptr<ASTBlock_>;

//////////////////////
// Declaration objects

using RecVar = std::pair<ASTIdentifier, ASTIdentifier>;

class ASTProc_ : public ASTBase_ {
  public:
    ASTIdentifier       name;
    ASTType             return_type{nullptr};
    RecVar              receiver{nullptr, nullptr};
    std::vector<VarDec> params;
};
using ASTProc = std::shared_ptr<ASTProc_>;

class ASTProcedureForward_ : public ASTProc_,
                             public std::enable_shared_from_this<ASTProcedureForward_> {
  public:
    ~ASTProcedureForward_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTProcedureForward(shared_from_this()); };
};
using ASTProcedureForward = std::shared_ptr<ASTProcedureForward_>;

class ASTProcedure_ : public ASTProc_, public std::enable_shared_from_this<ASTProcedure_> {
  public:
    ~ASTProcedure_() override = default;

    void accept(ASTVisitor *v) override {
        v->visit_ASTProcedure(ASTProcedure_::shared_from_this());
    };

    ASTDeclaration            decs;
    std::vector<ASTProc>      procedures;
    std::vector<ASTStatement> stats;

    std::vector<std::pair<std::string, Attrs>> free_variables;
};
using ASTProcedure = std::shared_ptr<ASTProcedure_>;

/**
 * @brief "VAR" (IDENT ":" type ";")*
 *
 */
class ASTVar_ : public ASTBase_, public std::enable_shared_from_this<ASTVar_> {
  public:
    ~ASTVar_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTVar(shared_from_this()); };

    std::vector<VarDec> vars;
};
using ASTVar = std::shared_ptr<ASTVar_>;

/**
 * @brief "TYPE" (IDENT "=" type ";")*
 *
 */
class ASTTypeDec_ : public ASTBase_, public std::enable_shared_from_this<ASTTypeDec_> {
  public:
    ~ASTTypeDec_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTTypeDec(shared_from_this()); };

    std::vector<VarDec> types;
};
using ASTTypeDec = std::shared_ptr<ASTTypeDec_>;

struct ConstDec {
    ASTIdentifier ident;
    ASTExpr       value;
    ASTType       type;
};

/**
 * @brief "CONST" (IDENT "=" INTEGER ";")*
 *
 */
class ASTConst_ : public ASTBase_, public std::enable_shared_from_this<ASTConst_> {
  public:
    ~ASTConst_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTConst(shared_from_this()); };

    std::vector<ConstDec> consts;
};
using ASTConst = std::shared_ptr<ASTConst_>;

class ASTDeclaration_ : public ASTBase_, public std::enable_shared_from_this<ASTDeclaration_> {
  public:
    ~ASTDeclaration_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTDeclaration(shared_from_this()); };

    ASTTypeDec type;
    ASTConst   cnst;
    ASTVar     var;
};
using ASTDeclaration = std::shared_ptr<ASTDeclaration_>;

/**
 * @brief "IMPORT" Import {"," Import} ";".
 *
 * Import = = [ident ":="] ident.
 *
 */
class ASTImport_ : public ASTBase_, public std::enable_shared_from_this<ASTImport_> {
  public:
    ~ASTImport_() override = default;

    void accept(ASTVisitor *v) override { v->visit_ASTImport(shared_from_this()); };

    using Pair = std::pair<ASTIdentifier,  // Module
                           ASTIdentifier>; // Alias

    std::vector<Pair> imports;
};
using ASTImport = std::shared_ptr<ASTImport_>;

class ASTModule_ : public ASTBase_, public std::enable_shared_from_this<ASTModule_> {
  public:
    ~ASTModule_() override = default;
    void accept(ASTVisitor *v) override { v->visit_ASTModule(shared_from_this()); };

    std::string               name;
    ASTImport                 import;
    ASTDeclaration            decs;
    std::vector<ASTProc>      procedures;
    std::vector<ASTStatement> stats;
};
using ASTModule = std::shared_ptr<ASTModule_>;

template <class... Ts> struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

} // namespace ax
