//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "inspector.hh"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>

#include <fmt/core.h>

#include <llvm/Support/Debug.h>

#include "ast.hh"
#include "astvisitor.hh"
#include "error.hh"
#include "symbol.hh"
#include "token.hh"
#include "type.hh"
#include "typetable.hh"

namespace ax {

constexpr auto closure_arg{"_closure"};

#define DEBUG_TYPE "inspector "

template <typename... T> static void debug(const T &...msg) {
    LLVM_DEBUG(llvm::dbgs() << DEBUG_TYPE << fmt::format(msg...) << '\n'); // NOLINT
}

Inspector::Inspector(SymbolFrameTable &s, TypeTable &t, ErrorManager &e, Importer &i)
    : symboltable(s), types(t), errors(e), importer(i){};

void Inspector::visit_ASTModule(ASTModule ast) {
    debug("ASTModule");
    if (ast->import) {
        ast->import->accept(this);
    }

    variable_type = Attr::global_var;
    ast->decs->accept(this);

    variable_type = Attr::local_var;
    std::for_each(ast->procedures.begin(), ast->procedures.end(),
                  [this](auto const &proc) { proc->accept(this); });

    last_proc = nullptr;

    std::for_each(ast->stats.begin(), ast->stats.end(),
                  [this](auto const &x) { x->accept(this); });
}

void Inspector::visit_ASTImport(ASTImport ast) {
    debug("ASTImport");
    std::for_each(begin(ast->imports), end(ast->imports), [this](auto const &i) {
        const auto &[name, _] = i;
        auto found = importer.find_module(name->value, symboltable, types);
        if (!found) {
            throw TypeError(fmt::format("MODULE {0} not found", name->value),
                            name->get_location());
        }
    });
}

void Inspector::visit_ASTConst(ASTConst ast) {
    debug("ASTConst");
    std::for_each(begin(ast->consts), end(ast->consts), [this, ast](auto &c) {
        c.value->accept(this);
        if (!is_const) {
            auto e =
                TypeError(fmt::format("CONST {0} is not a constant expression", c.ident->value),
                          ast->get_location());
            errors.add(e);
        }
        if (c.ident->is(Attr::read_only)) {
            auto e = TypeError(fmt::format("CONST {0} is always read only", c.ident->value),
                               ast->get_location());
            errors.add(e);
        }
        c.type = std::make_shared<ASTType_>();
        c.type->set_type(last_type);
        c.type->type = make<ASTQualident_>(last_type->get_name());
        debug("ASTConst type: {0}", last_type->get_name());
        auto sym = mkSym(last_type, Attr::cnst);
        sym->set(variable_type);
        symboltable.put(c.ident->value, sym);
    });
}

void Inspector::visit_ASTTypeDec(ASTTypeDec ast) {
    debug("ASTTypeDec");
    for (auto const &[name, type_expr] : ast->types) {
        if (types.find(name->value)) {
            auto e = TypeError(fmt::format("TYPE {0} already defined", name->value),
                               ast->get_location());
            errors.add(e);
            continue;
        }
        type_expr->accept(this);
        if (last_type->id == TypeId::record) {
            std::dynamic_pointer_cast<RecordType>(last_type)->set_identified(name->value);
        }
        auto type = std::make_shared<TypeAlias>(name->value, last_type);
        debug("ASTTypeDec put type {0}", name->value);
        types.put(name->value, type);
    };
}

void Inspector::visit_ASTVar(ASTVar ast) {
    debug("ASTVar");

    std::for_each(ast->vars.begin(), ast->vars.end(), [this](auto const &v) {
        auto const &[name, expr] = v;
        // No need to check the identifier - its being
        // defined.
        expr->accept(this);
        // Update VAR declaration symbols with type
        debug("var type: {0}", last_type->get_name());
        symboltable.put(name->value, mkSym(last_type, variable_type));
    });

    // Post process all pointer defintions
    for (auto &ptr_type : pointer_types) {
        if (!ptr_type->get_reference()) {
            debug("ASTVar resolve pointer type: {0}", ptr_type->get_ref_name());
            auto ref = types.resolve(ptr_type->get_ref_name());
            if (!ref) {
                auto e = TypeError(fmt::format("TYPE {0} not found", ptr_type->get_ref_name()),
                                   ast->get_location());
                errors.add(e);
            }
            ptr_type->set_reference(ref);
        }
    };
    pointer_types.clear();
}

void Inspector::do_receiver(RecVar &r) {
    if (!r.first) {
        return;
    }
    auto t = types.resolve(r.second->value);
    if (!t) {
        auto e = TypeError(
            fmt::format("bound type {0} not found for type-bound PROCEDURE", r.second->value),
            r.second->get_location());
        errors.add(e);
        return;
    }
    if (t->id != TypeId::record && !is_ptr_to_record(t)) {
        auto e = TypeError(
            fmt::format(
                "bound type {0} must be a RECORD or POINTER TO RECORD in type-bound PROCEDURE",
                r.second->value),
            r.second->get_location());
        errors.add(e);
        return;
    }
    r.second->set_type(t);
}

std::pair<TypePtr, ProcedureType::ParamsList> Inspector::do_proc(ASTProc_ &ast) {
    // Check name if not defined;
    if (auto name_check = symboltable.find(ast.name->value);
        name_check && name_check->type->id != TypeId::procedureFwd) {
        auto e =
            TypeError(fmt::format("PROCEDURE {0}, identifier is already defined", ast.name->value),
                      ast.get_location());
        errors.add(e);
    }

    // Check return type
    auto retType = TypeTable::VoidType;
    if (ast.return_type != nullptr) {
        ast.return_type->accept(this);
        retType = last_type;
    }

    // Check global modifiers
    if (ast.name->is(Attr::read_only)) {
        auto e = TypeError(fmt::format("PROCEDURE {0} is always read only", ast.name->value),
                           ast.get_location());
        errors.add(e);
    }

    // Check receiver
    do_receiver(ast.receiver);

    // Check parameter types
    debug("ASTProcedure check parameter types");
    ProcedureType::ParamsList argTypes;
    std::for_each(ast.params.begin(), ast.params.end(), [this, &argTypes](auto const &p) {
        p.second->accept(this); // type
        auto attr = Attr::null;
        if (p.first->is(Attr::var)) {
            attr = Attr::var;
        }
        argTypes.emplace_back(last_type, attr);
    });
    return {retType, argTypes};
}

void Inspector::visit_ASTProcedure(ASTProcedure ast) {
    debug("ASTProcedure: {0}", ast->name->value);

    auto [retType, argTypes] = do_proc(*ast);

    auto proc_type = std::make_shared<ProcedureType>(retType, argTypes);
    auto sym = mkSym(proc_type, Attr::global_var);
    symboltable.put(ast->name->value, sym);

    last_proc = ast;

    // new symbol table
    symboltable.push_frame(ast->name->value);

    // do receiver
    if (ast->receiver.first) {
        auto attr = ast->receiver.first->is(Attr::var) ? Attr::var : Attr::null;
        symboltable.put(ast->receiver.first->value, mkSym(ast->receiver.second->get_type(), attr));
        if (ast->receiver.second->get_type()) {
            debug("ASTProcedure: receiver type {0}", string(ast->receiver.second->get_type()));
            proc_type->receiver = ast->receiver.second->get_type();
            proc_type->receiver_type = attr;
        }
    }

    int count = 0;
    for (auto const &[p, type] : ast->params) {
        auto const &param = p;
        std::visit(
            overloaded{
                [this](auto arg) { arg->accept(this); }, // lambda arg can't be reference here
                [this, param](ASTQualident const &tname) {
                    debug("ASTProcedure param type ident");
                    auto type = types.find(tname->id->value);
                    symboltable.put(param->value,
                                    mkSym(type, param->is(Attr::var) ? Attr::var : Attr::null));
                }},
            type->type);
        symboltable.put(param->value, mkSym(argTypes[count].first,
                                            param->is(Attr::var) ? Attr::var : Attr::null));
        count++;
    };
    if (ast->decs) {
        ast->decs->accept(this);
    }

    // check local procedures
    std::for_each(cbegin(ast->procedures), cend(ast->procedures),
                  [this](auto const &proc) { proc->accept(this); });

    // check statements
    symboltable.reset_free_variables();
    std::for_each(cbegin(ast->stats), cend(ast->stats),
                  [this, ast](auto const &x) { x->accept(this); });
    auto free_variables = symboltable.get_free_variables();
    for (auto const &f : free_variables) {
        if (f == ast->name->value) {
            // skip recursive defintions
            continue;
        }
        auto sym = symboltable.find(f);
        if (sym->is(Attr::global_var)) {
            continue;
        }

        Attrs attrs;
        attrs.set(Attr::free_var);
        if (sym->is(Attr::modified)) {
            attrs.set(Attr::modified);
        }
        debug("ASTProcedure {0}: Free variable {1} {2}", ast->name->value, f,
              attrs.contains(Attr::modified) ? "Modified" : "");
        ast->free_variables.emplace_back(f, attrs);

        // add to type def
        proc_type->free_vars.emplace_back(f, sym->type);
    };
    if (!ast->free_variables.empty()) {
        debug("ASTProcedure: {0} closure function", ast->name->value);
        sym->set(Attr::closure);
        // This defines the closure as an int which is not true, but is corrected in the codegen
        auto        type = std::make_shared<ASTType_>();
        std::string c{"INTEGER"};
        type->type = std::make_shared<ASTQualident_>(c);
        auto v = std::make_pair(std::make_shared<ASTIdentifier_>(closure_arg), type);
        ast->params.insert(ast->params.begin(), v);
        symboltable.put(closure_arg, mkSym(proc_type->get_closure_struct(), Attr::closure));
    }
    symboltable.pop_frame();
}

void Inspector::visit_ASTProcedureForward(ASTProcedureForward ast) {
    debug("ASTProcedureForward: {0}", ast->name->value);
    auto [retType, argTypes] = do_proc(*ast);

    auto proc_type = std::make_shared<ProcedureFwdType>();
    proc_type->ret = retType;
    proc_type->params = argTypes;
    // Override the defintion placed in the symbol table by the parser
    symboltable.put(ast->name->value, mkSym(proc_type, Attr::global_var));
};

void Inspector::visit_ASTAssignment(ASTAssignment ast) {
    debug("ASTAssignment");
    ast->expr->accept(this);
    auto expr_type = last_type;

    auto res = symboltable.find(ast->ident->ident->make_coded_id());
    if (res) {
        if (res->is(Attr::cnst)) {
            auto e = TypeError(
                fmt::format("Can't assign to CONST variable {0}", std::string(*ast->ident)),
                ast->get_location());
            errors.add(e);
            return;
        }
        if (res->is(Attr::read_only)) {
            auto e = TypeError(fmt::format("Can't assign to read only (-) variable {0}",
                                           std::string(*ast->ident)),
                               ast->get_location());
            errors.add(e);
            return;
        }
        res->set(Attr::modified);
    };

    ast->ident->accept(this);
    if (!last_type) {
        // error return;
        return;
    }
    auto alias = types.resolve(last_type->get_name());
    assert(alias); // NOLINT
    debug("ASTAssignment type of ident: {0} -> {1}", last_type->get_name(), alias->get_name());
    last_type = alias;
    if (!(types.check(TokenType::assign, last_type, expr_type) ||
          (last_type->is_assignable() && last_type->equiv(expr_type)))) {
        auto e = TypeError(fmt::format("Can't assign expression of type {0} to {1}",
                                       string(expr_type), std::string(*ast->ident)),
                           ast->get_location());
        errors.add(e);
    }
}

void Inspector::visit_ASTReturn(ASTReturn ast) {
    debug("ASTReturn");
    TypePtr expr_type = TypeTable::VoidType;
    if (ast->expr) {
        ast->expr->accept(this);
        if (last_type) {
            expr_type = types.resolve(last_type->get_name());
        }
    }

    // check return type
    if (last_proc) {
        auto type = last_proc->return_type;
        auto retType = TypeTable::VoidType;
        if (type != nullptr) {
            std::visit(overloaded{
                           [this, &retType](auto arg) {
                               arg->accept(this);
                               retType = types.resolve(last_type->get_name());
                           },
                           [this, &retType](ASTQualident const &type) {
                               if (auto t = types.resolve(type->id->value); t) {
                                   retType = t;
                               };
                           },
                       },
                       type->type);
        }

        if (!expr_type->equiv(retType)) {
            auto e = TypeError(
                fmt::format("RETURN type ({1}) does not match return type for function {0}: {2}",
                            last_proc->name->value, retType->get_name(), expr_type->get_name()),
                ast->get_location());
            errors.add(e);
        }
    }
}

void Inspector::visit_ASTCall(ASTCall ast) {
    auto name = get_Qualident(ast->name->ident);
    bool skip_argument_typecheck{false};

    debug("ASTCall: {0}", name);
    auto res = symboltable.find(name);
    if (!res) {
        std::replace(begin(name), end(name), '_', '.');
        auto e =
            CodeGenException(fmt::format("undefined PROCEDURE {0}", name), ast->get_location());
        errors.add(e);
        return;
    }
    auto procType = std::dynamic_pointer_cast<ProcedureType>(res->type);
    if (!procType) {

        // check bound procedure
        auto field = ast->name->first_field();
        if (!field) {
            std::replace(begin(name), end(name), '_', '.');
            auto e = TypeError(fmt::format("{0} is not a PROCEDURE", name), ast->get_location());
            errors.add(e);
            return;
        }

        name = field->value;
        if (res = symboltable.find(name); !res) {
            std::replace(begin(name), end(name), '_', '.');
            auto e = TypeError(fmt::format("{0} is not a PROCEDURE", name), ast->get_location());
            errors.add(e);
            return;
        }

        procType = std::dynamic_pointer_cast<ProcedureType>(res->type);
        if (!procType) {
            auto e = TypeError(fmt::format("{0} is not a PROCEDURE", name), ast->get_location());
            errors.add(e);
            return;
        }

        debug("ASTCall found bound procedure {0}", name);
        ast->name->ident->accept(this); // type of only the identifier
        auto base_type = last_type;
        ast->name->ident->set_type(base_type);
        debug("ASTCall type {0}", string(base_type));
        if (!base_type->equiv(procType->receiver)) {
            auto e = TypeError(
                fmt::format("base type: {0} does not match bound procedure {1}, type: {2}",
                            string(base_type), name, string(procType->receiver)),
                ast->get_location());
            errors.add(e);
            return;
        }
    }

    // Check if procedure argument is a AnyType - then skip check argument types
    if (procType->params.empty() || procType->params[0].first != TypeTable::AnyType) {

        if (ast->args.size() != procType->params.size()) {
            std::replace(begin(name), end(name), '_', '.');
            auto e = TypeError(fmt::format("calling PROCEDURE {0}, incorrect number of "
                                           "arguments: {1} instead of {2}",
                                           name, ast->args.size(), procType->params.size()),
                               ast->get_location());
            errors.add(e);
            return;
        }
    } else {
        debug("ASTCall: {0} AnyType args", name);
        skip_argument_typecheck = true;
    }

    // Check argument types
    auto proc_iter = procType->params.begin();
    for (auto call_iter = ast->args.begin(); call_iter != ast->args.end();
         call_iter++, proc_iter++) {

        (*call_iter)->accept(this);

        if ((*proc_iter).second == Attr::var) {
            if (!is_lvalue || is_const) {
                debug("ASTCall is_lvalue: {0} is_const: {0}", is_lvalue, is_const);
                std::replace(begin(name), end(name), '_', '.');
                auto e = TypeError(fmt::format("procedure call {0} does not have a variable "
                                               "reference for VAR parameter {2}",
                                               name, last_type->get_name(),
                                               (*proc_iter).first->get_name()),
                                   ast->get_location());
                errors.add(e);
            } else if (is_lvalue) {
                auto var_name = std::string(*(*call_iter));
                auto res = symboltable.find(var_name);
                if (res) {
                    res->set(Attr::modified);
                }
            }
        }

        if (skip_argument_typecheck) {
            continue;
        }
        auto base_last = types.resolve(last_type->get_name());
        auto proc_base = types.resolve((*proc_iter).first->get_name());

        if (proc_base->id == TypeId::null || base_last->id == TypeId::any) {
            // Void type - accepts any type, used for builtin compile time functions
            continue;
        }

        debug("check parameter {0}: {1} with {2}", name, base_last->get_name(),
              proc_base->get_name());
        if (!proc_base->equiv(base_last)) {
            std::replace(begin(name), end(name), '_', '.');
            debug("incorrect parameter");
            auto e =
                TypeError(fmt::format("procedure call {0} has incorrect "
                                      "type {1} for parameter {2}",
                                      name, last_type->get_name(), (*proc_iter).first->get_name()),
                          ast->get_location());
            errors.add(e);
        }
    }

    // OK
    res->set(Attr::used);
    last_type = procType->ret;
    ast->set_type(last_type);
}

void Inspector::visit_ASTIf(ASTIf ast) {
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

void Inspector::visit_ASTCaseElement(ASTCaseElement ast) {
    std::for_each(std::begin(ast->stats), end(ast->stats), [this](auto &s) { s->accept(this); });
}

void Inspector::visit_ASTCase(ASTCase ast) {

    ast->expr->accept(this);
    if (!last_type->equiv(TypeTable::IntType) && !last_type->equiv(TypeTable::CharType)) {
        auto ex =
            TypeError("CASE expression has to be INTEGER or CHAR", ast->expr->get_location());
        errors.add(ex);
    }
    TypePtr case_type = last_type;

    // elements
    for (auto &e : ast->elements) {
        for (auto expr : e->exprs) {
            if (std::holds_alternative<ASTSimpleExpr>(expr)) {
                auto casexpr = std::get<ASTSimpleExpr>(expr);
                casexpr->accept(this);
                if (!last_type->equiv(case_type)) {
                    auto ex = TypeError(fmt::format("CASE expression mismatch type {0} does not "
                                                    "match CASE expression type {1}",
                                                    string(last_type), string(case_type)),
                                        casexpr->get_location());
                    errors.add(ex);
                }
            } else if (std::holds_alternative<ASTRange>(expr)) {
                auto range = std::get<ASTRange>(expr);
                range->first->accept(this);
                if (!last_type->equiv(case_type)) {
                    auto ex = TypeError(
                        fmt::format("CASE expression range mismatch first type {0} does not "
                                    "match CASE expression type {1}",
                                    string(last_type), string(case_type)),
                        range->get_location());
                    errors.add(ex);
                }
                range->last->accept(this);
                if (!last_type->equiv(case_type)) {
                    auto ex = TypeError(
                        fmt::format("CASE expression range mismatch last type {0} does not "
                                    "match CASE expression type {1}",
                                    string(last_type), string(case_type)),
                        range->get_location());
                    errors.add(ex);
                }
            }
        }
        e->accept(this);
    }

    // else
    std::for_each(begin(ast->else_stats), end(ast->else_stats),
                  [this](auto &s) { s->accept(this); });
}

void Inspector::visit_ASTFor(ASTFor ast) {
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
        ast->by->accept(this);
        if (!last_type->is_numeric()) {
            auto e = TypeError("FOR BY expression must be numeric type", ast->get_location());
            errors.add(e);
        }
    }

    auto res = symboltable.find(ast->ident->value);
    if (!res) {
        auto e = TypeError(fmt::format("FOR index variable {0} not defined", ast->ident->value),
                           ast->get_location());
        errors.add(e);
    } else {
        auto resType = types.resolve(res->type->get_name());
        if (!resType || !ast->start->get_type()->equiv(resType)) {
            auto e = TypeError(fmt::format("FOR index variable {0} wrong type", ast->ident->value),
                               ast->get_location());
            errors.add(e);
        }
    }

    std::for_each(begin(ast->stats), end(ast->stats), [this](auto const &s) { s->accept(this); });
}

void Inspector::visit_ASTWhile(ASTWhile ast) {
    ast->expr->accept(this);
    if (last_type != TypeTable::BoolType) {
        auto e = TypeError("WHILE expression must be type BOOLEAN", ast->get_location());
        errors.add(e);
    }

    std::for_each(begin(ast->stats), end(ast->stats), [this](auto const &x) { x->accept(this); });
}

void Inspector::visit_ASTRepeat(ASTRepeat ast) {
    std::for_each(begin(ast->stats), end(ast->stats), [this](auto const &x) { x->accept(this); });
    ast->expr->accept(this);
    if (last_type != TypeTable::BoolType) {
        auto e = TypeError("REPEAT expression must be type BOOLEAN", ast->get_location());
        errors.add(e);
    }
}

void Inspector::visit_ASTLoop(ASTLoop ast) {
    std::for_each(begin(ast->stats), end(ast->stats), [this](auto const &x) { x->accept(this); });
}

void Inspector::visit_ASTBlock(ASTBlock ast) {
    std::for_each(begin(ast->stats), end(ast->stats), [this](auto const &x) { x->accept(this); });
}

void Inspector::visit_ASTExpr(ASTExpr ast) {
    ast->expr->accept(this);
    auto c1 = is_const;
    if (ast->relation) {
        is_lvalue = false;
        auto t1 = last_type;
        ast->relation_expr->accept(this);
        auto result_type = types.check(*ast->relation, t1, last_type);
        if (!result_type) {
            auto e = TypeError(fmt::format("operator {0} doesn't takes types {1} and {2}",
                                           string(*ast->relation), string(t1), string(last_type)),
                               ast->get_location());
            errors.add(e);
            return;
        }
        last_type = result_type;
        c1 = c1 && is_const; // if both are const
        is_const = c1;       // return the const value
    }
    ast->set_type(last_type);
}

void Inspector::visit_ASTSimpleExpr(ASTSimpleExpr ast) {
    ast->term->accept(this);
    auto t1 = last_type;
    auto c1 = is_const;
    for (auto const &t : ast->rest) {
        is_lvalue = false;
        t.second->accept(this);
        auto result_type = types.check(t.first, t1, last_type);
        if (!result_type) {
            auto e = TypeError(fmt::format("operator {0} doesn't takes types {1} and {2}",
                                           string(t.first), string(t1), string(last_type)),
                               ast->get_location());
            errors.add(e);
            return;
        }
        t1 = result_type;
        last_type = t1;
        c1 = c1 && is_const; // if both are const
        is_const = c1;       // return the const value
    };
    ast->set_type(last_type);
}

void Inspector::visit_ASTTerm(ASTTerm ast) {
    ast->factor->accept(this);
    auto t1 = last_type;
    auto c1 = is_const;
    for (auto const &t : ast->rest) {
        is_lvalue = false;
        t.second->accept(this);
        auto result_type = types.check(t.first, t1, last_type);
        if (!result_type) {
            auto e = TypeError(fmt::format("operator {0} doesn't takes types {1} and {2}",
                                           string(t.first), string(t1), string(last_type)),
                               ast->get_location());
            errors.add(e);
            return;
        }

        c1 = c1 && is_const; // if both are const
        is_const = c1;       // return the const value

        t1 = result_type;
        last_type = t1;
    };
    ast->set_type(last_type);
}

void Inspector::visit_ASTFactor(ASTFactor ast) {
    std::visit(overloaded{
                   [this, ast](auto factor) {
                       factor->accept(this);
                       ast->set_type(last_type);
                   },
                   [this, ast](ASTCall const &factor) {
                       // need to pass on return type */
                       factor->accept(this);
                       is_lvalue = false;
                       ast->set_type(last_type);
                   },
                   [this, ast](ASTFactor const &arg) {
                       if (ast->is_not) {
                           arg->accept(this);
                           auto result_type = types.check(TokenType::tilde, last_type);
                           if (!result_type) {
                               auto e = TypeError("type in ~ expression must be BOOLEAN",
                                                  ast->get_location());
                               errors.add(e);
                               return;
                           }
                           last_type = result_type;
                           ast->set_type(last_type);
                           is_lvalue = false;
                       }
                   },
               },
               ast->factor);
}

void Inspector::visit_ASTDesignator(ASTDesignator ast) {
    debug("ASTDesignator");
    ast->ident->accept(this);

    // check type array before processing selectors
    if (!last_type) {
        // null - error in identifier
        return;
    }
    bool is_array = last_type->is_array();
    bool is_record = last_type->id == TypeId::record;
    bool is_string = last_type->id == TypeId::string;
    bool is_pointer = last_type->id == TypeId::pointer;
    auto b_type = last_type;
    if (!(is_array || is_record || is_string || is_pointer) && !ast->selectors.empty()) {
        auto e =
            TypeError(fmt::format("variable {0} is not an indexable type", ast->ident->id->value),
                      ast->get_location());
        errors.add(e);
        return;
    }

    // Not array or record type - no more checks.
    if (!(is_array || is_record || is_string || is_pointer)) {
        is_lvalue = true;
        return;
    }

    for (auto &ss : ast->selectors) {

        // can't do a std::visit as need to break out this loop
        if (std::holds_alternative<ArrayRef>(ss)) {

            // do ARRAY indexes
            debug("ASTDesignator array index");
            auto s = std::get<ArrayRef>(ss);
            if (!(is_array || is_string)) {
                auto e = TypeError("value not indexable type", ast->get_location());
                errors.add(e);
                return;
            }

            std::for_each(begin(s), end(s), [this, ast](auto &e) {
                e->accept(this);
                if (!last_type->is_numeric()) {
                    auto e = TypeError("expression in array index must be numeric",
                                       ast->get_location());
                    errors.add(e);
                }
            });

            if (is_array) {
                auto array_type = std::dynamic_pointer_cast<ArrayType>(b_type);

                // check index count, zero array dimensions means open array
                if (!array_type->dimensions.empty() && array_type->dimensions.size() != s.size()) {
                    auto e =
                        TypeError(fmt::format("array indexes don't match array dimensions of {0}",
                                              ast->ident->id->value),
                                  ast->get_location());
                    errors.add(e);
                }

                last_type = array_type->base_type;
                ast->set_type(last_type);
            } else if (is_string) {
                last_type = TypeTable::CharType;
                ast->set_type(last_type);
            }
        } else if (std::holds_alternative<FieldRef>(ss)) {

            // do RECORD type
            debug("ASTDesignator record field");
            auto &s = std::get<FieldRef>(ss);
            if (!is_record) {
                auto e = TypeError(fmt::format("value not RECORD: {0}", last_type->get_name()),
                                   s.first->get_location());
                errors.add(e);
                return;
            }

            auto record_type = std::dynamic_pointer_cast<RecordType>(b_type);
            auto field = record_type->get_type(s.first->value);
            if (!field) {
                auto e = TypeError(fmt::format("no field <{0}> in RECORD", s.first->value),
                                   ast->get_location());
                errors.add(e);
                return;
            }

            // Store field index with identifier, for use later in code
            // generator.

            s.second = record_type->get_index(s.first->value);
            debug("ASTDesignator record index {0} for {1}", s.second, s.first->value);

            last_type = *field;
            ast->set_type(last_type);
        } else if (std::holds_alternative<PointerRef>(ss)) {
            // Reference
            if (!is_pointer) {
                auto e = TypeError(
                    fmt::format("variable {0} is not a pointer ", std::string(*ast->ident)),
                    ast->get_location());
                errors.add(e);
                return;
            }
            auto ptr_type = std::dynamic_pointer_cast<PointerType>(b_type);
            debug("ASTDesignator ptr {0} ref: {1}", ptr_type->get_name(),
                  ptr_type->get_reference()->get_name());
            last_type = ptr_type->get_reference();
            debug("ASTDesignator {0} ", last_type->get_name());
            ast->set_type(last_type);
        }
        b_type = types.resolve(last_type->get_name());
        is_array = b_type->is_array();
        is_record = b_type->id == TypeId::record;
        is_string = b_type->id == TypeId::string;
    }
} // namespace ax

void Inspector::visit_ASTType(ASTType ast) {
    std::visit(overloaded{[this, ast](ASTQualident const &type) {
                              debug("ASTType {0}", type->id->value);
                              auto result = types.find(type->id->value);
                              if (!result) {
                                  throw TypeError(
                                      fmt::format("Unknown type: {0}", type->id->value),
                                      ast->get_location());
                              }
                              debug("ASTType 2 {0}", type->id->value);
                              ast->set_type(result);
                              last_type = result;
                          },
                          [this, ast](ASTArray const &arg) {
                              arg->accept(this);
                              ast->set_type(last_type);
                          },
                          [this, ast](ASTRecord const &arg) {
                              arg->accept(this);
                              ast->set_type(last_type);
                          },
                          [this, ast](ASTPointerType const &arg) {
                              arg->accept(this);
                              ast->set_type(last_type);
                          }},
               ast->type);
}

void Inspector::visit_ASTArray(ASTArray ast) {
    int i = 0;
    for (auto &expr : ast->dimensions) {
        expr->accept(this);
        if (!is_const) {
            auto e =
                TypeError(fmt::format("ARRAY expecting constant expression for dimension {0}", i),
                          ast->get_location());
            errors.add(e);
        }
        if (!last_type->is_numeric()) {
            auto e = TypeError(fmt::format("ARRAY expecting numeric size for dimension {0}", i),
                               ast->get_location());
            errors.add(e);
        }
        i++;
    }

    ast->type->accept(this);

    TypePtr array_type{nullptr};
    if (ast->dimensions.empty()) {
        array_type = std::make_shared<ax::OpenArrayType>(last_type);
    } else {
        auto at = std::make_shared<ax::ArrayType>(last_type);
        std::for_each(begin(ast->dimensions), end(ast->dimensions),
                      [&at](auto &d) { at->dimensions.push_back(d->value); });
        array_type = at;
    }
    last_type = array_type;
    ast->set_type(last_type);
    types.put(last_type->get_name(), last_type);
}

void Inspector::visit_ASTRecord(ASTRecord ast) {
    debug("ASTRecord");

    auto rec_type = std::make_shared<ax::RecordType>();
    if (ast->base) {
        ast->base->accept(this);
        auto baseType_name = std::string(*ast->base);
        debug("ASTRecord base {0}", baseType_name);
        auto baseType = types.resolve(baseType_name);
        if (!baseType) {
            auto e = TypeError(fmt::format("RECORD base type {0} not found", baseType_name),
                               ast->base->get_location());
            errors.add(e);
        } else if (baseType->id != TypeId::record) {
            auto e = TypeError(fmt::format("RECORD base type {0} is not a record", baseType_name),
                               ast->base->get_location());
            errors.add(e);
        } else {
            auto base_rec = std::dynamic_pointer_cast<RecordType>(baseType);
            rec_type->set_baseType(base_rec);
        }
    }

    std::for_each(begin(ast->fields), end(ast->fields), [this, rec_type, ast](auto const &v) {
        // check types
        v.second->accept(this);

        // check if not already defined
        if (rec_type->has_field(v.first->value)) {
            auto e = TypeError(fmt::format("RECORD already has field {0}", v.first->value),
                               v.first->get_location());
            errors.add(e);
        } else {
            rec_type->insert(v.first->value, last_type);
        }
    });
    last_type = rec_type;
    ast->set_type(last_type);
    types.put(last_type->get_name(), last_type);
}

void Inspector::visit_ASTPointerType(ASTPointerType ast) {
    auto ref_name = std::string(*ast->reference);
    debug("ASTPointerType {0}", ref_name);
    std::shared_ptr<ax::PointerType> ptr_type;
    try {
        ast->reference->accept(this);
        ptr_type = std::make_shared<ax::PointerType>(last_type);
    } catch (TypeError const &) {
        // Not found
        ptr_type = std::make_shared<ax::PointerType>(ref_name);
    }
    ast->set_type(ptr_type);
    last_type = ptr_type;
    // Put into type table
    types.put(last_type->get_name(), ptr_type);
    pointer_types.push_back(ptr_type);
}

std::string Inspector::get_Qualident(ASTQualident const &ast) {
    std::string result;
    auto        res = symboltable.find(ast->qual);

    if (ast->qual.empty()) {
        return ast->id->value;
    }
    if (res && res->type->id == TypeId::module) {
        auto module_name = std::dynamic_pointer_cast<ModuleType>(res->type)->module_name();
        result = ASTQualident_::make_coded_id(module_name, ast->id->value);
        // Rewrite AST with real module name
        ast->qual = module_name;
    }
    return result;
}

void Inspector::visit_ASTQualident(ASTQualident ast) {
    debug("ASTQualident");
    if (ast->qual.empty()) {
        visit_ASTIdentifier(ast->id);
        ast->set_type(last_type);
    } else {
        debug("ASTQualident {0}", ast->qual);

        auto new_ast = make<ASTIdentifier_>();
        new_ast->value = get_Qualident(ast);
        is_qualid = true;
        qualid_error = false;
        visit_ASTIdentifier(new_ast);
        if (qualid_error) {
            auto e = CodeGenException(
                fmt::format("undefined identifier {0} in MODULE {1}", ast->id->value, ast->qual),
                ast->get_location());
            errors.add(e);
        }
        ast->set_type(last_type);
    }
}

void Inspector::visit_ASTIdentifier(ASTIdentifier ast) {
    debug("ASTIdentifier");
    auto res = symboltable.find(ast->value);
    if (!res) {
        if (!is_qualid) {

            // Check is type name and accept, can only then be passed to objects of
            // type VOID
            auto typep = types.find(ast->value);
            if (typep) {
                debug("type: {0}", ast->value);
                return;
            }

            auto e = CodeGenException(fmt::format("undefined identifier {0}", ast->value),
                                      ast->get_location());
            errors.add(e);
            return;
        } else {
            qualid_error = true;
            return;
        }
    }
    debug("find type: {0} for {1}", res->type->get_name(), ast->value);
    auto resType = types.resolve(res->type->get_name());
    if (!resType) {
        auto e = TypeError(
            fmt::format("Unknown type: {0} for identifier {1}", res->type->get_name(), ast->value),
            ast->get_location());
        errors.add(e);
        return;
    }
    last_type = resType;
    ast->set_type(last_type);
    is_const = res->is(Attr::cnst);
    if (last_type->id == TypeId::string) {
        ast->set(Attr::ptr);
    }
    is_lvalue = true;
}

// Constant literals

void Inspector::visit_ASTSet(ASTSet ast) {
    bool set_const = true;
    std::for_each(cbegin(ast->values), cend(ast->values), [this, &set_const, ast](auto &exp) {
        std::visit(
            overloaded{
                [this, &set_const, ast](ASTSimpleExpr const &exp) {
                    debug("ASTSet exp");
                    exp->accept(this);
                    if (!TypeTable::IntType->equiv(last_type)) {
                        auto e = TypeError(
                            fmt::format("Expression {0} is not a integer type", std::string(*exp)),
                            exp->get_location());
                        errors.add(e);
                    }
                    set_const &= is_const;
                },
                [this, &set_const, ast](ASTRange const &exp) {
                    debug("ASTSet range");
                    exp->first->accept(this);
                    if (!TypeTable::IntType->equiv(last_type)) {
                        auto e = TypeError(fmt::format("Expression {0} is not a integer type",
                                                       std::string(*exp->first)),
                                           exp->first->get_location());
                        errors.add(e);
                    }
                    set_const &= is_const;
                    exp->last->accept(this);
                    if (!TypeTable::IntType->equiv(last_type)) {
                        auto e = TypeError(fmt::format("Expression {0} is not a integer type",
                                                       std::string(*exp->last)),
                                           exp->last->get_location());
                        errors.add(e);
                    }
                    set_const &= is_const;
                }},
            exp);
    });
    is_const = set_const;
    last_type = TypeTable::SetType;
    is_lvalue = false;
}

void Inspector::visit_ASTInteger(ASTInteger /* not used */) {
    last_type = TypeTable::IntType;
    is_const = true;
    is_lvalue = false;
}

void Inspector::visit_ASTReal(ASTReal /* not used */) {
    last_type = TypeTable::RealType;
    is_const = true;
    is_lvalue = false;
}

void Inspector::visit_ASTChar(ASTCharPtr /* not used */) {
    last_type = TypeTable::CharType;
    is_const = true;
    is_lvalue = false;
}

void Inspector::visit_ASTString(ASTString ast) {
    if (ast->value.size() == 1) {
        last_type = TypeTable::Str1Type;
    } else {
        last_type = TypeTable::StrType;
    }
    is_const = true;
    is_lvalue = false;
};

void Inspector::visit_ASTBool(ASTBool /* not used */) {
    last_type = TypeTable::BoolType;
    is_const = true;
    is_lvalue = false;
}

void Inspector::visit_ASTNil(ASTNil /*not used*/) {
    debug("ASTNil");
    last_type = TypeTable::VoidType; // NIL can be assigned to any pointer
    is_const = true;
    is_lvalue = false;
}

void Inspector::check(ASTModule const &ast) {
    ast->accept(this);
}

} // namespace ax