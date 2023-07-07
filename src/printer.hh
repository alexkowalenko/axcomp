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

    void print(ASTModule const &ast) { ast->accept(this); };

    void visit_ASTModule(ASTModule ast) override;
    void visit_ASTImport(ASTImport ast) override;
    void visit_ASTConst(ASTConst ast) override;
    void visit_ASTTypeDec(ASTTypeDec ast) override;
    void visit_ASTVar(ASTVar ast) override;
    void proc_rec(RecVar const &r);
    void proc_header(ASTProc_ const &ast, bool forward);
    void visit_ASTProcedure(ASTProcedure ast) override;
    void visit_ASTProcedureForward(ASTProcedureForward ast) override;
    void visit_ASTAssignment(ASTAssignment ast) override;
    void visit_ASTReturn(ASTReturn ast) override;
    void visit_ASTExit(ASTExit ast) override;
    void visit_ASTCall(ASTCall ast) override;
    void print_stats(std::vector<ASTStatement> stats);
    void visit_ASTIf(ASTIf ast) override;
    void visit_ASTCaseElement(ASTCaseElement ast) override;
    void visit_ASTCase(ASTCase ast) override;
    void visit_ASTFor(ASTFor ast) override;
    void visit_ASTWhile(ASTWhile ast) override;
    void visit_ASTRepeat(ASTRepeat ast) override;
    void visit_ASTLoop(ASTLoop ast) override;
    void visit_ASTBlock(ASTBlock ast) override;
    void visit_ASTExpr(ASTExpr ast) override;
    void visit_ASTRange(ASTRange ast) override;
    void visit_ASTSimpleExpr(ASTSimpleExpr ast) override;
    void visit_ASTTerm(ASTTerm ast) override;
    void visit_ASTFactor(ASTFactor ast) override;
    void visit_ASTDesignator(ASTDesignator ast) override;
    void visit_ASTType(ASTType ast) override;
    void visit_ASTArray(ASTArray ast) override;
    void visit_ASTRecord(ASTRecord ast) override;
    void visit_ASTPointerType(ASTPointerType ast) override;
    void visit_ASTIdentifier(ASTIdentifier ast) override;
    void visit_ASTQualident(ASTQualident ast) override;
    void visit_ASTSet(ASTSet ast) override;
    void visit_ASTInteger(ASTInteger ast) override;
    void visit_ASTReal(ASTReal ast) override;
    void visit_ASTChar(ASTCharPtr ast) override;
    void visit_ASTString(ASTString ast) override;
    void visit_ASTBool(ASTBool ast) override;
    void visit_ASTNil(ASTNil /*not used*/) override;

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
