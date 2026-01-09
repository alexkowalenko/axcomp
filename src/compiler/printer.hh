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

    void visit(ASTModule const &ast) override;
    void visit(ASTImport const &ast) override;
    void visit(ASTConst const &ast) override;
    void visit(ASTTypeDec const &ast) override;
    void visit(ASTVar const &ast) override;
    void proc_rec(RecVar const &r);
    void proc_header(ASTProc_ const &ast, bool forward);
    void visit(ASTProcedure const &ast) override;
    void visit(ASTProcedureForward const &ast) override;
    void visit(ASTAssignment const &ast) override;
    void visit(ASTReturn const &ast) override;
    void visit(ASTExit const &ast) override;
    void visit(ASTCall const &ast) override;
    void print_stats(std::vector<ASTStatement> stats);
    void visit(ASTIf const &ast) override;
    void visit(ASTCaseElement const &ast) override;
    void visit(ASTCase const &ast) override;
    void visit(ASTFor const &ast) override;
    void visit(ASTWhile const &ast) override;
    void visit(ASTRepeat const &ast) override;
    void visit(ASTLoop const &ast) override;
    void visit(ASTBlock const &ast) override;
    void visit(ASTExpr const &ast) override;
    void visit(ASTRange const &ast) override;
    void visit(ASTSimpleExpr const &ast) override;
    void visit(ASTTerm const &ast) override;
    void visit(ASTFactor const &ast) override;
    void visit(ASTDesignator const &ast) override;
    void visit(ASTType const &ast) override;
    void visit(ASTArray const &ast) override;
    void visit(ASTRecord const &ast) override;
    void visit(ASTPointerType const &ast) override;
    void visit(ASTIdentifier const &ast) override;
    void visit(ASTQualident const &ast) override;
    void visit(ASTSet const &ast) override;
    void visit(ASTInteger const &ast) override;
    void visit(ASTReal const &ast) override;
    void visit(ASTCharPtr const &ast) override;
    void visit(ASTString const &ast) override;
    void visit(ASTBool const &ast) override;
    void visit(ASTNil const & /*not used*/) override;

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
