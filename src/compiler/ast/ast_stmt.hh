//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <optional>
#include <variant>
#include <vector>

#include "ast_expr.hh"

namespace ax {

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

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

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

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

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

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

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

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

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

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

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

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

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

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

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

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

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

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

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

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

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

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

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

    void accept(ASTVisitor *v) override { v->visit(shared_from_this()); };

    std::vector<ASTStatement> stats;
};
using ASTBlock = std::shared_ptr<ASTBlock_>;

} // namespace ax
