//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "printer.hh"

#include <fmt/core.h>

#include "astmod.hh"

namespace ax {

void ASTVisitor::visit_ASTModule(ASTModule *ast) {
    ast->decs->accept(this);
    for (auto const &proc : ast->procedures) {
        proc->accept(this);
    }
    for (auto const &x : ast->stats) {
        x->accept(this);
    }
}

void ASTVisitor::visit_ASTDeclaration(ASTDeclaration *ast) {
    if (ast->cnst) {
        ast->cnst->accept(this);
    }
    if (ast->var) {
        ast->var->accept(this);
    }
}

void ASTVisitor::visit_ASTConst(ASTConst *ast) {
    if (!ast->consts.empty()) {
        for (auto const &c : ast->consts) {
            c.first->accept(this);
            c.second->accept(this);
        }
    }
}

void ASTVisitor::visit_ASTVar(ASTVar *ast) {
    if (!ast->vars.empty()) {
        for (auto const &c : ast->vars) {
            c.first->accept(this);
        }
    }
}

void ASTVisitor::visit_ASTProcedure(ASTProcedure *ast) {
    ast->decs->accept(this);
    for (auto const &x : ast->stats) {
        x->accept(this);
    }
}

void ASTVisitor::visit_ASTAssignment(ASTAssignment *ast) {
    visit_ASTIdentifier(ast->ident.get());
    visit_ASTExpr(ast->expr.get());
}

void ASTVisitor::visit_ASTReturn(ASTReturn *ast) {
    if (ast->expr) {
        visit_ASTExpr(ast->expr.get());
    }
}

void ASTVisitor::visit_ASTCall(ASTCall *ast) {
    ast->name->accept(this);
}

void ASTVisitor::visit_ASTExpr(ASTExpr *ast) {
    visit_ASTTerm(ast->term.get());
    for (auto const &t : ast->rest) {
        visit_ASTTerm(t.term.get());
    }
}

void ASTVisitor::visit_ASTTerm(ASTTerm *ast) {
    visit_ASTFactor(ast->factor.get());
    for (auto const &t : ast->rest) {
        visit_ASTFactor(t.factor.get());
    }
}

void ASTVisitor::visit_ASTFactor(ASTFactor *ast) {
    // Visit the appropriate variant
    std::visit([this](auto &&arg) { arg->accept(this); }, ast->factor);
}

void ASTVisitor::visit_ASTInteger(ASTInteger *ast) {}

void ASTVisitor::visit_ASTIdentifier(ASTIdentifier *ast) {}

} // namespace ax