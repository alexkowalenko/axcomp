//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "printer.hh"

#include <algorithm>

#include "ast.hh"

namespace ax {

void ASTVisitor::visit(ASTModule const &ast) { // NOLINT
    ast->decs->accept(this);
    for (auto const &proc : ast->procedures) {
        proc->accept(this);
    }
    for (auto const &stat : ast->stats) {
        stat->accept(this);
    }
}

void ASTVisitor::visit(ASTImport const &ast) { // NOLINT
    for (const auto &[module, alias] : ast->imports) {
        module->accept(this);
        if (alias) {
            alias->accept(this);
        }
    }
};

void ASTVisitor::visit(ASTDeclaration const &ast) { // NOLINT
    if (ast->cnst) {
        ast->cnst->accept(this);
    }
    if (ast->type) {
        ast->type->accept(this);
    }
    if (ast->var) {
        ast->var->accept(this);
    }
}

void ASTVisitor::visit(ASTConst const &ast) { // NOLINT
    for (auto const &cnst : ast->consts) {
        cnst.ident->accept(this);
        cnst.value->accept(this);
    }
}

void ASTVisitor::visit(ASTTypeDec const & /*unused*/) {} // NOLINT

void ASTVisitor::visit(ASTVar const &ast) { // NOLINT

    for (auto const &var : ast->vars) {
        var.first->accept(this);
    }
}

void ASTVisitor::visit(ASTProcedure const &ast) { // NOLINT
    ast->return_type->accept(this);
    ast->decs->accept(this);
    for (auto const &stat : ast->stats) {
        stat->accept(this);
    }
}

void ASTVisitor::visit(ASTProcedureForward const & /*not used*/) {} // NOLINT

void ASTVisitor::visit(ASTAssignment const &ast) { // NOLINT
    ast->ident->accept(this);
    ast->expr->accept(this);
}

void ASTVisitor::visit(ASTReturn const &ast) { // NOLINT
    if (ast->expr) {
        ast->expr->accept(this);
    }
}

void ASTVisitor::visit(ASTExit const &ast) {} // NOLINT

void ASTVisitor::visit(ASTCall const &ast) { // NOLINT
    ast->name->accept(this);
}

void ASTVisitor::visit(ASTIf const &ast) { // NOLINT
    ast->if_clause.expr->accept(this);
    for (auto const &s : ast->if_clause.stats) {
        s->accept(this);
    }

    for (auto const &elsif_clause : ast->elsif_clause) {
        elsif_clause.expr->accept(this);
        for (auto const &stmt : elsif_clause.stats) {
            stmt->accept(this);
        }
    }
    if (ast->else_clause) {
        auto elses = *ast->else_clause;
        for (auto const &stmt : elses) {
            stmt->accept(this);
        }
    }
}

void ASTVisitor::visit(ASTCaseElement const &ast) {}; // NOLINT

void ASTVisitor::visit(ASTCase const &ast) {}; // NOLINT

void ASTVisitor::visit(ASTFor const &ast) { // NOLINT
    ast->ident->accept(this);
    ast->start->accept(this);
    ast->end->accept(this);
    if (ast->by) {
        ast->by->accept(this);
    }
    for (auto const &stat : ast->stats) {
        stat->accept(this);
    }
}

void ASTVisitor::visit(ASTWhile const &ast) { // NOLINT
    ast->expr->accept(this);
    for (auto const &stat : ast->stats) {
        stat->accept(this);
    }
}

void ASTVisitor::visit(ASTRepeat const &ast) { // NOLINT
    ast->expr->accept(this);
    for (auto const &stat : ast->stats) {
        stat->accept(this);
    }
}

void ASTVisitor::visit(ASTLoop const &ast) { // NOLINT
    for (auto const &stat : ast->stats) {
        stat->accept(this);
    }
}

void ASTVisitor::visit(ASTBlock const &ast) { // NOLINT
    for (auto const &stat : ast->stats) {
        stat->accept(this);
    }
}

void ASTVisitor::visit(ASTExpr const &ast) { // NOLINT
    ast->expr->accept(this);
    if (ast->relation_expr) {
        ast->relation_expr->accept(this);
    }
}

void ASTVisitor::visit(ASTRange const &ast) { // NOLINT
    ast->first->accept(this);
    ast->last->accept(this);
};

void ASTVisitor::visit(ASTSimpleExpr const &ast) { // NOLINT
    ast->term->accept(this);
    for (auto const &rest : ast->rest) {
        rest.second->accept(this);
    }
}

void ASTVisitor::visit(ASTTerm const &ast) { // NOLINT
    ast->factor->accept(this);
    for (auto const &rest : ast->rest) {
        rest.second->accept(this);
    }
}

void ASTVisitor::visit(ASTFactor const &ast) { // NOLINT
    // Visit the appropriate variant
    std::visit([this](auto &&arg) { arg->accept(this); }, ast->factor);
}

void ASTVisitor::visit(ASTDesignator const &ast) { // NOLINT
    ast->ident->accept(this);
    for (auto const &arg : ast->selectors) {
        std::visit(overloaded{[this](ArrayRef const &s) {
                                  for (auto &expr : s) {
                                      expr->accept(this);
                                  }
                              },
                              [this](FieldRef const &s) { s.first->accept(this); },
                              [this](PointerRef /* unused */) {}},
                   arg);
    }
} // namespace ax

void ASTVisitor::visit(ASTType const &ast) { // NOLINT
    // Visit the appropriate type
    std::visit([this](auto &&arg) { arg->accept(this); }, ast->type);
}

void ASTVisitor::visit(ASTArray const &ast) { // NOLINT
    for (auto const &dimension : ast->dimensions) {
        dimension->accept(this);
    }
    ast->type->accept(this);
};

void ASTVisitor::visit(ASTRecord const &ast) { // NOLINT
    for (auto const &field : ast->fields) {
        field.first->accept(this);
        field.second->accept(this);
    }
}

void ASTVisitor::visit(ASTPointerType const &ast) { // NOLINT
    ast->reference->accept(this);
}

} // namespace ax
