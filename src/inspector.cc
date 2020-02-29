//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "inspector.hh"

#include <fmt/core.h>

#include "astmod.hh"
#include "error.hh"

namespace ax {

inline constexpr bool debug_inspect{false};

template <typename... T> inline void debug(const T &... msg) {
    if constexpr (debug_inspect) {
        std::cout << fmt::format(msg...) << std::endl;
    }
}

void Inspector::visit_ASTModule(ASTModule *ast) {
    debug("Inspector::visit_ASTModule");
    ast->decs->accept(this);
    for (auto const &proc : ast->procedures) {
        proc->accept(this);
    }
    has_return = false;
    for (auto const &x : ast->stats) {
        x->accept(this);
    }
    if (!has_return) {
        throw CodeGenException(
            fmt::format("MODULE {} has no RETURN function", ast->name), 0);
    }
}

void Inspector::visit_ASTVar(ASTVar *ast) {
    debug("Inspector::visit_ASTVar");
    if (!ast->vars.empty()) {
        for (auto const &c : ast->vars) {
            c.ident->accept(this);

            auto result = types.find(c.type);
            if (!result) {
                throw TypeError(fmt::format("Unknown type: {}", c.type), 0);
            }
        }
    }
}

void Inspector::visit_ASTProcedure(ASTProcedure *ast) {
    ast->decs->accept(this);
    for (auto const &x : ast->stats) {
        has_return = false;
        x->accept(this);
        if (!has_return) {
            throw CodeGenException(
                fmt::format("PROCEDURE {} has no RETURN function", ast->name),
                0);
        }
    }
}

void Inspector::visit_ASTReturn(ASTReturn *ast) {
    if (ast->expr) {
        visit_ASTExpr(ast->expr.get());

        has_return = true;
    }
}

void Inspector::visit_ASTCall(ASTCall *ast) {
    auto res = symbols.find(ast->name->value);
    if (!res) {
        throw CodeGenException(
            fmt::format("undefined PROCEDURE {}", ast->name->value), 0);
    }
    if (res->type != "PROCEDURE") {
        throw CodeGenException(
            fmt::format("{} is not a PROCEDURE", ast->name->value), 0);
    }
}

void Inspector::visit_ASTFactor(ASTFactor *ast) {
    if (ast->identifier != nullptr) {
        if (auto res = symbols.find(ast->identifier->value); !res) {
            throw CodeGenException(
                fmt::format("undefined identifier {}", ast->identifier->value),
                0);
        }
    }
}

} // namespace ax