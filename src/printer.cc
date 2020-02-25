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
    os << fmt::format("MODULE {};\n", ast->name);
    visit_ASTDeclaration(ast->decs.get());
    os << fmt::format("BEGIN\n");
    for (auto x : ast->exprs) {
        visit_ASTExpr(x.get());
        os << ";\n";
    }
    os << fmt::format("END {}.\n", ast->name);
}

void ASTPrinter::visit_ASTDeclaration(ASTDeclaration *ast) {
    if (ast->cnst) {
        visit_ASTConst(ast->cnst.get());
    }
}

void ASTPrinter::visit_ASTConst(ASTConst *ast) {
    if (ast->consts.size() > 0) {
        os << "CONST\n";
        for (auto c : ast->consts) {
            visit_ASTIdentifier(c.indent.get());
            os << " = ";
            visit_ASTExpr(c.expr.get());
            os << ";\n";
        }
    }
}

void ASTPrinter::visit_ASTExpr(ASTExpr *ast) {
    if (ast->first_sign) {
        os << string(ast->first_sign.value());
    }
    visit_ASTTerm(ast->term.get());
    for (auto t : ast->rest) {
        os << string(t.sign);
        visit_ASTTerm(t.term.get());
    }
}

void ASTPrinter::visit_ASTTerm(ASTTerm *ast) {
    visit_ASTFactor(ast->factor.get());
    for (auto t : ast->rest) {
        if (t.sign == TokenType::div || t.sign == TokenType::mod) {
            os << fmt::format(" {} ", string(t.sign));
        } else {
            os << string(t.sign);
        }
        visit_ASTFactor(t.factor.get());
    }
}

void ASTPrinter::visit_ASTFactor(ASTFactor *ast) {
    if (ast->integer) {
        visit_ASTInteger(ast->integer.get());
    } else {
        os << " (";
        visit_ASTExpr(ast->expr.get());
        os << ") ";
    }
}

void ASTPrinter::visit_ASTInteger(ASTInteger *ast) {
    os << fmt::format("{}", ast->value);
}

void ASTPrinter::visit_ASTIdentifier(ASTIdentifier *ast) {
    os << fmt::format("{}", ast->value);
};

} // namespace ax