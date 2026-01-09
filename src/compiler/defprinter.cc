//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "defprinter.hh"

#include <algorithm>
#include <format>

#include "ast.hh"
#include "token.hh"

namespace ax {

void DefPrinter::visit(ASTModule const &ast) {
    os << std::string(std::format("DEFINITION {0};\n", ast->name));
    ast->decs->accept(this);
    std::for_each(ast->procedures.begin(), ast->procedures.end(), [this](auto const &proc) {
        if (proc->name->is(Attr::global)) {
            proc->accept(this);
        }
    });
    os << std::string(std::format("END {0}.\n", ast->name));
}

void DefPrinter::visit(ASTConst const &ast) {
    bool print_hdr{false};

    std::for_each(ast->consts.begin(), ast->consts.end(), [this, &print_hdr](auto const &c) {
        if (c.ident->is(Attr::global)) {
            if (!print_hdr) {
                os << "CONST\n";
                print_hdr = true;
            }
            c.ident->accept(this);
            os << std::string(c.ident->attrs);
            os << " = ";
            c.value->accept(this);
            os << ";\n";
        }
    });
}

void DefPrinter::visit(ASTTypeDec const &ast) {
    bool print_hdr{false};

    std::for_each(begin(ast->types), end(ast->types), [this, &print_hdr](auto const &v) {
        if (v.first->is(Attr::global) || v.first->is(Attr::read_only)) {
            if (!print_hdr) {
                os << "TYPE\n";
                print_hdr = true;
            }
            v.first->accept(this);
            os << std::string(v.first->attrs);
            os << " = ";
            v.second->accept(this);
            os << ";\n";
        }
    });
}

void DefPrinter::visit(ASTVar const &ast) {
    bool print_hdr{false};

    std::for_each(ast->vars.begin(), ast->vars.end(), [this, &print_hdr](auto const &v) {
        if (v.first->is(Attr::global) || v.first->is(Attr::read_only)) {
            if (!print_hdr) {
                os << "VAR\n";
                print_hdr = true;
            }
            v.first->accept(this);
            os << std::string(v.first->attrs);
            os << ": ";
            v.second->accept(this);
            os << ";\n";
        }
    });
}

void DefPrinter::visit(ASTProcedure const &ast) {
    os << std::string(std::format("PROCEDURE {0}", ast->name->value))
       << std::string(ast->name->attrs);
    if (!ast->params.empty() || ast->return_type != nullptr) {
        os << "(";
        std::for_each(ast->params.begin(), ast->params.end(), [this, ast](auto const &p) {
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
}

} // namespace ax
