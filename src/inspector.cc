//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "inspector.hh"

#include <llvm/Support/FormatVariadic.h>

#include "ast.hh"
#include "error.hh"

namespace ax {

template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

inline constexpr bool debug_inspect{false};

template <typename... T> inline void debug(const T &... msg) {
    if constexpr (debug_inspect) {
        std::cout << formatv(msg...) << std::endl;
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
            llvm::formatv("MODULE {0} has no RETURN function", ast->name),
            ast->get_location());
    }
}

void Inspector::visit_ASTConst(ASTConst *ast) {
    if (!ast->consts.empty()) {
        for (auto &c : ast->consts) {
            c.value->accept(this);
            c.type = std::make_shared<ASTType>();
            c.type->type_info = last_type;
            current_symboltable->put(c.ident->value, last_type);
        }
    }
}

void Inspector::visit_ASTVar(ASTVar *ast) {
    debug("Inspector::visit_ASTVar");
    if (!ast->vars.empty()) {
        std::for_each(ast->vars.begin(), ast->vars.end(),
                      [this](auto const &v) {
                          // No need to check the identifier - its being
                          // defined.
                          v.second->accept(this);
                          // Update VAR declaration symbols with type
                          current_symboltable->put(v.first->value, last_type);
                      });
    }
}

void Inspector::visit_ASTProcedure(ASTProcedure *ast) {

    // Check return type
    auto retType = TypeTable::VoidType;
    if (ast->return_type != nullptr) {
        ast->return_type->accept(this);
        retType = last_type;
    }

    // Check parameter types
    std::vector<TypePtr> argTypes;
    std::for_each(ast->params.begin(), ast->params.end(),
                  [this, &argTypes](auto const &p) {
                      p.second->accept(this);
                      argTypes.push_back(last_type);
                  });

    auto proc_type = std::make_shared<ProcedureType>(retType, argTypes);
    current_symboltable->put(ast->name, proc_type);
    types.put(ast->name, proc_type);

    last_proc = ast;

    // new symbol table
    auto former_symboltable = current_symboltable;
    current_symboltable =
        std::shared_ptr<SymbolTable<TypePtr>>(former_symboltable);
    std::for_each(
        ast->params.begin(), ast->params.end(), [this](auto const &p) {
            std::visit(
                overloaded{
                    [this](auto arg) { ; },
                    [this, p](std::shared_ptr<ASTIdentifier> const &tname) {
                        auto type = types.find(tname->value);
                        current_symboltable->put(p.first->value, *type);
                    },
                },
                p.second->type);
        });
    ast->decs->accept(this);
    std::for_each(
        ast->stats.begin(), ast->stats.end(), [this, ast](auto const &x) {
            has_return = false;
            x->accept(this);
            if (!has_return) {
                throw CodeGenException(
                    llvm::formatv("PROCEDURE {0} has no RETURN function",
                                  ast->name),
                    ast->get_location());
            }
        });
    current_symboltable = former_symboltable;
}

void Inspector::visit_ASTAssignment(ASTAssignment *ast) {
    debug("Inspector::visit_ASTAssignment");
    ast->expr->accept(this);
    auto expr_type = last_type;

    ast->ident->accept(this);
    debug("type of ident: {} ", last_type->get_name());
    if (!last_type->equiv(expr_type)) {
        throw TypeError(
            llvm::formatv("Can't assign expression of type {0} to {1}",
                          std::string(*expr_type), std::string(*ast->ident)),
            ast->get_location());
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
        if (type != nullptr) {
            std::visit(overloaded{
                           [this](auto arg) { ; },
                           [this, &retType](
                               std::shared_ptr<ASTIdentifier> const &type) {
                               if (auto t = types.find(type->value); t) {
                                   retType = *t;
                               };
                           },
                       },
                       type->type);
        }

        if (!last_type->equiv(retType)) {
            throw TypeError(
                llvm::formatv(
                    "RETURN does not match return type for function {0}",
                    last_proc->name),
                ast->get_location());
        }
    }
}

void Inspector::visit_ASTCall(ASTCall *ast) {
    auto res = current_symboltable->find(ast->name->value);
    if (!res) {
        throw CodeGenException(
            llvm::formatv("undefined PROCEDURE {0}", ast->name->value),
            ast->get_location());
    }
    auto procType = std::dynamic_pointer_cast<ProcedureType>(*res);
    if (!procType) {
        throw TypeError(
            llvm::formatv("{0} is not a PROCEDURE", ast->name->value),
            ast->get_location());
    }

    if (ast->args.size() != procType->params.size()) {
        throw TypeError(
            llvm::formatv("calling PROCEDURE {0}, incorrect number of "
                          "arguments: {1} instead of {2}",
                          ast->name->value, ast->args.size(),
                          procType->params.size()),
            ast->get_location());
    }

    last_type = procType->ret;
}

