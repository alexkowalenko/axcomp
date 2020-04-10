//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "printer.hh"

#include <algorithm>

#include "ast.hh"

namespace ax {

void ASTVisitor::visit_ASTModule(ASTModule *ast) {
    ast->decs->accept(this);
    std::for_each(begin(ast->procedures), end(ast->procedures),
                  [this](auto const &proc) { proc->accept(this); });
    std::for_each(begin(ast->stats), end(ast->stats),
                  [this](auto const &x) { x->accept(this); });
}

void ASTVisitor::visit_ASTImport(ASTImport *ast) {
    std::for_each(begin(ast->imports), end(ast->imports),
                  [this](auto const &i) {
                      i.first->accept(this);
                      if (i.second) {
                          i.second->accept(this);
                      }
                  });
};

void ASTVisitor::visit_ASTDeclaration(ASTDeclaration *ast) {
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

void ASTVisitor::visit_ASTConst(ASTConst *ast) {
    std::for_each(begin(ast->consts), end(ast->consts), [this](auto const &c) {
        c.ident->accept(this);
        c.value->accept(this);
    });
}

void ASTVisitor::visit_ASTTypeDec(ASTTypeDec * /*unused*/) {}

void ASTVisitor::visit_ASTVar(ASTVar *ast) {

    std::for_each(begin(ast->vars), end(ast->vars),
                  [this](auto const &v) { v.first->accept(this); });
}

void ASTVisitor::visit_ASTProcedure(ASTProcedure *ast) {
    ast->return_type->accept(this);
    ast->decs->accept(this);
    std::for_each(begin(ast->stats), end(ast->stats),
                  [this](auto const &x) { x->accept(this); });
}

void ASTVisitor::visit_ASTAssignment(ASTAssignment *ast) {
    ast->ident->accept(this);
    ast->expr->accept(this);
}

void ASTVisitor::visit_ASTReturn(ASTReturn *ast) {
    if (ast->expr) {
        ast->expr->accept(this);
    }
}

void ASTVisitor::visit_ASTExit(ASTExit *ast) {}

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

void ASTVisitor::visit_ASTWhile(ASTWhile *ast) {
    ast->expr->accept(this);
    std::for_each(begin(ast->stats), end(ast->stats),
                  [this](auto const &x) { x->accept(this); });
}

void ASTVisitor::visit_ASTRepeat(ASTRepeat *ast) {
    ast->expr->accept(this);
    std::for_each(begin(ast->stats), end(ast->stats),
                  [this](auto const &x) { x->accept(this); });
}

void ASTVisitor::visit_ASTLoop(ASTLoop *ast) {
    std::for_each(begin(ast->stats), end(ast->stats),
                  [this](auto const &x) { x->accept(this); });
}

void ASTVisitor::visit_ASTBlock(ASTBlock *ast) {
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
    ast->term->accept(this);
    std::for_each(ast->rest.begin(), ast->rest.end(),
                  [this](auto t) { t.second->accept(this); });
}

void ASTVisitor::visit_ASTTerm(ASTTerm *ast) {
    ast->factor->accept(this);
    std::for_each(ast->rest.begin(), ast->rest.end(),
                  [this](auto t) { t.second->accept(this); });
}

void ASTVisitor::visit_ASTFactor(ASTFactor *ast) {
    // Visit the appropriate variant
    std::visit([this](auto &&arg) { arg->accept(this); }, ast->factor);
}

void ASTVisitor::visit_ASTDesignator(ASTDesignator *ast) {
    ast->ident->accept(this);
    std::for_each(
        begin(ast->selectors), end(ast->selectors), [this](auto const &arg) {
            std::visit(overloaded{[this](std::shared_ptr<ASTExpr> const &s) {
                                      s->accept(this);
                                  },
                                  [this](FieldRef const &s) {
                                      s.first->accept(this);
                                  }},
                       arg);
        });
}

void ASTVisitor::visit_ASTQualident(ASTQualident *ast) {}

void ASTVisitor::visit_ASTIdentifier(ASTIdentifier *ast) {}

void ASTVisitor::visit_ASTType(ASTType *ast) {
    // Visit the appropriate type
    std::visit([this](auto &&arg) { arg->accept(this); }, ast->type);
}

void ASTVisitor::visit_ASTArray(ASTArray *ast) {
    ast->size->accept(this);
    ast->type->accept(this);
};

void ASTVisitor::visit_ASTRecord(ASTRecord *ast) {
    std::for_each(begin(ast->fields), end(ast->fields), [this](auto const &s) {
        s.first->accept(this);
        s.second->accept(this);
    });
}

void ASTVisitor::visit_ASTInteger(ASTInteger *ast) {}

void ASTVisitor::visit_ASTBool(ASTBool *ast){};

} // namespace ax