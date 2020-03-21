//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "printer.hh"

#include <algorithm>

#include <fmt/core.h>

#include "astmod.hh"
#include "token.hh"

namespace ax {

template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...)->overloaded<Ts...>;

void ASTPrinter::visit_ASTModule(ASTModule *ast) {
    os << fmt::format("MODULE {};\n", ast->name);
    ast->decs->accept(this);
    std::for_each(ast->procedures.begin(), ast->procedures.end(),
                  [this](auto const &proc) { proc->accept(this); });
    os << fmt::format("BEGIN\n");
    std::for_each(ast->stats.begin(), ast->stats.end(), [this](auto const &x) {
        x->accept(this);
        os << ";\n";
    });
    os << fmt::format("END {}.\n", ast->name);
}

void ASTPrinter::visit_ASTDeclaration(ASTDeclaration *ast) {
    if (ast->cnst) {
        ast->cnst->accept(this);
    }
    if (ast->var) {
        ast->var->accept(this);
    }
}

void ASTPrinter::visit_ASTConst(ASTConst *ast) {
    if (!ast->consts.empty()) {
        os << "CONST\n";
        std::for_each(ast->consts.begin(), ast->consts.end(),
                      [this](auto const &c) {
                          c.ident->accept(this);
                          os << " = ";
                          c.value->accept(this);
                          os << ";\n";
                      });
    }
}

void ASTPrinter::visit_ASTVar(ASTVar *ast) {
    if (!ast->vars.empty()) {
        os << "VAR\n";
        std::for_each(ast->vars.begin(), ast->vars.end(),
                      [this](auto const &v) {
                          v.first->accept(this);
                          os << ": ";
                          v.second->accept(this);
                          os << ";\n";
                      });
    }
}

void ASTPrinter::visit_ASTProcedure(ASTProcedure *ast) {
    os << fmt::format("PROCEDURE {}", ast->name);
    if (!ast->params.empty() || ast->return_type != nullptr) {
        os << "(";
        std::for_each(ast->params.begin(), ast->params.end(),
                      [this, ast](auto const &p) {
                          p.first->accept(this);
                          os << " : ";
                          p.second->accept(this);
                          if (p != *(ast->params.end() - 1)) {
                              os << "; ";
                          }
                      });
        os << ")";
    }
    if (ast->return_type != nullptr) {
        os << ": ";
        ast->return_type->accept(this);
    }
    os << ";\n";
    ast->decs->accept(this);
    os << fmt::format("BEGIN\n");
    std::for_each(ast->stats.begin(), ast->stats.end(), [this](auto const &x) {
        x->accept(this);
        os << ";\n";
    });
    os << fmt::format("END {}.\n", ast->name);
}

void ASTPrinter::visit_ASTAssignment(ASTAssignment *ast) {
    ast->ident->accept(this);
    os << " := ";
    ast->expr->accept(this);
}

void ASTPrinter::visit_ASTReturn(ASTReturn *ast) {
    os << "RETURN ";
    if (ast->expr) {
        ast->expr->accept(this);
    }
}

void ASTPrinter::visit_ASTExit(ASTExit *ast) {
    os << "EXIT";
}

void ASTPrinter::visit_ASTCall(ASTCall *ast) {
    ast->name->accept(this);
    os << "(";
    std::for_each(ast->args.begin(), ast->args.end(),
                  [this, ast](auto const &a) {
                      a->accept(this);
                      if (a != *(ast->args.end() - 1)) {
                          os << ", ";
                      }
                  });
    os << ")";
}

void ASTPrinter::visit_ASTIf(ASTIf *ast) {
    os << "IF ";
    ast->if_clause.expr->accept(this);
    os << " THEN\n";
    std::for_each(ast->if_clause.stats.begin(), ast->if_clause.stats.end(),
                  [this](auto const &x) {
                      x->accept(this);
                      os << ";\n";
                  });

    std::for_each(ast->elsif_clause.begin(), ast->elsif_clause.end(),
                  [this](auto const &x) {
                      os << "ELSIF ";
                      x.expr->accept(this);
                      os << " THEN\n";
                      std::for_each(x.stats.begin(), x.stats.end(),
                                    [this](auto const &s) {
                                        s->accept(this);
                                        os << ";\n";
                                    });
                  });
    if (ast->else_clause) {
        os << "ELSE\n";
        auto elses = *ast->else_clause;
        std::for_each(begin(elses), end(elses), [this](auto const &s) {
            s->accept(this);
            os << ";\n";
        });
    }
    os << "END";
}

