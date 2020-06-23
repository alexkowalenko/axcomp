//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <iostream>
#include <ostream>

#include "ast.hh"
#include "astvisitor.hh"
#include "defparser.hh"

namespace ax {

class ASTPrinter : public ASTVisitor {

  public:
    explicit ASTPrinter(std::ostream &ostream) : os(ostream){};

    void print(ASTModulePtr const &ast) { ast->accept(this); };

    void visit_ASTModule(ASTModulePtr ast) override;
    void visit_ASTImport(ASTImportPtr ast) override;
    void visit_ASTConst(ASTConstPtr ast) override;
    void visit_ASTTypeDec(ASTTypeDecPtr ast) override;
    void visit_ASTVar(ASTVarPtr ast) override;
    void visit_ASTProcedure(ASTProcedurePtr ast) override;
    void visit_ASTProcedureForward(ASTProcedureForwardPtr ast) override;
    void visit_ASTAssignment(ASTAssignmentPtr ast) override;
    void visit_ASTReturn(ASTReturnPtr ast) override;
    void visit_ASTExit(ASTExitPtr ast) override;
    void visit_ASTCall(ASTCallPtr ast) override;
    void print_stats(std::vector<ASTStatementPtr> stats);
    void visit_ASTIf(ASTIfPtr ast) override;
    void visit_ASTCaseElement(ASTCaseElementPtr ast) override;
    void visit_ASTCase(ASTCasePtr ast) override;
    void visit_ASTFor(ASTForPtr ast) override;
    void visit_ASTWhile(ASTWhilePtr ast) override;
    void visit_ASTRepeat(ASTRepeatPtr ast) override;
    void visit_ASTLoop(ASTLoopPtr ast) override;
    void visit_ASTBlock(ASTBlockPtr ast) override;
    void visit_ASTExpr(ASTExprPtr ast) override;
    void visit_ASTRange(ASTRangePtr ast) override;
    void visit_ASTSimpleExpr(ASTSimpleExprPtr ast) override;
    void visit_ASTTerm(ASTTermPtr ast) override;
    void visit_ASTFactor(ASTFactorPtr ast) override;
    void visit_ASTDesignator(ASTDesignatorPtr ast) override;
    void visit_ASTType(ASTTypePtr ast) override;
    void visit_ASTArray(ASTArrayPtr ast) override;
    void visit_ASTRecord(ASTRecordPtr ast) override;
    void visit_ASTPointerType(ASTPointerTypePtr ast) override;
    void visit_ASTIdentifier(ASTIdentifierPtr ast) override;
    void visit_ASTQualident(ASTQualidentPtr ast) override;
    void visit_ASTSet(ASTSetPtr ast) override;
    void visit_ASTInteger(ASTIntegerPtr ast) override;
    void visit_ASTReal(ASTRealPtr ast) override;
    void visit_ASTChar(ASTCharPtr ast) override;
    void visit_ASTString(ASTStringPtr ast) override;
    void visit_ASTBool(ASTBoolPtr ast) override;
    void visit_ASTNil(ASTNilPtr /*not used*/) override;

    void set_indent(size_t i) { indent_width = i; }

  protected:
    std::ostream &os;

    void push() { level++; }
    void pop() { level--; }

    [[nodiscard]] std::string indent() const {
        return std::string(indent_width * level, char(' '));
    }

  private:
    size_t indent_width{0};
    size_t level{0};
};

} // namespace ax
