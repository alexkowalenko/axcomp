//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "printer.hh"

#include <algorithm>

#include <llvm/Support/FormatVariadic.h>

#include "ast.hh"
#include "token.hh"

namespace ax {

void ASTPrinter::visit_ASTModule(ASTModule *ast) {
    os << std::string(llvm::formatv("MODULE {0};\n", ast->name));
    ast->decs->accept(this);
    std::for_each(ast->procedures.begin(), ast->procedures.end(),
                  [this](auto const &proc) { proc->accept(this); });
    os << "BEGIN\n";
    print_stats(ast->stats);
    os << std::string(llvm::formatv("END {0}.\n", ast->name));
}

void ASTPrinter::visit_ASTConst(ASTConst *ast) {
    if (!ast->consts.empty()) {
        os << "CONST\n";
        std::for_each(ast->consts.begin(), ast->consts.end(),
                      [this](auto const &c) {
                          c.ident->accept(this);
                          os << std::string(c.ident->attrs);
                          os << " = ";
                          c.value->accept(this);
                          os << ";\n";
                      });
    }
}

void ASTPrinter::visit_ASTTypeDec(ASTTypeDec *ast) {
    if (!ast->types.empty()) {
        os << "TYPE\n";
        std::for_each(begin(ast->types), end(ast->types),
                      [this](auto const &v) {
                          v.first->accept(this);
                          os << std::string(v.first->attrs);
                          os << " = ";
                          v.second->accept(this);
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
                          os << std::string(v.first->attrs);
                          os << ": ";
                          v.second->accept(this);
                          os << ";\n";
                      });
    }
}

void ASTPrinter::visit_ASTProcedure(ASTProcedure *ast) {
    os << std::string(llvm::formatv("PROCEDURE {0}", ast->name->value))
       << std::string(ast->name->attrs);
    if (!ast->params.empty() || ast->return_type != nullptr) {
        os << "(";
        std::for_each(ast->params.begin(), ast->params.end(),
                      [this, ast](auto const &p) {
                          if (p.first->is(Attr::var)) {
                              os << "VAR ";
                          }
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
    os << "BEGIN\n";
    print_stats(ast->stats);
    os << std::string(llvm::formatv("END {0}.\n", ast->name->value));
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

void ASTPrinter::print_stats(std::vector<std::shared_ptr<ASTStatement>> stats) {
    std::for_each(begin(stats), end(stats), [stats, this](auto const &x) {
        x->accept(this);
        if (x != *(stats.end() - 1)) {
            os << ';';
        }
        os << '\n';
    });
};

void ASTPrinter::visit_ASTIf(ASTIf *ast) {
    os << "IF ";
    ast->if_clause.expr->accept(this);
    os << " THEN\n";
    print_stats(ast->if_clause.stats);

    std::for_each(ast->elsif_clause.begin(), ast->elsif_clause.end(),
                  [this](auto const &x) {
                      os << "ELSIF ";
                      x.expr->accept(this);
                      os << " THEN\n";
                      print_stats(x.stats);
                  });
    if (ast->else_clause) {
        os << "ELSE\n";
        print_stats(*ast->else_clause);
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
        os << " BY ";
        (*ast->by)->accept(this);
    }
    os << " DO\n";
    print_stats(ast->stats);
    os << "END";
}

void ASTPrinter::visit_ASTWhile(ASTWhile *ast) {
    os << "WHILE ";
    ast->expr->accept(this);
    os << " DO\n";
    print_stats(ast->stats);
    os << "END";
}

void ASTPrinter::visit_ASTRepeat(ASTRepeat *ast) {
    os << "REPEAT\n";
    print_stats(ast->stats);
    os << "UNTIL ";
    ast->expr->accept(this);
}

void ASTPrinter::visit_ASTLoop(ASTLoop *ast) {
    os << "LOOP\n";
    print_stats(ast->stats);
    os << "END";
}

void ASTPrinter::visit_ASTBlock(ASTBlock *ast) {
    os << "BEGIN\n";
    print_stats(ast->stats);
    os << "END";
}

void ASTPrinter::visit_ASTExpr(ASTExpr *ast) {
    ast->expr->accept(this);
    if (ast->relation) {
        os << std::string(llvm::formatv(" {0} ", string(*ast->relation)));
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
            os << std::string(llvm::formatv(" {0} ", string(t.first)));
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
            os << std::string(llvm::formatv(" {0} ", string(t.first)));
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

void ASTPrinter::visit_ASTDesignator(ASTDesignator *ast) {
    ast->ident->accept(this);
    std::for_each(begin(ast->selectors), end(ast->selectors), [this](auto &s) {
        std::visit(overloaded{[this](std::shared_ptr<ASTExpr> const &s) {
                                  os << '[';
                                  s->accept(this);
                                  os << ']';
                              },
                              [this](FieldRef const &s) {
                                  os << '.';
                                  s.first->accept(this);
                              }},
                   s);
    });
}

void ASTPrinter::visit_ASTIdentifier(ASTIdentifier *ast) {
    os << ast->value;
}

void ASTPrinter::visit_ASTInteger(ASTInteger *ast) {
    os << ast->value;
}

void ASTPrinter::visit_ASTType(ASTType *ast) {
    std::visit(overloaded{[this](auto arg) { arg->accept(this); }}, ast->type);
}

void ASTPrinter::visit_ASTArray(ASTArray *ast) {
    os << "ARRAY ";
    ast->size->accept(this);
    os << " OF ";
    ast->type->accept(this);
}

void ASTPrinter::visit_ASTRecord(ASTRecord *ast) {
    os << "RECORD\n";
    std::for_each(begin(ast->fields), end(ast->fields),
                  [this, ast](auto const &s) {
                      os << "  ";
                      s.first->accept(this);
                      os << std::string(s.first->attrs);
                      os << ": ";
                      s.second->accept(this);
                      if (s != *(ast->fields.end() - 1)) {
                          os << ';';
                      }
                      os << '\n';
                  });
}

void ASTPrinter::visit_ASTBool(ASTBool *ast) {
    if (ast->value) {
        os << string(TokenType::true_k);
    } else {
        os << string(TokenType::false_k);
    }
}

} // namespace ax