void ASTPrinter::visit_ASTFor(ASTFor *ast) {
    os << "FOR ";
    ast->ident->accept(this);
    os << " := ";
    ast->start->accept(this);
    os << " TO ";
    ast->end->accept(this);
    if (ast->by) {
        os << fmt::format(" BY ");
        (*ast->by)->accept(this);
    }
    os << " DO\n";
    std::for_each(begin(ast->stats), end(ast->stats), [this](auto const &x) {
        x->accept(this);
        os << ";\n";
    });
    os << "END";
}

void ASTPrinter::visit_ASTWhile(ASTWhile *ast) {
    os << "WHILE ";
    ast->expr->accept(this);
    os << " DO\n";
    std::for_each(begin(ast->stats), end(ast->stats), [this](auto const &x) {
        x->accept(this);
        os << ";\n";
    });
    os << "END";
}

void ASTPrinter::visit_ASTRepeat(ASTRepeat *ast) {
    os << "REPEAT\n";
    std::for_each(begin(ast->stats), end(ast->stats), [this](auto const &x) {
        x->accept(this);
        os << ";\n";
    });
    os << "UNTIL ";
    ast->expr->accept(this);
}

void ASTPrinter::visit_ASTLoop(ASTLoop *ast) {
    os << "LOOP\n";
    std::for_each(begin(ast->stats), end(ast->stats), [this](auto const &x) {
        x->accept(this);
        os << ";\n";
    });
    os << "END";
}

void ASTPrinter::visit_ASTBlock(ASTBlock *ast) {
    os << "BEGIN\n";
    std::for_each(begin(ast->stats), end(ast->stats), [this](auto const &x) {
        x->accept(this);
        os << ";\n";
    });
    os << "END";
}

void ASTPrinter::visit_ASTExpr(ASTExpr *ast) {
    ast->expr->accept(this);
    if (ast->relation) {
        os << fmt::format(" {} ", string(*ast->relation));
        (*ast->relation_expr)->accept(this);
    }
}

void ASTPrinter::visit_ASTSimpleExpr(ASTSimpleExpr *ast) {
    if (ast->first_sign) {
        os << string(ast->first_sign.value());
    }
    ast->term->accept(this);
    std::for_each(ast->rest.begin(), ast->rest.end(), [this](auto t) {
        if (t.first == TokenType::or_k) {
            os << fmt::format(" {} ", string(t.first));
        } else {
            os << string(t.first);
        }
        t.second->accept(this);
    });
}

void ASTPrinter::visit_ASTTerm(ASTTerm *ast) {
    ast->factor->accept(this);
    std::for_each(ast->rest.begin(), ast->rest.end(), [this](auto t) {
        if (t.first == TokenType::asterisk) {
            os << string(t.first);
        } else {
            os << fmt::format(" {} ", string(t.first));
        }
        t.second->accept(this);
    });
}

void ASTPrinter::visit_ASTFactor(ASTFactor *ast) {
    std::visit(overloaded{[this](auto arg) { arg->accept(this); },
                          [this](std::shared_ptr<ASTExpr> const &arg) {
                              this->os << " (";
                              arg->accept(this);
                              this->os << ") ";
                          },
                          [this, ast](std::shared_ptr<ASTFactor> const &arg) {
                              if (ast->is_not) {
                                  os << "~ ";
                              }
                              arg->accept(this);
                          }},
               ast->factor);
}

void ASTPrinter::visit_ASTIdentifier(ASTIdentifier *ast) {
    os << fmt::format("{}", ast->value);
};

void ASTPrinter::visit_ASTInteger(ASTInteger *ast) {
    os << fmt::format("{}", ast->value);
}

void ASTPrinter::visit_ASTType(ASTType *ast) {
    std::visit(overloaded{[this](auto arg) { arg->accept(this); }}, ast->type);
}

void ASTPrinter::visit_ASTArray(ASTArray *ast) {
    os << "ARRAY [";
    ast->size->accept(this);
    os << "] OF ";
    ast->type->accept(this);
}

void ASTPrinter::visit_ASTBool(ASTBool *ast) {
    if (ast->value) {
        os << string(TokenType::true_k);
    } else {
        os << string(TokenType::false_k);
    }
}

} // namespace ax