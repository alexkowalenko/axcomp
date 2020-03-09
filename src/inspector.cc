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
    last_proc = nullptr;

    std::for_each(ast->stats.begin(), ast->stats.end(),
                  [this](auto const &x) { x->accept(this); });
    if (!has_return) {
        throw CodeGenException(
            fmt::format("MODULE {} has no RETURN function", ast->name), 0);
    }
}

void Inspector::visit_ASTConst(ASTConst *ast) {
    if (!ast->consts.empty()) {
        for (auto &c : ast->consts) {
            c.value->accept(this);
            c.type = std::string(*last_type);
        }
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
    auto retType = TypeTable::VoidType;
    if (!ast->return_type.empty()) {
        auto result = types.find(ast->return_type);
        if (!result) {
            throw TypeError(
                fmt::format("Unknown type: {} for return from function {}",
                            ast->return_type, ast->name),
                0);
        }
        retType = *result;
    }

    // Check parameter types
    std::vector<TypePtr> argTypes;
    std::for_each(
        ast->params.begin(), ast->params.end(),
        [this, ast, &argTypes](auto const &p) {
            auto r = types.find(p.second);
            if (!r) {
                throw TypeError(
                    fmt::format(
                        "Unknown type: {} for paramater {} from function {}",
                        p.second, p.first->value, ast->name),
                    0);
            }
            argTypes.push_back(*r);
        });

    auto procType = std::make_shared<ProcedureType>(retType, argTypes);
    types.put(ast->name, procType);

    last_proc = ast;

    // new symbol table
    auto former_symboltable = current_symboltable;
    current_symboltable =
        std::make_shared<SymbolTable<Symbol>>(former_symboltable);
    std::for_each(ast->params.begin(), ast->params.end(),
                  [this](auto const &p) {
                      current_symboltable->put(
                          p.first->value, Symbol(p.first->value, p.second));
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
    current_symboltable = former_symboltable;
}

void Inspector::visit_ASTAssignment(ASTAssignment *ast) {
    ast->expr->accept(this);
    auto expr_type = last_type;

    ast->ident->accept(this);
    if (!last_type->equiv(expr_type)) {
        throw TypeError(fmt::format("Can't assign expression of type {} to {}",
                                    std::string(*expr_type), ast->ident->value),
                        0);
    }
}

void Inspector::visit_ASTReturn(ASTReturn *ast) {
    has_return = true;
    if (ast->expr) {
        ast->expr->accept(this);
    } else {
        // No expression with RETURN statement
        last_type = TypeTable::VoidType;
    }

    // check return type
    if (last_proc) {
        auto type = last_proc->return_type;
        auto retType = TypeTable::VoidType;
        if (!type.empty()) {
            if (auto t = types.find(type); t) {
                retType = *t;
            }
        }

        if (!last_type->equiv(retType)) {
            throw TypeError(
                fmt::format("RETURN does not match return type for function {}",
                            last_proc->name),
                0);
        }
    }
}

void Inspector::visit_ASTCall(ASTCall *ast) {
    auto res = current_symboltable->find(ast->name->value);
    if (!res) {
        throw CodeGenException(
            fmt::format("undefined PROCEDURE {}", ast->name->value), 0);
    }
    if (res->type != "PROCEDURE") {
        throw TypeError(fmt::format("{} is not a PROCEDURE", ast->name->value),
                        0);
    }

    // Find return type from procedure and put in last_type
    auto pType = types.find(ast->name->value);
    if (!pType) {
        throw TypeError(
            fmt::format("undefined type PROCEDURE {}", ast->name->value), 0);
    }
    auto procType = std::dynamic_pointer_cast<ProcedureType>(*pType);
    if (!procType) {
        throw TypeError(
            fmt::format("{} is not type PROCEDURE", ast->name->value), 0);
    }

    if (ast->args.size() != procType->params.size()) {
        throw TypeError(fmt::format("calling PROCEDURE {}, incorrect number of "
                                    "arguments: {} instead of {}",
                                    ast->name->value, ast->args.size(),
                                    procType->params.size()),
                        0);
    }

    last_type = procType->ret;
}

void Inspector::visit_ASTIf(ASTIf *ast) {
    ast->if_clause.expr->accept(this);
    if (last_type != TypeTable::BoolType) {
        throw TypeError(fmt::format("IF expression must be type BOOLEAN"), 0);
    }

    std::for_each(ast->if_clause.stats.begin(), ast->if_clause.stats.end(),
                  [this](auto const &x) { x->accept(this); });

    std::for_each(
        ast->elsif_clause.begin(), ast->elsif_clause.end(),
        [this](auto const &x) {
            x.expr->accept(this);
            if (last_type != TypeTable::BoolType) {
                throw TypeError(
                    fmt::format("ELSIF expression must be type BOOLEAN"), 0);
            }
            std::for_each(x.stats.begin(), x.stats.end(),
                          [this](auto const &s) { s->accept(this); });
        });
    if (ast->else_clause) {
        auto elses = *ast->else_clause;
        std::for_each(begin(elses), end(elses),
                      [this](auto const &s) { s->accept(this); });
    }
}

void Inspector::visit_ASTExpr(ASTExpr *ast) {
    ast->expr->accept(this);
    if (ast->relation) {
        auto t1 = last_type;
        (*ast->relation_expr)->accept(this);
        // Types have to be the same BOOLEANs or INTEGERs
        if (!last_type->equiv(t1)) {
            throw TypeError(
                fmt::format("types in expression don't match {} and {}",
                            std::string(*t1), std::string(*last_type)),
                0);
        }
        // Comparison operators return BOOLEAN
        last_type = TypeTable::BoolType;
    }
}

void Inspector::visit_ASTSimpleExpr(ASTSimpleExpr *ast) {
    visit_ASTTerm(ast->term.get());
    auto t1 = last_type;
    std::for_each(ast->rest.begin(), ast->rest.end(), [this, &t1](auto t) {
        t.second->accept(this);
        if (!last_type->equiv(t1)) {
            throw TypeError(
                fmt::format("types in expression don't match {} and {}",
                            std::string(*t1), std::string(*last_type)),
                0);
        }
        if (t.first == TokenType::or_k) {
            if (last_type != TypeTable::BoolType) {
                throw TypeError(
                    fmt::format("types in OR expression must be BOOLEAN"), 0);
            }
        } else {
            if (last_type != TypeTable::IntType) {
                throw TypeError(
                    fmt::format("types in {} expression must be numeric",
                                string(t.first)),
                    0);
            }
        }
        t1 = last_type;
    });
}

void Inspector::visit_ASTTerm(ASTTerm *ast) {
    visit_ASTFactor(ast->factor.get());
    auto t1 = last_type;
    std::for_each(ast->rest.begin(), ast->rest.end(), [this, &t1](auto t) {
        t.second->accept(this);
        if (!last_type->equiv(t1)) {
            throw TypeError(
                fmt::format("types in expression don't match {} and {}",
                            std::string(*t1), std::string(*last_type)),
                0);
        }
        if (t.first == TokenType::ampersand) {
            if (last_type != TypeTable::BoolType) {
                throw TypeError(
                    fmt::format("types in & expression must be BOOLEAN"), 0);
            }
        } else {
            if (last_type != TypeTable::IntType) {
                throw TypeError(
                    fmt::format("types in {} expression must be numeric",
                                string(t.first)),
                    0);
            }
        }
        t1 = last_type;
    });
}

void Inspector::visit_ASTFactor(ASTFactor *ast) {
    std::visit(
        overloaded{
            [this](auto factor) { factor->accept(this); },
            [this](std::shared_ptr<ASTCall> const &factor) {
                // need to pass on return type */
                factor->accept(this);
            },
            [this, ast](std::shared_ptr<ASTFactor> const &arg) {
                if (ast->is_not) {
                    arg->accept(this);
                    if (last_type != TypeTable::BoolType) {
                        throw TypeError(
                            fmt::format("type in ~ expression must be BOOLEAN"),
                            0);
                    }
                }
            },

        },
        ast->factor);
}

void Inspector::visit_ASTIdentifier(ASTIdentifier *ast) {
    debug("Inspector::visit_ASTIdentifier");
    auto res = current_symboltable->find(ast->value);
    if (!res) {
        throw CodeGenException(
            fmt::format("undefined identifier {}", ast->value), 0);
    }
    debug("find type: {} for {}", res->type, res->name);
    auto resType = types.find(res->type);
    if (!resType) {
        throw TypeError(fmt::format("Unknown type: {} for identifier {}",
                                    res->type, ast->value),
                        0);
    }
    last_type = *resType;
}

void Inspector::visit_ASTInteger(ASTInteger *) {
    last_type = TypeTable::IntType;
}

void Inspector::visit_ASTBool(ASTBool *ast) {
    last_type = TypeTable::BoolType;
}

} // namespace ax