void Inspector::visit_ASTIf(ASTIf *ast) {
    ast->if_clause.expr->accept(this);
    if (last_type != TypeTable::BoolType) {
        throw TypeError("IF expression must be type BOOLEAN",
                        ast->get_location());
    }

    std::for_each(ast->if_clause.stats.begin(), ast->if_clause.stats.end(),
                  [this](auto const &x) { x->accept(this); });

    std::for_each(ast->elsif_clause.begin(), ast->elsif_clause.end(),
                  [this, ast](auto const &x) {
                      x.expr->accept(this);
                      if (last_type != TypeTable::BoolType) {
                          throw TypeError(
                              "ELSIF expression must be type BOOLEAN",
                              ast->get_location());
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

void Inspector::visit_ASTFor(ASTFor *ast) {
    ast->start->accept(this);
    if (!last_type->is_numeric()) {
        throw TypeError("FOR start expression must be numeric type",
                        ast->get_location());
    }
    ast->end->accept(this);
    if (!last_type->is_numeric()) {
        throw TypeError("FOR end expression must be numeric type",
                        ast->get_location());
    }
    if (ast->by) {
        (*ast->by)->accept(this);
        if (!last_type->is_numeric()) {
            throw TypeError("FOR BY expression must be numeric type",
                            ast->get_location());
        }
    }

    // new symbol table
    auto former_symboltable = current_symboltable;
    current_symboltable =
        std::shared_ptr<SymbolTable<TypePtr>>(former_symboltable);
    current_symboltable->put(ast->ident->value, TypeTable::IntType);

    std::for_each(begin(ast->stats), end(ast->stats),
                  [this](auto const &s) { s->accept(this); });
    current_symboltable = former_symboltable;
}

void Inspector::visit_ASTWhile(ASTWhile *ast) {
    ast->expr->accept(this);
    if (last_type != TypeTable::BoolType) {
        throw TypeError("WHILE expression must be type BOOLEAN",
                        ast->get_location());
    }

    std::for_each(begin(ast->stats), end(ast->stats),
                  [this](auto const &x) { x->accept(this); });
}

void Inspector::visit_ASTRepeat(ASTRepeat *ast) {
    std::for_each(begin(ast->stats), end(ast->stats),
                  [this](auto const &x) { x->accept(this); });
    ast->expr->accept(this);
    if (last_type != TypeTable::BoolType) {
        throw TypeError("REPEAT expression must be type BOOLEAN",
                        ast->get_location());
    }
}

void Inspector::visit_ASTLoop(ASTLoop *ast) {
    std::for_each(begin(ast->stats), end(ast->stats),
                  [this](auto const &x) { x->accept(this); });
}

void Inspector::visit_ASTBlock(ASTBlock *ast) {
    std::for_each(begin(ast->stats), end(ast->stats),
                  [this](auto const &x) { x->accept(this); });
}

void Inspector::visit_ASTExpr(ASTExpr *ast) {
    ast->expr->accept(this);
    if (ast->relation) {
        auto t1 = last_type;
        (*ast->relation_expr)->accept(this);
        // Types have to be the same BOOLEANs or INTEGERs
        if (!last_type->equiv(t1)) {
            throw TypeError(
                llvm::formatv("types in expression don't match {0} and {1}",
                              std::string(*t1), std::string(*last_type)),
                ast->get_location());
        }
        // Comparison operators return BOOLEAN
        last_type = TypeTable::BoolType;
    }
}

void Inspector::visit_ASTSimpleExpr(ASTSimpleExpr *ast) {
    visit_ASTTerm(ast->term.get());
    auto t1 = last_type;
    std::for_each(ast->rest.begin(), ast->rest.end(), [this, ast, &t1](auto t) {
        t.second->accept(this);
        if (!last_type->equiv(t1)) {
            throw TypeError(
                llvm::formatv("types in expression don't match {0} and {1}",
                              std::string(*t1), std::string(*last_type)),
                ast->get_location());
        }
        if (t.first == TokenType::or_k) {
            if (last_type != TypeTable::BoolType) {
                throw TypeError("types in OR expression must be BOOLEAN",
                                ast->get_location());
            }
        } else {
            if (last_type != TypeTable::IntType) {
                throw TypeError(
                    llvm::formatv("types in {0} expression must be numeric",
                                  string(t.first)),
                    ast->get_location());
            }
        }
        t1 = last_type;
    });
}

void Inspector::visit_ASTTerm(ASTTerm *ast) {
    visit_ASTFactor(ast->factor.get());
    auto t1 = last_type;
    std::for_each(ast->rest.begin(), ast->rest.end(), [this, ast, &t1](auto t) {
        t.second->accept(this);
        if (!last_type->equiv(t1)) {
            throw TypeError(
                llvm::formatv("types in expression don't match {0} and {1}",
                              std::string(*t1), std::string(*last_type)),
                ast->get_location());
        }
        if (t.first == TokenType::ampersand) {
            if (last_type != TypeTable::BoolType) {
                throw TypeError("types in & expression must be BOOLEAN",
                                ast->get_location());
            }
        } else {
            if (last_type != TypeTable::IntType) {
                throw TypeError(
                    llvm::formatv("types in {0} expression must be numeric",
                                  string(t.first)),
                    ast->get_location());
            }
        }
        t1 = last_type;
    });
}

void Inspector::visit_ASTFactor(ASTFactor *ast) {
    std::visit(overloaded{
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
                                   "type in ~ expression must be BOOLEAN",
                                   ast->get_location());
                           }
                       }
                   },

               },
               ast->factor);
}

