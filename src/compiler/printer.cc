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
        visit(ast->import);
    }

    push();
    visit(ast->decs);
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
                visit(iter->second);
                os << " := ";
            }
            visit(iter->first);
            if (std::next(iter) != ast->imports.end()) {
                os << ",\n";
            }
            os << indent();
        }
        os << ";\n";
    }
}

void ASTPrinter::visit(ASTDeclaration const &ast) {
    if (ast->cnst) {
        visit(ast->cnst);
    }
    if (ast->type) {
        visit(ast->type);
    }
    if (ast->var) {
        visit(ast->var);
    }
}

void ASTPrinter::visit(ASTConst const &ast) {
    if (!ast->consts.empty()) {
        os << indent() << "CONST\n";
        push();
        for (auto const &c : ast->consts) {
            os << indent();
            visit(c.ident);
            os << std::string(c.ident->attrs);
            os << " = ";
            visit(c.value);
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
            visit(name);
            os << std::string(name->attrs);
            os << " = ";
            visit(snd);
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
            visit(name);
            os << std::string(name->attrs);
            os << ": ";
            visit(snd);
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
    visit(r.first);
    os << " : ";
    visit(r.second);
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
            visit(name);
            os << " : ";
            visit(type);
            if (std::next(param_iter) != ast.params.end()) {
                os << "; ";
            }
        }
        os << ")";
    }
    if (ast.return_type != nullptr) {
        os << ": ";
        visit(ast.return_type);
    }
    os << ";\n";
}

void ASTPrinter::visit(ASTProcedure const &ast) {
    proc_header(*ast, false);
    push();
    visit(ast->decs);
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
    visit(ast->ident);
    os << " := ";
    visit(ast->expr);
}

void ASTPrinter::visit(ASTReturn const &ast) {
    os << "RETURN ";
    if (ast->expr) {
        visit(ast->expr);
    }
}

void ASTPrinter::visit(ASTExit const & /*ast*/) {
    os << "EXIT";
}

void ASTPrinter::visit(ASTCall const &ast) {
    visit(ast->name);
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
    visit(ast->expr);
    os << " OF\n";
    push();
    int i = 0;
    for (auto const &element : ast->elements) {
        os << indent();
        if (i != 0) {
            os << "| ";
        }
        visit(element);
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
    visit(ast->ident);
    os << " := ";
    visit(ast->start);
    os << " TO ";
    visit(ast->end);
    if (ast->by) {
        os << " BY ";
        visit(ast->by);
    }
    os << " DO\n";
    print_stats(ast->stats);
    os << indent() << "END";
}

void ASTPrinter::visit(ASTWhile const &ast) {
    os << "WHILE ";
    visit(ast->expr);
    os << " DO\n";
    print_stats(ast->stats);
    os << indent() << "END";
}

void ASTPrinter::visit(ASTRepeat const &ast) {
    os << "REPEAT\n";
    print_stats(ast->stats);
    os << indent() << "UNTIL ";
    visit(ast->expr);
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
    visit(ast->expr);
    if (ast->relation) {
        os << std::string(std::format(" {0} ", string(*ast->relation)));
        visit(ast->relation_expr);
    }
}

void ASTPrinter::visit(ASTRange const &ast) {
    visit(ast->first);
    os << "..";
    visit(ast->last);
};

void ASTPrinter::visit(ASTSimpleExpr const &ast) {
    if (ast->first_sign) {
        os << string(ast->first_sign.value());
    }
    visit(ast->term);
    for (auto const &rest : ast->rest) {
        if (rest.first == TokenType::OR) {
            os << std::string(std::format(" {0} ", string(rest.first)));
        } else {
            os << string(rest.first);
        }
        rest.second->accept(this);
    }
}

void ASTPrinter::visit(ASTTerm const &ast) {
    visit(ast->factor);
    for (auto const &rest : ast->rest) {
        if (rest.first == TokenType::ASTÉRIX) {
            os << string(rest.first);
        } else {
            os << std::string(std::format(" {0} ", string(rest.first)));
        }
        rest.second->accept(this);
    }
}

void ASTPrinter::visit(ASTFactor const &ast) {
    std::visit(overloaded{[this](auto arg) { visit(arg); },
                          [this](ASTExpr const &arg) {
                              this->os << " (";
                              visit(arg);
                              this->os << ") ";
                          },
                          [this, ast](ASTFactor const &arg) {
                              if (ast->is_not) {
                                  os << "~ ";
                              }
                              visit(arg);
                          }},
               ast->factor);
}

void ASTPrinter::visit(ASTDesignator const &ast) {
    visit(ast->ident);
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
    std::visit(overloaded{[this](auto arg) { visit(arg); }}, ast->type);
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
    visit(ast->type);
}

void ASTPrinter::visit(ASTRecord const &ast) {
    os << "RECORD";
    if (ast->base) {
        os << " (";
        visit(ast->base);
        os << ')';
    }
    os << '\n';
    push();
    for (auto iter = ast->fields.begin(); iter != ast->fields.end(); ++iter) {
        os << indent();
        visit(iter->first);
        os << std::string(iter->first->attrs);
        os << ": ";
        visit(iter->second);
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
    visit(ast->reference);
}

void ASTPrinter::visit(ASTQualident const &ast) {
    if (!ast->qual.empty()) {
        os << ast->qual + ".";
    }
    visit(ast->id);
}

void ASTPrinter::visit(ASTIdentifier const &ast) {
    os << ast->value;
}

void ASTPrinter::visit(ASTSet const &ast) {
    os << '{';
    for (auto iter = ast->values.begin(); iter != ast->values.end(); ++iter) {
        std::visit(overloaded{[this](auto e) { visit(e); }}, *iter);
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
        os << string(TokenType::TRUE);
    } else {
        os << string(TokenType::FALSE);
    }
}

void ASTPrinter::visit(ASTNil const & /*not used*/) {
    os << "NIL";
}

} // namespace ax
