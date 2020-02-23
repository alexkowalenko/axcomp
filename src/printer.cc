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
    visit_ASTInteger(ast->integer.get());
}

void ASTPrinter::visit_ASTInteger(ASTInteger *ast) {
    os << fmt::format("{};\n", ast->value);
}

} // namespace ax