void Inspector::visit_ASTDesignator(ASTDesignator *ast) {
    debug("Inspector::visit_ASTDesignator");
    ast->ident->accept(this);

    // check type array before processing selectors
    auto array_type = std::dynamic_pointer_cast<ArrayType>(last_type);
    if (!array_type && !ast->selectors.empty()) {
        throw TypeError(
            llvm::formatv("variable {0} is not an array", ast->ident->value),
            ast->get_location());
    }

    // Not array type - no more checks.
    if (!array_type) {
        return;
    }
    debug("Inspector::visit_ASTDesignator type: {}", array_type->get_name());
    TypePtr current_type = array_type->base_type;

    auto count = 0;
    for (auto &s : ast->selectors) {
        count++;
        s->accept(this);
        if (!last_type->is_numeric()) {
            throw TypeError("expression in array index must be numeric",
                            ast->get_location());
        }

        debug("Inspector::visit_ASTDesignator selector: {}",
              current_type->get_name());
        last_type = current_type;
        array_type =
            std::dynamic_pointer_cast<ArrayType>(array_type->base_type);
        if (!array_type) {
            break;
        }
        current_type = array_type->base_type;
    }
    debug("Inspector::visit_ASTDesignator size: {} for {}",
          ast->selectors.size(), count);
    if (ast->selectors.size() > count) {
        throw TypeError(
            llvm::formatv("array indexes greater than array defintion: {0}",
                          std::string(*ast)),
            ast->get_location());
    }
}

void Inspector::visit_ASTType(ASTType *ast) {
    std::visit(
        overloaded{[this](auto arg) { ; },
                   [this, ast](std::shared_ptr<ASTIdentifier> const &type) {
                       auto result = types.find(type->value);
                       if (!result) {
                           throw TypeError(
                               llvm::formatv("Unknown type: {0}", type->value),
                               ast->get_location());
                       }
                       ast->type_info = *result;
                       last_type = *result;
                   },
                   [this, ast](std::shared_ptr<ASTArray> const &arg) {
                       arg->accept(this);
                       ast->type_info = last_type;
                   }},
        ast->type);
}

void Inspector::visit_ASTArray(ASTArray *ast) {
    ast->size->accept(this);
    if (!last_type->is_numeric()) {
        throw TypeError("ARRAY expecting numeric size", ast->get_location());
    }
    ast->type->accept(this);
    last_type = std::make_shared<ax::ArrayType>(last_type, ast->size->value);
    types.put(last_type->get_name(), last_type);
}

void Inspector::visit_ASTIdentifier(ASTIdentifier *ast) {
    debug("Inspector::visit_ASTIdentifier");
    auto res = current_symboltable->find(ast->value);
    if (!res) {
        throw CodeGenException(
            llvm::formatv("undefined identifier {0}", ast->value),
            ast->get_location());
    }
    // debug("find type: {} for {}", res, res->name);
    auto resType = types.find((*res)->get_name());
    if (!resType) {
        throw TypeError(llvm::formatv("Unknown type: {0} for identifier {1}",
                                      (*res)->get_name(), ast->value),
                        ast->get_location());
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