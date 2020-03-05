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

template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...)->overloaded<Ts...>;

inline constexpr bool debug_inspect{false};

template <typename... T> inline void debug(const T &... msg) {
    if constexpr (debug_inspect) {
        std::cout << fmt::format(msg...) << std::endl;
    }
}

void Inspector::visit_ASTModule(ASTModule *ast) {
    debug("Inspector::visit_ASTModule");
    ast->decs->accept(this);
    std::for_each(ast->procedures.begin(), ast->procedures.end(),
                  [this](auto const &proc) { proc->accept(this); });
    has_return = false;
    std::for_each(ast->stats.begin(), ast->stats.end(),
                  [this](auto const &x) { x->accept(this); });
    if (!has_return) {
        throw CodeGenException(
            fmt::format("MODULE {} has no RETURN function", ast->name), 0);
    }
}

void Inspector::visit_ASTVar(ASTVar *ast) {
    debug("Inspector::visit_ASTVar");
    if (!ast->vars.empty()) {
        std::for_each(ast->vars.begin(), ast->vars.end(),
                      [this](auto const &v) {
                          v.first->accept(this);

                          auto result = types.find(v.second);
                          if (!result) {
                              throw TypeError(
                                  fmt::format("Unknown type: {}", v.second), 0);
                          }
                      });
    }
}

void Inspector::visit_ASTProcedure(ASTProcedure *ast) {

    // Check return type
    if (!ast->return_type.empty()) {
        auto result = types.find(ast->return_type);
        if (!result) {
            throw TypeError(
                fmt::format("Unknown type: {} for return from function {}",
                            ast->return_type, ast->name),
                0);
        }
    };

    // Check parameter types
    std::for_each(
        ast->params.begin(), ast->params.end(), [this, ast](auto const &p) {
            if (auto r = types.find(p.second); !r) {
                throw TypeError(
                    fmt::format(
                        "Unknown type: {} for paramater {} from function {}",
                        p.second, p.first->value, ast->name),
                    0);
            }
        });

    ast->decs->accept(this);
    std::for_each(ast->stats.begin(), ast->stats.end(),
                  [this, ast](auto const &x) {
                      has_return = false;
                      x->accept(this);
                      if (!has_return) {
                          throw CodeGenException(
                              fmt::format("PROCEDURE {} has no RETURN function",
                                          ast->name),
                              0);
                      }
                  });
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
    std::visit(
        overloaded{
            [](auto arg) { /* do nothing for others*/ },
            [this](std::shared_ptr<ASTCall> const &arg) { arg->accept(this); },
            [this](std::shared_ptr<ASTIdentifier> const &arg) {
                if (auto res = this->symbols.find(arg->value); !res) {
                    throw CodeGenException(
                        fmt::format("undefined identifier {}", arg->value), 0);
                }
            },
        },
        ast->factor);
}

} // namespace ax