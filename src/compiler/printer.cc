//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "printer.hh"

#include <algorithm>
#include <experimental/iterator>
#include <format>

#include "ast.hh"
#include "token.hh"

namespace ax {

void ASTPrinter::visit(ASTModule const &ast) {

    os << std::format("MODULE {0};\n", ast->name);
    if (ast->import) {
        ast->import->accept(this);
    }

    push();
    ast->decs->accept(this);
    if (indent_width > 0) {
        os << '\n';
    }
    std::for_each(ast->procedures.begin(), ast->procedures.end(), [this](auto const &proc) {
        proc->accept(this);
        if (indent_width > 0) {
            os << '\n';
        }
    });
    pop();
    if (!ast->stats.empty()) {
        os << "BEGIN\n";
        print_stats(ast->stats);
    }
    os << std::string(std::format("END {0}.\n", ast->name));
}

void ASTPrinter::visit(ASTImport const &ast) {
    if (!ast->imports.empty()) {
        os << indent() << "IMPORT ";
        std::for_each(begin(ast->imports), end(ast->imports), [this, ast](auto const &i) {
            if (i.second) {
                i.second->accept(this);
                os << " := ";
            }
            i.first->accept(this);
            if (i != *(ast->imports.end() - 1)) {
                os << ",\n";
            }
            os << indent();
        });
        os << ";\n";
    }
}

void ASTPrinter::visit(ASTConst const &ast) {
    if (!ast->consts.empty()) {
        os << indent() << "CONST\n";
        push();
        std::for_each(ast->consts.begin(), ast->consts.end(), [this](auto const &c) {
            os << indent();
            c.ident->accept(this);
            os << std::string(c.ident->attrs);
            os << " = ";
            c.value->accept(this);
            os << ";\n";
        });
        pop();
    }
}

void ASTPrinter::visit(ASTTypeDec const &ast) {
    if (!ast->types.empty()) {
        os << indent() << "TYPE\n";
        push();
        std::for_each(begin(ast->types), end(ast->types), [this](auto const &v) {
            os << indent();
            v.first->accept(this);
            os << std::string(v.first->attrs);
            os << " = ";
            v.second->accept(this);
            os << ";\n";
        });
        pop();
    }
}

void ASTPrinter::visit(ASTVar const &ast) {
    if (!ast->vars.empty()) {
        os << indent() << "VAR\n";
        push();
        std::for_each(ast->vars.begin(), ast->vars.end(), [this](auto const &v) {
            os << indent();
            v.first->accept(this);
            os << std::string(v.first->attrs);
            os << ": ";
            v.second->accept(this);
            os << ";\n";
        });
        pop();
    }
}

void ASTPrinter::proc_rec(RecVar const &r) {
    os << '(';
    if (r.first->is(Attr::var)) {
        os << "VAR ";
    }
    r.first->accept(this);
    os << " : ";
    r.second->accept(this);
    os << ") ";
}

void ASTPrinter::proc_header(ASTProc_ const &ast, bool forward) {
    os << indent() << "PROCEDURE ";
    if (forward) {
        os << "^ ";
    }
    if (ast.receiver.first) {
        proc_rec(ast.receiver);
    }
    os << ast.name->value << std::string(ast.name->attrs);
    if (!ast.params.empty() || ast.return_type != nullptr) {
        os << "(";
        std::for_each(cbegin(ast.params), cend(ast.params), [this, &ast](auto const &p) {
            if (p.first->is(Attr::var)) {
                os << "VAR ";
            }
            p.first->accept(this);
            os << " : ";
            p.second->accept(this);
            if (p != *(ast.params.end() - 1)) {
                os << "; ";
            }
        });
        os << ")";
    }
    if (ast.return_type != nullptr) {
        os << ": ";
        ast.return_type->accept(this);
    }
    os << ";\n";
}

void ASTPrinter::visit(ASTProcedure const &ast) {
    proc_header(*ast, false);
    push();
    ast->decs->accept(this);
    std::for_each(cbegin(ast->procedures), cend(ast->procedures),
                  [this](auto const &p) { p->accept(this); });
    pop();
    if (!ast->stats.empty()) {
        os << indent() << "BEGIN\n";
        print_stats(ast->stats);
    }
    os << indent() << std::string(std::format("END {0};\n", ast->name->value));
}

void ASTPrinter::visit(ASTProcedureForward const &ast) {
    proc_header(*ast, true);
}

void ASTPrinter::visit(ASTAssignment const &ast) {
    ast->ident->accept(this);
    os << " := ";
    ast->expr->accept(this);
}

void ASTPrinter::visit(ASTReturn const &ast) {
    os << "RETURN ";
    if (ast->expr) {
        ast->expr->accept(this);
    }
}

void ASTPrinter::visit(ASTExit const &/*ast*/) {
    os << "EXIT";
}

void ASTPrinter::visit(ASTCall const &ast) {
    ast->name->accept(this);
    os << "(";
    std::for_each(ast->args.begin(), ast->args.end(), [this, ast](auto const &a) {
        a->accept(this);
        if (a != *(ast->args.end() - 1)) {
            os << ", ";
        }
    });
    os << ")";
}

void ASTPrinter::print_stats(std::vector<ASTStatement> stats) {
    push();
    std::for_each(begin(stats), end(stats), [stats, this](auto const &x) {
        os << indent();
        x->accept(this);
        if (x != *(stats.end() - 1)) {
            os << ';';
        }
        os << '\n';
    });
    pop();
};

void ASTPrinter::visit(ASTIf const &ast) {
    os << "IF ";
    ast->if_clause.expr->accept(this);
    os << " THEN\n";
    print_stats(ast->if_clause.stats);
    std::for_each(begin(ast->elsif_clause), end(ast->elsif_clause), [this](auto const &x) {
        os << indent() << "ELSIF ";
        x.expr->accept(this);
        os << " THEN\n";
        print_stats(x.stats);
    });
    if (ast->else_clause) {
        os << indent() << "ELSE\n";
        print_stats(*ast->else_clause);
    }
    os << indent() << "END";
}

void ASTPrinter::visit(ASTCaseElement const &ast) {
    std::for_each(begin(ast->exprs), end(ast->exprs), [this, ast](auto const &e) {
        if (std::holds_alternative<ASTSimpleExpr>(e)) {
            std::get<ASTSimpleExpr>(e)->accept(this);
        } else if (std::holds_alternative<ASTRange>(e)) {
            auto const &casexpr = std::get<ASTRange>(e);
            casexpr->first->accept(this);
            os << "..";
            casexpr->last->accept(this);
        }
        if (e != ast->exprs.back()) {
            os << ", ";
        }
    });
    os << " : ";
    int i = 0;
    push();
    std::for_each(begin(ast->stats), end(ast->stats), [this, &i](auto const &x) {
        if (i != 0) {
            os << indent();
        }
        x->accept(this);
        os << ";\n";
        i++;
    });
    pop();
}

void ASTPrinter::visit(ASTCase const &ast) {
    os << "CASE ";
    ast->expr->accept(this);
    os << " OF\n";
    push();
    int i = 0;
    std::for_each(begin(ast->elements), end(ast->elements), [this, &i](auto const &x) {
        os << indent();
        if (i != 0) {
            os << "| ";
        }
        x->accept(this);
        i++;
    });
    pop();
    if (!ast->else_stats.empty()) {
        os << indent() << "ELSE\n";
        print_stats(ast->else_stats);
    }
    os << indent() << "END";
} // namespace ax

void ASTPrinter::visit(ASTFor const &ast) {
    os << "FOR ";
    ast->ident->accept(this);
    os << " := ";
    ast->start->accept(this);
    os << " TO ";
    ast->end->accept(this);
    if (ast->by) {
        os << " BY ";
        ast->by->accept(this);
    }
    os << " DO\n";
    print_stats(ast->stats);
    os << indent() << "END";
}

void ASTPrinter::visit(ASTWhile const &ast) {
    os << "WHILE ";
    ast->expr->accept(this);
    os << " DO\n";
    print_stats(ast->stats);
    os << indent() << "END";
}

void ASTPrinter::visit(ASTRepeat const &ast) {
    os << "REPEAT\n";
    print_stats(ast->stats);
    os << indent() << "UNTIL ";
    ast->expr->accept(this);
}

void ASTPrinter::visit(ASTLoop const &ast) {
    os << "LOOP\n";
    print_stats(ast->stats);
    os << indent() << "END";
}

void ASTPrinter::visit(ASTBlock const &ast) {
    os << "BEGIN\n";
    print_stats(ast->stats);
    os << indent() << "END";
}

void ASTPrinter::visit(ASTExpr const &ast) {
    ast->expr->accept(this);
    if (ast->relation) {
        os << std::string(std::format(" {0} ", string(*ast->relation)));
        ast->relation_expr->accept(this);
    }
}

void ASTPrinter::visit(ASTRange const &ast) {
    ast->first->accept(this);
    os << "..";
    ast->last->accept(this);
};

void ASTPrinter::visit(ASTSimpleExpr const &ast) {
    if (ast->first_sign) {
        os << string(ast->first_sign.value());
    }
    ast->term->accept(this);
    std::for_each(ast->rest.begin(), ast->rest.end(), [this](auto t) {
        if (t.first == TokenType::or_k) {
            os << std::string(std::format(" {0} ", string(t.first)));
        } else {
            os << string(t.first);
        }
        t.second->accept(this);
    });
}

void ASTPrinter::visit(ASTTerm const &ast) {
    ast->factor->accept(this);
    std::for_each(ast->rest.begin(), ast->rest.end(), [this](auto t) {
        if (t.first == TokenType::asterisk) {
            os << string(t.first);
        } else {
            os << std::string(std::format(" {0} ", string(t.first)));
        }
        t.second->accept(this);
    });
}

void ASTPrinter::visit(ASTFactor const &ast) {
    std::visit(overloaded{[this](auto arg) { arg->accept(this); },
                          [this](ASTExpr const &arg) {
                              this->os << " (";
                              arg->accept(this);
                              this->os << ") ";
                          },
                          [this, ast](ASTFactor const &arg) {
                              if (ast->is_not) {
                                  os << "~ ";
                              }
                              arg->accept(this);
                          }},
               ast->factor);
}

void ASTPrinter::visit(ASTDesignator const &ast) {
    ast->ident->accept(this);
    std::for_each(begin(ast->selectors), end(ast->selectors), [this](auto &s) {
        std::visit(overloaded{[this](ArrayRef const &s) {
                                  os << '[';
                                  std::for_each(begin(s), end(s), [this, s](auto &e) {
                                      e->accept(this);
                                      if (e != s.back()) {
                                          os << ',';
                                      }
                                  });
                                  os << ']';
                              },
                              [this](FieldRef const &s) {
                                  os << '.';
                                  s.first->accept(this);
                              },
                              [this](PointerRef /* unused */) { os << '^'; }},
                   s);
    });
}

void ASTPrinter::visit(ASTType const &ast) {
    std::visit(overloaded{[this](auto arg) { arg->accept(this); }}, ast->type);
}

void ASTPrinter::visit(ASTArray const &ast) {
    os << "ARRAY ";
    std::for_each(begin(ast->dimensions), end(ast->dimensions), [this, ast](auto &expr) {
        expr->accept(this);
        if (expr != ast->dimensions.back()) {
            os << ", ";
        } else {
            os << ' ';
        }
    });
    os << "OF ";
    ast->type->accept(this);
}

void ASTPrinter::visit(ASTRecord const &ast) {
    os << "RECORD";
    if (ast->base) {
        os << " (";
        ast->base->accept(this);
        os << ')';
    }
    os << '\n';
    push();
    std::for_each(begin(ast->fields), end(ast->fields), [this, ast](auto const &s) {
        os << indent();
        s.first->accept(this);
        os << std::string(s.first->attrs);
        os << ": ";
        s.second->accept(this);
        if (s != *(ast->fields.end() - 1)) {
            os << ";\n";
        } else {
            pop();
            os << "\n" << indent() << "END";
        }
    });
}

void ASTPrinter::visit(ASTPointerType const &ast) {
    os << "POINTER TO ";
    ast->reference->accept(this);
}

void ASTPrinter::visit(ASTQualident const &ast) {
    if (!ast->qual.empty()) {
        os << ast->qual + ".";
    }
    ast->id->accept(this);
}

void ASTPrinter::visit(ASTIdentifier const &ast) {
    os << ast->value;
}

void ASTPrinter::visit(ASTSet const &ast) {
    os << '{';
    std::for_each(cbegin(ast->values), cend(ast->values), [this, ast](auto const &e) {
        std::visit(overloaded{[this](auto e) { e->accept(this); }}, e);
        if (e != *(ast->values.end() - 1)) {
            os << ',';
        };
    });
    os << '}';
}

void ASTPrinter::visit(ASTInteger const &ast) {
    if (ast->hex) {
        os << std::hex << '0';
    };
    os << ast->value;
    if (ast->hex) {
        os << 'H' << std::dec;
    };
}

void ASTPrinter::visit(ASTReal const &ast) {
    os << ast->style;
}

void ASTPrinter::visit(ASTCharPtr const &ast) {
    if (ast->hex) {
        os << std::hex << '0' << int(ast->value) << 'X' << std::dec;
    } else {
        os << '\'' << ast->str() << '\'';
    }
}

void ASTPrinter::visit(ASTString const &ast) {
    os << ast->delim << ast->value << ast->delim;
};

void ASTPrinter::visit(ASTBool const &ast) {
    if (ast->value) {
        os << string(TokenType::true_k);
    } else {
        os << string(TokenType::false_k);
    }
}

void ASTPrinter::visit(ASTNil const &/*not used*/) {
    os << "NIL";
}

} // namespace ax