//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "printer.hh"

#include <algorithm>

#include "astmod.hh"

namespace ax {

void ASTVisitor::visit_ASTModule(ASTModule *ast) {
    ast->decs->accept(this);
    std::for_each(ast->procedures.begin(), ast->procedures.end(),
                  [this](auto const &proc) { proc->accept(this); });
    std::for_each(ast->stats.begin(), ast->stats.end(),
                  [this](auto const &x) { x->accept(this); });
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
        std::for_each(ast->consts.begin(), ast->consts.end(),
                      [this](auto const &c) {
                          c.ident->accept(this);
                          c.value->accept(this);
                      });
    }
}

void ASTVisitor::visit_ASTVar(ASTVar *ast) {
    if (!ast->vars.empty()) {
        std::for_each(ast->vars.begin(), ast->vars.end(),
                      [this](auto const &v) { v.first->accept(this); });
    }
}

void ASTVisitor::visit_ASTProcedure(ASTProcedure *ast) {
    ast->decs->accept(this);
    std::for_each(ast->stats.begin(), ast->stats.end(),
                  [this](auto const &x) { x->accept(this); });
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

void ASTVisitor::visit_ASTIf(ASTIf *ast) {
    ast->if_clause.expr->accept(this);
    std::for_each(ast->if_clause.stats.begin(), ast->if_clause.stats.end(),
                  [this](auto const &x) { x->accept(this); });

    std::for_each(ast->elsif_clause.begin(), ast->elsif_clause.end(),
                  [this](auto const &x) {
                      x.expr->accept(this);
                      std::for_each(x.stats.begin(), x.stats.end(),
                                    [this](auto const &s) { s->accept(this); });
                  });
    if (ast->else_clause) {
        auto elses = *ast->else_clause;
        std::for_each(begin(elses), end(elses),
                      [this](auto const &s) { s->accept(this); });
    }
}

void ASTVisitor::visit_ASTFor(ASTFor *ast) {
    ast->ident->accept(this);
    ast->start->accept(this);
    ast->end->accept(this);
    if (ast->by) {
        (*ast->by)->accept(this);
    }
    std::for_each(begin(ast->stats), end(ast->stats),
                  [this](auto const &x) { x->accept(this); });
}

void ASTVisitor::visit_ASTExpr(ASTExpr *ast) {
    ast->expr->accept(this);
    if (*ast->relation_expr) {
        (*ast->relation_expr)->accept(this);
    }
}

void ASTVisitor::visit_ASTSimpleExpr(ASTSimpleExpr *ast) {
    visit_ASTTerm(ast->term.get());
    std::for_each(ast->rest.begin(), ast->rest.end(),
                  [this](auto t) { t.second->accept(this); });
}

void ASTVisitor::visit_ASTTerm(ASTTerm *ast) {
    visit_ASTFactor(ast->factor.get());
    std::for_each(ast->rest.begin(), ast->rest.end(),
                  [this](auto t) { t.second->accept(this); });
}

void ASTVisitor::visit_ASTFactor(ASTFactor *ast) {
    // Visit the appropriate variant
    std::visit([this](auto &&arg) { arg->accept(this); }, ast->factor);
}

void ASTVisitor::visit_ASTIdentifier(ASTIdentifier *ast) {}

void ASTVisitor::visit_ASTInteger(ASTInteger *ast) {}

void ASTVisitor::visit_ASTBool(ASTBool *ast){};

} // namespace ax