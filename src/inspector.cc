//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "inspector.hh"

#include <algorithm>
#include <iostream>
#include <llvm/Support/FormatVariadic.h>
#include <memory>

#include "ast.hh"
#include "error.hh"
#include "symbol.hh"
#include "type.hh"

namespace ax {

inline constexpr bool debug_inspect{false};

template <typename... T> inline void debug(const T &... msg) {
    if constexpr (debug_inspect) {
        std::cout << std::string(llvm::formatv(msg...)) << std::endl;
    }
}

Inspector::Inspector(SymbolFrameTable &s, TypeTable &t, ErrorManager &e, Importer &i)
    : symboltable(s), types(t), errors(e), importer(i){};

void Inspector::visit_ASTModule(ASTModulePtr ast) {
    debug("Inspector::visit_ASTModule");
    if (ast->import) {
        ast->import->accept(this);
    }
    ast->decs->accept(this);
    std::for_each(ast->procedures.begin(), ast->procedures.end(),
                  [this](auto const &proc) { proc->accept(this); });

    last_proc = nullptr;

    std::for_each(ast->stats.begin(), ast->stats.end(),
                  [this](auto const &x) { x->accept(this); });
}

void Inspector::visit_ASTImport(ASTImportPtr ast) {
    debug("Inspector::visit_ASTImport");
    std::for_each(begin(ast->imports), end(ast->imports), [this](auto const &i) {
        auto found = importer.find_module(i.first->value, symboltable, types);
        if (!found) {
            throw TypeError(llvm::formatv("MODULE {0} not found", i.first->value),
                            i.first->get_location());
        }
    });
}

void Inspector::visit_ASTConst(ASTConstPtr ast) {
    debug("Inspector::visit_ASTConst");
    std::for_each(begin(ast->consts), end(ast->consts), [this, ast](auto &c) {
        c.value->accept(this);
        if (!is_const) {
            auto e =
                TypeError(llvm::formatv("CONST {0} is not a constant expression", c.ident->value),
                          ast->get_location());
            errors.add(e);
        }
        if (c.ident->is(Attr::read_only)) {
            auto e = TypeError(llvm::formatv("CONST {0} is always read only", c.ident->value),
                               ast->get_location());
            errors.add(e);
        }
        c.type = std::make_shared<ASTType>();
        c.type->type_info = last_type;
        c.type->type = make<ASTQualident>(last_type->get_name());
        debug("Inspector::visit_ASTConst type: {0}", last_type->get_name());
        symboltable.put(c.ident->value, mkSym(last_type, Attr::cnst));
    });
}

void Inspector::visit_ASTTypeDec(ASTTypeDecPtr ast) {
    debug("Inspector::visit_ASTTypeDec");
    std::for_each(begin(ast->types), end(ast->types), [this, ast](auto const &t) {
        if (t.first->is(Attr::read_only)) {
            auto e = TypeError(llvm::formatv("TYPE {0} is always read only", t.first->value),
                               ast->get_location());
            errors.add(e);
        }
        t.second->accept(this);
        auto type = std::make_shared<TypeAlias>(t.first->value, last_type);
        debug("Inspector::visit_ASTTypeDec put type {0}", t.first->value);
        types.put(t.first->value, type);
    });
}

void Inspector::visit_ASTVar(ASTVarPtr ast) {
    debug("Inspector::visit_ASTVar");

    std::for_each(ast->vars.begin(), ast->vars.end(), [this](auto const &v) {
        // No need to check the identifier - its being
        // defined.
        v.second->accept(this);
        // Update VAR declaration symbols with type
        symboltable.put(v.first->value, mkSym(last_type));
    });
}

void Inspector::visit_ASTProcedure(ASTProcedurePtr ast) {
    debug("Inspector::visit_ASTProcedure");
    // Check return type
    auto retType = TypeTable::VoidType;
    if (ast->return_type != nullptr) {
        ast->return_type->accept(this);
        retType = last_type;
    }

    // Check global modifiers
    if (ast->name->is(Attr::read_only)) {
        auto e = TypeError(llvm::formatv("PROCEDURE {0} is always read only", ast->name->value),
                           ast->get_location());
        errors.add(e);
    }

    // Check parameter types
    debug("Inspector::visit_ASTProcedure check parameter types");
    ProcedureType::ParamsList argTypes;
    std::for_each(ast->params.begin(), ast->params.end(), [this, &argTypes](auto const &p) {
        p.second->accept(this);
        auto attr = Attr::null;
        if (p.first->is(Attr::var)) {
            attr = Attr::var;
        }
        argTypes.push_back({last_type, attr});
    });

    auto proc_type = std::make_shared<ProcedureType>(retType, argTypes);
    symboltable.put(ast->name->value, mkSym(proc_type));

    last_proc = ast;

    debug("Inspector::visit_ASTProcedure new symbol table");
    // new symbol table
    symboltable.push_frame(ast->name->value);
    std::for_each(ast->params.begin(), ast->params.end(), [this](auto const &p) {
        std::visit(
            overloaded{
                [this](auto arg) { arg->accept(this); }, // lambda arg can't be reference here
                [this, p](ASTQualidentPtr const &tname) {
                    debug("Inspector::visit_ASTProcedure param type ident");
                    auto type = types.find(tname->id->value);
                    symboltable.put(p.first->value,
                                    mkSym(type, p.first->is(Attr::var) ? Attr::var : Attr::null));
                }},
            p.second->type);
        symboltable.put(p.first->value,
                        mkSym(last_type, p.first->is(Attr::var) ? Attr::var : Attr::null));
    });
    if (ast->decs) {
        ast->decs->accept(this);
    }
    std::for_each(ast->stats.begin(), ast->stats.end(),
                  [this, ast](auto const &x) { x->accept(this); });
    symboltable.pop_frame();
}

void Inspector::visit_ASTAssignment(ASTAssignmentPtr ast) {
    debug("Inspector::visit_ASTAssignment");
    ast->expr->accept(this);
    auto expr_type = last_type;

    auto res = symboltable.find(ast->ident->ident->make_coded_id());
    if (res) {
        if (res->is(Attr::cnst)) {
            auto e = TypeError(
                llvm::formatv("Can't assign to CONST variable {0}", std::string(*ast->ident)),
                ast->get_location());
            errors.add(e);
            return;
        }
        if (res->is(Attr::read_only)) {
            auto e = TypeError(llvm::formatv("Can't assign to read only (-) variable {0}",
                                             std::string(*ast->ident)),
                               ast->get_location());
            errors.add(e);
            return;
        }
    };

    ast->ident->accept(this);
    debug("type of ident: {} ", last_type->get_name());
    auto alias = types.resolve(last_type->get_name());
    if (!alias) {
        auto e = TypeError(llvm::formatv("Can't assign expression of type {0} to {1}",
                                         std::string(*expr_type), std::string(*ast->ident)),
                           ast->get_location());
        errors.add(e);
    }
    last_type = *alias;
    if (!last_type->equiv(expr_type)) {
        auto e = TypeError(llvm::formatv("Can't assign expression of type {0} to {1}",
                                         std::string(*expr_type), std::string(*ast->ident)),
                           ast->get_location());
        errors.add(e);
    }
}

void Inspector::visit_ASTReturn(ASTReturnPtr ast) {
    debug("Inspector::visit_ASTReturn");
    TypePtr expr_type = TypeTable::VoidType;
    if (ast->expr) {
        ast->expr->accept(this);
        expr_type = last_type;
    }

    // check return type
    if (last_proc) {
        auto type = last_proc->return_type;
        auto retType = TypeTable::VoidType;
        if (type != nullptr) {
            std::visit(overloaded{
                           [this, &retType](auto arg) {
                               arg->accept(this);
                               retType = last_type;
                           },
                           [this, &retType](ASTQualidentPtr const &type) {
                               if (auto t = types.find(type->id->value); t) {
                                   retType = t;
                               };
                           },
                       },
                       type->type);
        }

        if (!expr_type->equiv(retType)) {
            auto e = TypeError(llvm::formatv("RETURN does not match return type for function {0}",
                                             last_proc->name->value),
                               ast->get_location());
            errors.add(e);
        }
    } // namespace ax
}

void Inspector::visit_ASTCall(ASTCallPtr ast) {
    debug("Inspector::visit_ASTCall");
    // auto name = ast->name->ident->make_coded_id();
    auto name = get_Qualident(ast->name->ident);

    debug("Inspector::visit_ASTCall - {0} {1}", name, name);
    auto res = symboltable.find(name);
    if (!res) {
        std::replace(begin(name), end(name), '_', '.');
        throw CodeGenException(llvm::formatv("undefined PROCEDURE {0}", name),
                               ast->get_location());
    }
    auto procType = std::dynamic_pointer_cast<ProcedureType>(res->type);
    if (!procType) {
        std::replace(begin(name), end(name), '_', '.');
        throw TypeError(llvm::formatv("{0} is not a PROCEDURE", name), ast->get_location());
    }

    if (ast->args.size() != procType->params.size()) {
        std::replace(begin(name), end(name), '_', '.');
        throw TypeError(llvm::formatv("calling PROCEDURE {0}, incorrect number of "
                                      "arguments: {1} instead of {2}",
                                      name, ast->args.size(), procType->params.size()),
                        ast->get_location());
    }

    // Check argument types
    auto proc_iter = procType->params.begin();
    for (auto call_iter = ast->args.begin(); call_iter != ast->args.end();
         call_iter++, proc_iter++) {

        (*call_iter)->accept(this);
        auto base_last = types.resolve(last_type->get_name());
        auto proc_base = types.resolve((*proc_iter).first->get_name());

        if (!(*base_last)->equiv(*proc_base)) {
            std::replace(begin(name), end(name), '_', '.');
            auto e = TypeError(llvm::formatv("procedure call {0} has incorrect "
                                             "type {1} for parameter {2}",
                                             name, last_type->get_name(),
                                             (*proc_iter).first->get_name()),
                               ast->get_location());
            errors.add(e);
        }

        if ((*proc_iter).second == Attr::var && !is_lvalue) {
            debug("Inspector::visit_ASTCall is_lvalue");
            std::replace(begin(name), end(name), '_', '.');
            auto e = TypeError(llvm::formatv("procedure call {0} does not have a variable "
                                             "reference for VAR parameter {2}",
                                             name, last_type->get_name(),
                                             (*proc_iter).first->get_name()),
                               ast->get_location());
            errors.add(e);
        }
    }

    last_type = procType->ret;
}

void Inspector::visit_ASTIf(ASTIfPtr ast) {
    ast->if_clause.expr->accept(this);
    if (last_type != TypeTable::BoolType) {
        auto e = TypeError("IF expression must be type BOOLEAN", ast->get_location());
        errors.add(e);
    }

    std::for_each(ast->if_clause.stats.begin(), ast->if_clause.stats.end(),
                  [this](auto const &x) { x->accept(this); });

    std::for_each(ast->elsif_clause.begin(), ast->elsif_clause.end(), [this, ast](auto const &x) {
        x.expr->accept(this);
        if (last_type != TypeTable::BoolType) {
            auto e = TypeError("ELSIF expression must be type BOOLEAN", ast->get_location());
            errors.add(e);
        }
        std::for_each(x.stats.begin(), x.stats.end(), [this](auto const &s) { s->accept(this); });
    });
    if (ast->else_clause) {
        auto elses = *ast->else_clause;
        std::for_each(begin(elses), end(elses), [this](auto const &s) { s->accept(this); });
    }
}

void Inspector::visit_ASTFor(ASTForPtr ast) {
    ast->start->accept(this);
    if (!last_type->is_numeric()) {
        auto e = TypeError("FOR start expression must be numeric type", ast->get_location());
        errors.add(e);
    }
    ast->end->accept(this);
    if (!last_type->is_numeric()) {
        auto e = TypeError("FOR end expression must be numeric type", ast->get_location());
        errors.add(e);
    }
    if (ast->by) {
        (*ast->by)->accept(this);
        if (!last_type->is_numeric()) {
            auto e = TypeError("FOR BY expression must be numeric type", ast->get_location());
            errors.add(e);
        }
    }

    // new frame
    symboltable.push_frame("for");
    symboltable.put(ast->ident->value, mkSym(TypeTable::IntType));

    std::for_each(begin(ast->stats), end(ast->stats), [this](auto const &s) { s->accept(this); });
    symboltable.pop_frame();
}

void Inspector::visit_ASTWhile(ASTWhilePtr ast) {
    ast->expr->accept(this);
    if (last_type != TypeTable::BoolType) {
        auto e = TypeError("WHILE expression must be type BOOLEAN", ast->get_location());
        errors.add(e);
    }

    std::for_each(begin(ast->stats), end(ast->stats), [this](auto const &x) { x->accept(this); });
}

void Inspector::visit_ASTRepeat(ASTRepeatPtr ast) {
    std::for_each(begin(ast->stats), end(ast->stats), [this](auto const &x) { x->accept(this); });
    ast->expr->accept(this);
    if (last_type != TypeTable::BoolType) {
        auto e = TypeError("REPEAT expression must be type BOOLEAN", ast->get_location());
        errors.add(e);
    }
}

void Inspector::visit_ASTLoop(ASTLoopPtr ast) {
    std::for_each(begin(ast->stats), end(ast->stats), [this](auto const &x) { x->accept(this); });
}

void Inspector::visit_ASTBlock(ASTBlockPtr ast) {
    std::for_each(begin(ast->stats), end(ast->stats), [this](auto const &x) { x->accept(this); });
}

void Inspector::visit_ASTExpr(ASTExprPtr ast) {
    ast->expr->accept(this);
    auto c1 = is_const;
    if (ast->relation) {
        is_lvalue = false;
        auto t1 = last_type;
        (*ast->relation_expr)->accept(this);
        // Types have to be the same BOOLEANs or INTEGERs
        if (!last_type->equiv(t1)) {
            auto e = TypeError(llvm::formatv("types in expression don't match {0} and {1}",
                                             std::string(*t1), std::string(*last_type)),
                               ast->get_location());
            errors.add(e);
        }
        // Comparison operators return BOOLEAN
        last_type = TypeTable::BoolType;
        c1 = c1 && is_const; // if both are const
        is_const = c1;       // return the const value
    }
}

void Inspector::visit_ASTSimpleExpr(ASTSimpleExprPtr ast) {
    ast->term->accept(this);
    auto t1 = last_type;
    auto c1 = is_const;
    for (auto const &t : ast->rest) {
        is_lvalue = false;
        t.second->accept(this);
        if (!last_type->equiv(t1)) {
            auto e = TypeError(llvm::formatv("types in expression don't match {0} and {1}",
                                             std::string(*t1), std::string(*last_type)),
                               ast->get_location());
            errors.add(e);
        }
        if (t.first == TokenType::or_k) {
            if (last_type != TypeTable::BoolType) {
                auto e = TypeError("types in OR expression must be BOOLEAN", ast->get_location());
                errors.add(e);
            }
        } else {
            if (last_type != TypeTable::IntType) {
                auto e = TypeError(
                    llvm::formatv("types in {0} expression must be numeric", string(t.first)),
                    ast->get_location());
                errors.add(e);
            }
        }
        t1 = last_type;
        c1 = c1 && is_const; // if both are const
        is_const = c1;       // return the const value
    };
}

void Inspector::visit_ASTTerm(ASTTermPtr ast) {
    ast->factor->accept(this);
    auto t1 = last_type;
    auto c1 = is_const;
    for (auto const &t : ast->rest) {
        is_lvalue = false;
        t.second->accept(this);
        if (!last_type->equiv(t1)) {
            auto e = TypeError(llvm::formatv("types in expression don't match {0} and {1}",
                                             std::string(*t1), std::string(*last_type)),
                               ast->get_location());
            errors.add(e);
        }
        if (t.first == TokenType::ampersand) {
            if (last_type != TypeTable::BoolType) {
                auto e = TypeError("types in & expression must be BOOLEAN", ast->get_location());
                errors.add(e);
            }
        } else {
            if (last_type != TypeTable::IntType) {
                auto e = TypeError(
                    llvm::formatv("types in {0} expression must be numeric", string(t.first)),
                    ast->get_location());
                errors.add(e);
            }
        }
        c1 = c1 && is_const; // if both are const
        is_const = c1;       // return the const value

        t1 = last_type;
    };
}

void Inspector::visit_ASTFactor(ASTFactorPtr ast) {
    std::visit(overloaded{
                   [this](auto factor) { factor->accept(this); },
                   [this](ASTCallPtr const &factor) {
                       // need to pass on return type */
                       factor->accept(this);
                       is_lvalue = false;
                   },
                   [this, ast](ASTFactorPtr const &arg) {
                       if (ast->is_not) {
                           arg->accept(this);
                           if (last_type != TypeTable::BoolType) {
                               auto e = TypeError("type in ~ expression must be BOOLEAN",
                                                  ast->get_location());
                               errors.add(e);
                           }
                           is_lvalue = false;
                       }
                   },

               },
               ast->factor);
}

void Inspector::visit_ASTDesignator(ASTDesignatorPtr ast) {
    debug("Inspector::visit_ASTDesignator");
    ast->ident->accept(this);

    // check type array before processing selectors
    auto array_type = std::dynamic_pointer_cast<ArrayType>(last_type);
    auto record_type = std::dynamic_pointer_cast<RecordType>(last_type);
    if (!(array_type || record_type) && !ast->selectors.empty()) {
        auto e = TypeError(
            llvm::formatv("variable {0} is not an array or record", ast->ident->id->value),
            ast->get_location());
        errors.add(e);
    }

    // Not array or record type - no more checks.
    if (!(array_type || record_type)) {
        is_lvalue = true;
        return;
    }

    is_lvalue = false; // no lvalues for aggregate values
    for (auto &ss : ast->selectors) {

        // can't do a std::visit as need to break out this loop
        if (std::holds_alternative<ASTExprPtr>(ss)) {

            // do ARRAY type
            debug("Inspector::visit_ASTDesignator array index");
            auto s = std::get<ASTExprPtr>(ss);
            if (!array_type) {
                auto e = TypeError("value not ARRAY", s->get_location());
                errors.add(e);
                return;
            }

            s->accept(this);
            if (!last_type->is_numeric()) {
                auto e =
                    TypeError("expression in array index must be numeric", ast->get_location());
                errors.add(e);
            }
            last_type = array_type->base_type;
        } else if (std::holds_alternative<FieldRef>(ss)) {

            // do RECORD type
            debug("Inspector::visit_ASTDesignator record field");
            auto &s = std::get<FieldRef>(ss);
            if (!record_type) {
                auto e = TypeError("value not RECORD", s.first->get_location());
                errors.add(e);
                return;
            }

            auto field = record_type->get_type(s.first->value);
            if (!field) {
                auto e = TypeError(llvm::formatv("no field <{0}> in RECORD", s.first->value),
                                   ast->get_location());
                errors.add(e);
                return;
            }

            // Store field index with identifier, for use later in code
            // generator.

            s.second = record_type->get_index(s.first->value);
            debug("Inspector::visit_ASTDesignator record index {0} for {1}", s.second,
                  s.first->value);

            last_type = *field;
        }
        array_type = std::dynamic_pointer_cast<ArrayType>(last_type);
        record_type = std::dynamic_pointer_cast<RecordType>(last_type);
    }
}

void Inspector::visit_ASTType(ASTTypePtr ast) {
    debug("Inspector::visit_ASTType");
    std::visit(overloaded{[this, ast](ASTQualidentPtr const &type) {
                              debug("Inspector::visit_ASTType {0}", type->id->value);
                              auto result = types.find(type->id->value);
                              if (!result) {
                                  throw TypeError(
                                      llvm::formatv("Unknown type: {0}", type->id->value),
                                      ast->get_location());
                              }
                              debug("Inspector::visit_ASTType 2 {0}", type->id->value);
                              ast->type_info = result;
                              last_type = result;
                          },
                          [this, ast](ASTArrayPtr const &arg) {
                              arg->accept(this);
                              ast->type_info = last_type;
                          },
                          [this, ast](ASTRecordPtr const &arg) {
                              arg->accept(this);
                              ast->type_info = last_type;
                          }},
               ast->type);
}

void Inspector::visit_ASTArray(ASTArrayPtr ast) {
    ast->size->accept(this);
    if (!last_type->is_numeric()) {
        auto e = TypeError("ARRAY expecting numeric size", ast->get_location());
        errors.add(e);
    }
    if (!is_const) {
        auto e = TypeError("ARRAY expecting constant expression", ast->get_location());
        errors.add(e);
    }
    ast->type->accept(this);
    last_type = std::make_shared<ax::ArrayType>(last_type, ast->size->value);
    types.put(last_type->get_name(), last_type);
}

void Inspector::visit_ASTRecord(ASTRecordPtr ast) {
    debug("Inspector::visit_ASTRecord");
    auto rec_type = std::make_shared<ax::RecordType>();
    std::for_each(begin(ast->fields), end(ast->fields), [this, rec_type, ast](auto const &v) {
        // check types
        v.second->accept(this);

        // check if not already defined
        if (rec_type->has_field(v.first->value)) {
            auto e = TypeError(llvm::formatv("RECORD already has field {0}", v.first->value),
                               v.first->get_location());
            errors.add(e);
        }
        rec_type->insert(v.first->value, last_type);
    });
    last_type = rec_type;
    types.put(last_type->get_name(), last_type);
}

std::string Inspector::get_Qualident(ASTQualidentPtr const &ast) {
    std::string result;
    auto        res = symboltable.find(ast->qual);

    if (ast->qual.empty()) {
        return ast->id->value;
    }
    if (res && res->type->id == TypeId::module) {
        auto module_name = std::dynamic_pointer_cast<ModuleType>(res->type)->module_name();
        result = ASTQualident::make_coded_id(module_name, ast->id->value);
        // Rewrite AST with real module name
        ast->qual = module_name;
    }
    return result;
}

void Inspector::visit_ASTQualident(ASTQualidentPtr ast) {
    debug("Inspector::visit_ASTQualident");
    if (ast->qual.empty()) {
        visit_ASTIdentifier(ast->id);
    } else {
        debug("Inspector::visit_ASTQualident {0}", ast->qual);

        auto new_ast = make<ASTIdentifier>();
        new_ast->value = get_Qualident(ast);
        is_qualid = true;
        qualid_error = false;
        visit_ASTIdentifier(new_ast);
        if (qualid_error) {
            auto e = CodeGenException(
                llvm::formatv("undefined identifier {0} in MODULE {1}", ast->id->value, ast->qual),
                ast->get_location());
            errors.add(e);
        }
    }
}

void Inspector::visit_ASTIdentifier(ASTIdentifierPtr ast) {
    debug("Inspector::visit_ASTIdentifier");
    auto res = symboltable.find(ast->value);
    if (!res) {
        if (!is_qualid) {
            throw CodeGenException(llvm::formatv("undefined identifier {0}", ast->value),
                                   ast->get_location());
        } else {
            qualid_error = true;
            return;
        }
    }
    // debug("find type: {} for {}", res, res->name);
    auto resType = types.resolve(res->type->get_name());
    if (!resType) {
        auto e = TypeError(llvm::formatv("Unknown type: {0} for identifier {1}",
                                         res->type->get_name(), ast->value),
                           ast->get_location());
        errors.add(e);
        return;
    }
    last_type = *resType;
    is_const = res->is(Attr::cnst);
    is_lvalue = true;
}

void Inspector::visit_ASTInteger(ASTIntegerPtr /* not used */) {
    last_type = TypeTable::IntType;
    is_const = true;
    is_lvalue = false;
}

void Inspector::visit_ASTChar(ASTCharPtr /* not used */) {
    last_type = TypeTable::CharType;
    is_const = true;
    is_lvalue = false;
}

void Inspector::visit_ASTBool(ASTBoolPtr /* not used */) {
    last_type = TypeTable::BoolType;
    is_const = true;
    is_lvalue = false;
}

} // namespace ax