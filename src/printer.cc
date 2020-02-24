//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "printer.hh"

#include <fmt/core.h>

#include "astmod.hh"

namespace ax {

void ASTPrinter::visit_ASTModule(ASTModule *ast) {
    os << fmt::format("MODULE {};\nBEGIN\n", ast->name);
    for (auto x : ast->exprs) {
        visit_ASTExpr(x.get());
    }
    os << fmt::format("END {}.\n", ast->name);
}

void ASTPrinter::visit_ASTExpr(ASTExpr *ast) {
    if (ast->first_sign) {
        os << to_string(ast->first_sign.value());
    }
    visit_ASTTerm(ast->term.get());
    for (auto t : ast->rest) {
        os << to_string(t.sign);
        visit_ASTTerm(t.term.get());
    }
    os << ";\n";
}

void ASTPrinter::visit_ASTTerm(ASTTerm *ast) {
    visit_ASTFactor(ast->factor.get());
    for (auto t : ast->rest) {
        if (t.sign == TokenType::div || t.sign == TokenType::mod) {
            os << fmt::format(" {} ", to_string(t.sign));
        } else {
            os << to_string(t.sign);
        }
        visit_ASTFactor(t.factor.get());
    }
}

void ASTPrinter::visit_ASTFactor(ASTFactor *ast) {
    if (ast->integer) {
        visit_ASTInteger(ast->integer.get());
    } else {
        visit_ASTExpr(ast->expr.get());
    }
}

void ASTPrinter::visit_ASTInteger(ASTInteger *ast) {
    os << fmt::format("{}", ast->value);
}

} // namespace ax