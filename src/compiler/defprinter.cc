//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "defprinter.hh"

#include <algorithm>
#include <format>

#include "ast/all.hh"
#include "token.hh"

namespace ax {

void DefPrinter::visit(ASTModule const &ast) {
    os << std::string(std::format("DEFINITION {0};\n", ast->name));
    ast->decs->accept(this);
    for (auto const &proc : ast->procedures) {
        if (proc->name->is(Attr::global)) {
            proc->accept(this);
        }
    }
    os << std::string(std::format("END {0}.\n", ast->name));
}

void DefPrinter::visit(ASTConst const &ast) {
    bool print_hdr{false};

    for (auto const &c : ast->consts) {
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
    }
}

void DefPrinter::visit(ASTTypeDec const &ast) {
    bool print_hdr{false};

    for (auto const &type_pair : ast->types) {
        if (type_pair.first->is(Attr::global) || type_pair.first->is(Attr::read_only)) {
            if (!print_hdr) {
                os << "TYPE\n";
                print_hdr = true;
            }
            type_pair.first->accept(this);
            os << std::string(type_pair.first->attrs);
            os << " = ";
            type_pair.second->accept(this);
            os << ";\n";
        }
    }
}

void DefPrinter::visit(ASTVar const &ast) {
    bool print_hdr{false};

    for (auto const &var : ast->vars) {
        if (var.first->is(Attr::global) || var.first->is(Attr::read_only)) {
            if (!print_hdr) {
                os << "VAR\n";
                print_hdr = true;
            }
            var.first->accept(this);
            os << std::string(var.first->attrs);
            os << ": ";
            var.second->accept(this);
            os << ";\n";
        }
    }
}

void DefPrinter::visit(ASTProcedure const &ast) {
    os << std::string(std::format("PROCEDURE {0}", ast->name->value))
       << std::string(ast->name->attrs);
    if (!ast->params.empty() || ast->return_type != nullptr) {
        os << "(";
        for (auto param_iter = ast->params.begin(); param_iter != ast->params.end();
             ++param_iter) {
            auto const &param = *param_iter;
            if (param.first->is(Attr::var)) {
                os << "VAR ";
            }
            param.first->accept(this);
            os << " : ";
            param.second->accept(this);
            if (std::next(param_iter) != ast->params.end()) {
                os << "; ";
            }
        }
        os << ")";
    }
    if (ast->return_type != nullptr) {
        os << ": ";
        ast->return_type->accept(this);
    }
    os << ";\n";
}

} // namespace ax
