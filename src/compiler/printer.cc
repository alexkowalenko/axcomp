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
    for (auto const &proc : ast->procedures) {
        proc->accept(this);
        if (indent_width > 0) {
            os << '\n';
        }
    }
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
        for (auto iter = ast->imports.begin(); iter != ast->imports.end(); ++iter) {
            if (iter->second) {
                iter->second->accept(this);
                os << " := ";
            }
            iter->first->accept(this);
            if (std::next(iter) != ast->imports.end()) {
                os << ",\n";
            }
            os << indent();
        }
        os << ";\n";
    }
}

void ASTPrinter::visit(ASTConst const &ast) {
    if (!ast->consts.empty()) {
        os << indent() << "CONST\n";
        push();
        for (auto const &c : ast->consts) {
            os << indent();
            c.ident->accept(this);
            os << std::string(c.ident->attrs);
            os << " = ";
            c.value->accept(this);
            os << ";\n";
        }
        pop();
    }
}

void ASTPrinter::visit(ASTTypeDec const &ast) {
    if (!ast->types.empty()) {
        os << indent() << "TYPE\n";
        push();
        for (const auto &[name, snd] : ast->types) {
            os << indent();
            name->accept(this);
            os << std::string(name->attrs);
            os << " = ";
            snd->accept(this);
            os << ";\n";
        }
        pop();
    }
}

void ASTPrinter::visit(ASTVar const &ast) {
    if (!ast->vars.empty()) {
        os << indent() << "VAR\n";
        push();
        for (const auto &[name, snd] : ast->vars) {
            os << indent();
            name->accept(this);
            os << std::string(name->attrs);
            os << ": ";
            snd->accept(this);
            os << ";\n";
        }
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
        for (auto param_iter = ast.params.begin(); param_iter != ast.params.end(); ++param_iter) {
            const auto &[name, type] = *param_iter;
            if (name->is(Attr::var)) {
                os << "VAR ";
            }
            name->accept(this);
            os << " : ";
            type->accept(this);
            if (std::next(param_iter) != ast.params.end()) {
                os << "; ";
            }
        }
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
    for (auto const &proc : ast->procedures) {
        proc->accept(this);
    }
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

void ASTPrinter::visit(ASTExit const & /*ast*/) {
    os << "EXIT";
}

void ASTPrinter::visit(ASTCall const &ast) {
    ast->name->accept(this);
    os << "(";
    for (auto iter = ast->args.begin(); iter != ast->args.end(); ++iter) {
        (*iter)->accept(this);
        if (std::next(iter) != ast->args.end()) {
            os << ", ";
        }
    }
    os << ")";
}

void ASTPrinter::print_stats(std::vector<ASTStatement> stats) {
    push();
    for (auto iter = stats.begin(); iter != stats.end(); ++iter) {
        os << indent();
        (*iter)->accept(this);
        if (std::next(iter) != stats.end()) {
            os << ';';
        }
        os << '\n';
    }
    pop();
};

void ASTPrinter::visit(ASTIf const &ast) {
    os << "IF ";
    ast->if_clause.expr->accept(this);
    os << " THEN\n";
    print_stats(ast->if_clause.stats);
    for (auto const &elsif_clause : ast->elsif_clause) {
        os << indent() << "ELSIF ";
        elsif_clause.expr->accept(this);
        os << " THEN\n";
        print_stats(elsif_clause.stats);
    }
    if (ast->else_clause) {
        os << indent() << "ELSE\n";
        print_stats(*ast->else_clause);
    }
    os << indent() << "END";
}

void ASTPrinter::visit(ASTCaseElement const &ast) {
    for (auto iter = ast->exprs.begin(); iter != ast->exprs.end(); ++iter) {
        if (std::holds_alternative<ASTSimpleExpr>(*iter)) {
            std::get<ASTSimpleExpr>(*iter)->accept(this);
        } else if (std::holds_alternative<ASTRange>(*iter)) {
            auto const &casexpr = std::get<ASTRange>(*iter);
            casexpr->first->accept(this);
            os << "..";
            casexpr->last->accept(this);
        }
        if (std::next(iter) != ast->exprs.end()) {
            os << ", ";
        }
    }
    os << " : ";
    int i = 0;
    push();
    for (auto const &stmt : ast->stats) {
        if (i != 0) {
            os << indent();
        }
        stmt->accept(this);
        os << ";\n";
        i++;
    }
    pop();
}

void ASTPrinter::visit(ASTCase const &ast) {
    os << "CASE ";
    ast->expr->accept(this);
    os << " OF\n";
    push();
    int i = 0;
    for (auto const &element : ast->elements) {
        os << indent();
        if (i != 0) {
            os << "| ";
        }
        element->accept(this);
        i++;
    }
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
    for (auto const &rest : ast->rest) {
        if (rest.first == TokenType::or_k) {
            os << std::string(std::format(" {0} ", string(rest.first)));
        } else {
            os << string(rest.first);
        }
        rest.second->accept(this);
    }
}

void ASTPrinter::visit(ASTTerm const &ast) {
    ast->factor->accept(this);
    for (auto const &rest : ast->rest) {
        if (rest.first == TokenType::asterisk) {
            os << string(rest.first);
        } else {
            os << std::string(std::format(" {0} ", string(rest.first)));
        }
        rest.second->accept(this);
    }
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
    for (auto &selector : ast->selectors) {
        std::visit(overloaded{[this](ArrayRef const &s) {
                                  os << '[';
                                  for (auto iter = s.begin(); iter != s.end(); ++iter) {
                                      (*iter)->accept(this);
                                      if (std::next(iter) != s.end()) {
                                          os << ',';
                                      }
                                  }
                                  os << ']';
                              },
                              [this](FieldRef const &s) {
                                  os << '.';
                                  s.first->accept(this);
                              },
                              [this](PointerRef /* unused */) { os << '^'; }},
                   selector);
    }
}

void ASTPrinter::visit(ASTType const &ast) {
    std::visit(overloaded{[this](auto arg) { arg->accept(this); }}, ast->type);
}

void ASTPrinter::visit(ASTArray const &ast) {
    os << "ARRAY ";
    for (auto iter = ast->dimensions.begin(); iter != ast->dimensions.end(); ++iter) {
        (*iter)->accept(this);
        if (std::next(iter) != ast->dimensions.end()) {
            os << ", ";
        } else {
            os << ' ';
        }
    }
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
    for (auto iter = ast->fields.begin(); iter != ast->fields.end(); ++iter) {
        os << indent();
        iter->first->accept(this);
        os << std::string(iter->first->attrs);
        os << ": ";
        iter->second->accept(this);
        if (std::next(iter) != ast->fields.end()) {
            os << ";\n";
        } else {
            pop();
            os << "\n" << indent() << "END";
        }
    }
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
    for (auto iter = ast->values.begin(); iter != ast->values.end(); ++iter) {
        std::visit(overloaded{[this](auto e) { e->accept(this); }}, *iter);
        if (std::next(iter) != ast->values.end()) {
            os << ',';
        };
    }
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
        os << std::hex << '0' << static_cast<int>(ast->value) << 'X' << std::dec;
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

void ASTPrinter::visit(ASTNil const & /*not used*/) {
    os << "NIL";
}

} // namespace ax
