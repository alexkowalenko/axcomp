//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "printer.hh"

#include <algorithm>

#include "ast.hh"

namespace ax {

void ASTVisitor::visit_ASTModule(ASTModule ast) { // NOLINT
    ast->decs->accept(this);
    std::for_each(begin(ast->procedures), end(ast->procedures),
                  [this](auto const &proc) { proc->accept(this); });
    std::for_each(begin(ast->stats), end(ast->stats), [this](auto const &x) { x->accept(this); });
}

void ASTVisitor::visit_ASTImport(ASTImport ast) { // NOLINT
    std::for_each(begin(ast->imports), end(ast->imports), [this](auto const &i) {
        i.first->accept(this);
        if (i.second) {
            i.second->accept(this);
        }
    });
};

void ASTVisitor::visit_ASTDeclaration(ASTDeclaration ast) { // NOLINT
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

void ASTVisitor::visit_ASTConst(ASTConst ast) { // NOLINT
    std::for_each(begin(ast->consts), end(ast->consts), [this](auto const &c) {
        c.ident->accept(this);
        c.value->accept(this);
    });
}

void ASTVisitor::visit_ASTTypeDec(ASTTypeDec /*unused*/) {} // NOLINT

void ASTVisitor::visit_ASTVar(ASTVar ast) { // NOLINT

    std::for_each(begin(ast->vars), end(ast->vars),
                  [this](auto const &v) { v.first->accept(this); });
}

void ASTVisitor::visit_ASTProcedure(ASTProcedure ast) { // NOLINT
    ast->return_type->accept(this);
    ast->decs->accept(this);
    std::for_each(begin(ast->stats), end(ast->stats), [this](auto const &x) { x->accept(this); });
}

void ASTVisitor::visit_ASTProcedureForward(ASTProcedureForward /*not used*/) {} // NOLINT

void ASTVisitor::visit_ASTAssignment(ASTAssignment ast) { // NOLINT
    ast->ident->accept(this);
    ast->expr->accept(this);
}

void ASTVisitor::visit_ASTReturn(ASTReturn ast) { // NOLINT
    if (ast->expr) {
        ast->expr->accept(this);
    }
}

void ASTVisitor::visit_ASTExit(ASTExit ast) {} // NOLINT

void ASTVisitor::visit_ASTCall(ASTCall ast) { // NOLINT
    ast->name->accept(this);
}

void ASTVisitor::visit_ASTIf(ASTIf ast) { // NOLINT
    ast->if_clause.expr->accept(this);
    std::for_each(ast->if_clause.stats.begin(), ast->if_clause.stats.end(),
                  [this](auto const &x) { x->accept(this); });

    std::for_each(ast->elsif_clause.begin(), ast->elsif_clause.end(), [this](auto const &x) {
        x.expr->accept(this);
        std::for_each(x.stats.begin(), x.stats.end(), [this](auto const &s) { s->accept(this); });
    });
    if (ast->else_clause) {
        auto elses = *ast->else_clause;
        std::for_each(begin(elses), end(elses), [this](auto const &s) { s->accept(this); });
    }
}

void ASTVisitor::visit_ASTCaseElement(ASTCaseElement ast){}; // NOLINT

void ASTVisitor::visit_ASTCase(ASTCase ast){}; // NOLINT

void ASTVisitor::visit_ASTFor(ASTFor ast) { // NOLINT
    ast->ident->accept(this);
    ast->start->accept(this);
    ast->end->accept(this);
    if (ast->by) {
        ast->by->accept(this);
    }
    std::for_each(begin(ast->stats), end(ast->stats), [this](auto const &x) { x->accept(this); });
}

void ASTVisitor::visit_ASTWhile(ASTWhile ast) { // NOLINT
    ast->expr->accept(this);
    std::for_each(begin(ast->stats), end(ast->stats), [this](auto const &x) { x->accept(this); });
}

void ASTVisitor::visit_ASTRepeat(ASTRepeat ast) { // NOLINT
    ast->expr->accept(this);
    std::for_each(begin(ast->stats), end(ast->stats), [this](auto const &x) { x->accept(this); });
}

void ASTVisitor::visit_ASTLoop(ASTLoop ast) { // NOLINT
    std::for_each(begin(ast->stats), end(ast->stats), [this](auto const &x) { x->accept(this); });
}

void ASTVisitor::visit_ASTBlock(ASTBlock ast) { // NOLINT
    std::for_each(begin(ast->stats), end(ast->stats), [this](auto const &x) { x->accept(this); });
}

void ASTVisitor::visit_ASTExpr(ASTExpr ast) { // NOLINT
    ast->expr->accept(this);
    if (ast->relation_expr) {
        ast->relation_expr->accept(this);
    }
}

void ASTVisitor::visit_ASTRange(ASTRange ast) { // NOLINT
    ast->first->accept(this);
    ast->last->accept(this);
};

void ASTVisitor::visit_ASTSimpleExpr(ASTSimpleExpr ast) { // NOLINT
    ast->term->accept(this);
    std::for_each(ast->rest.begin(), ast->rest.end(), [this](auto t) { t.second->accept(this); });
}

void ASTVisitor::visit_ASTTerm(ASTTerm ast) { // NOLINT
    ast->factor->accept(this);
    std::for_each(ast->rest.begin(), ast->rest.end(), [this](auto t) { t.second->accept(this); });
}

void ASTVisitor::visit_ASTFactor(ASTFactor ast) { // NOLINT
    // Visit the appropriate variant
    std::visit([this](auto &&arg) { arg->accept(this); }, ast->factor);
}

void ASTVisitor::visit_ASTDesignator(ASTDesignator ast) { // NOLINT
    ast->ident->accept(this);
    std::for_each(begin(ast->selectors), end(ast->selectors), [this](auto const &arg) {
        std::visit(overloaded{[this](ArrayRef const &s) {
                                  std::for_each(begin(s), end(s),
                                                [this](auto &e) { e->accept(this); });
                              },
                              [this](FieldRef const &s) { s.first->accept(this); },
                              [this](PointerRef /* unused */) {}},
                   arg);
    });
} // namespace ax

void ASTVisitor::visit_ASTType(ASTType ast) { // NOLINT
    // Visit the appropriate type
    std::visit([this](auto &&arg) { arg->accept(this); }, ast->type);
}

void ASTVisitor::visit_ASTArray(ASTArray ast) { // NOLINT
    std::for_each(begin(ast->dimensions), end(ast->dimensions),
                  [this](auto &expr) { expr->accept(this); });
    ast->type->accept(this);
};

void ASTVisitor::visit_ASTRecord(ASTRecord ast) { // NOLINT
    std::for_each(begin(ast->fields), end(ast->fields), [this](auto const &s) {
        s.first->accept(this);
        s.second->accept(this);
    });
}

void ASTVisitor::visit_ASTPointerType(ASTPointerType ast) { // NOLINT
    ast->reference->accept(this);
}

} // namespace ax