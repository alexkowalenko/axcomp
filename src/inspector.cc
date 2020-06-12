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

#include <llvm/Support/Debug.h>
#include <llvm/Support/FormatVariadic.h>

#include "ast.hh"
#include "astvisitor.hh"
#include "error.hh"
#include "symbol.hh"
#include "token.hh"
#include "type.hh"
#include "typetable.hh"

namespace ax {

#define DEBUG_TYPE "inspector"

template <typename... T> static void debug(const T &... msg) {
    LLVM_DEBUG(llvm::dbgs() << llvm::formatv(msg...) << '\n');
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
        c.type->set_type(last_type);
        c.type->type = make<ASTQualident>(last_type->get_name());
        debug("Inspector::visit_ASTConst type: {0}", last_type->get_name());
        symboltable.put(c.ident->value, mkSym(last_type, Attr::cnst));
    });
}

void Inspector::visit_ASTTypeDec(ASTTypeDecPtr ast) {
    debug("Inspector::visit_ASTTypeDec");
    for (auto const &t : ast->types) {
        if (types.find(t.first->value)) {
            auto e = TypeError(llvm::formatv("TYPE {0} already defined", t.first->value),
                               ast->get_location());
            errors.add(e);
            continue;
        }
        if (t.first->is(Attr::read_only)) {
            auto e = TypeError(llvm::formatv("TYPE {0} is always read only", t.first->value),
                               ast->get_location());
            errors.add(e);
            continue;
        }
        t.second->accept(this);
        if (last_type->id == TypeId::record) {
            std::dynamic_pointer_cast<RecordType>(last_type)->set_identified(t.first->value);
        }
        auto type = std::make_shared<TypeAlias>(t.first->value, last_type);
        debug("Inspector::visit_ASTTypeDec put type {0}", t.first->value);
        types.put(t.first->value, type);
    };
}

void Inspector::visit_ASTVar(ASTVarPtr ast) {
    debug("Inspector::visit_ASTVar");

    std::for_each(ast->vars.begin(), ast->vars.end(), [this](auto const &v) {
        // No need to check the identifier - its being
        // defined.
        v.second->accept(this);
        // Update VAR declaration symbols with type
        debug("var type: {0}", last_type->get_name());
        symboltable.put(v.first->value, mkSym(last_type));
    });

    // Post process all pointer defintions
    for (auto &ptr_type : pointer_types) {
        if (!ptr_type->get_reference()) {
            debug("Inspector::visit_ASTVar resolve pointer type: {0}", ptr_type->get_ref_name());
            auto ref = types.resolve(ptr_type->get_ref_name());
            if (!ref) {
                auto e = TypeError(llvm::formatv("TYPE {0} not found", ptr_type->get_ref_name()),
                                   ast->get_location());
                errors.add(e);
            }
            ptr_type->set_reference(ref);
        }
    };
    pointer_types.clear();
}

std::pair<TypePtr, ProcedureType::ParamsList> Inspector::do_proc(ASTProc &ast) {
    // Check name if not defined;
    if (auto name_check = symboltable.find(ast.name->value);
        name_check && name_check->type->id != TypeId::procedureFwd) {
        auto e = TypeError(
            llvm::formatv("PROCEDURE {0}, identifier is already defined", ast.name->value),
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
        auto e = TypeError(llvm::formatv("PROCEDURE {0} is always read only", ast.name->value),
                           ast.get_location());
        errors.add(e);
    }

    // Check parameter types
    debug("Inspector::visit_ASTProcedure check parameter types");
    ProcedureType::ParamsList argTypes;
    std::for_each(ast.params.begin(), ast.params.end(), [this, &argTypes](auto const &p) {
        p.second->accept(this); // type
        auto attr = Attr::null;
        if (p.first->is(Attr::var)) {
            attr = Attr::var;
        }
        argTypes.push_back({last_type, attr});
    });
    return {retType, argTypes};
}

void Inspector::visit_ASTProcedure(ASTProcedurePtr ast) {
    debug("Inspector::visit_ASTProcedure: {0}", ast->name->value);

    auto [retType, argTypes] = do_proc(*ast);

    auto proc_type = std::make_shared<ProcedureType>(retType, argTypes);
    symboltable.put(ast->name->value, mkSym(proc_type));

    last_proc = ast;

    // new symbol table
    symboltable.push_frame(ast->name->value);
    int count = 0;
    for (auto const &p : ast->params) {
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
        symboltable.put(p.first->value, mkSym(argTypes[count].first,
                                              p.first->is(Attr::var) ? Attr::var : Attr::null));
        count++;
    };
    if (ast->decs) {
        ast->decs->accept(this);
    }

    // check local procedures
    std::for_each(cbegin(ast->procedures), cend(ast->procedures),
                  [this](auto const &proc) { proc->accept(this); });

    // check statements
    std::for_each(cbegin(ast->stats), cend(ast->stats),
                  [this, ast](auto const &x) { x->accept(this); });
    symboltable.pop_frame();
}

void Inspector::visit_ASTProcedureForward(ASTProcedureForwardPtr ast) {
    debug("Inspector::visit_ASTProcedureForward: {0}", ast->name->value);
    auto [retType, argTypes] = do_proc(*ast);

    auto proc_type = std::make_shared<ProcedureFwdType>();
    proc_type->ret = retType;
    proc_type->params = argTypes;
    symboltable.put(ast->name->value, mkSym(proc_type));
};

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
    if (!last_type) {
        // error return;
        return;
    }
    auto alias = types.resolve(last_type->get_name());
    assert(alias);
    debug("type of ident: {0} -> {1}", last_type->get_name(), alias->get_name());
    last_type = alias;
    if (!(types.check(TokenType::assign, last_type, expr_type) || last_type->equiv(expr_type))) {
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
                           [this, &retType](ASTQualidentPtr const &type) {
                               if (auto t = types.resolve(type->id->value); t) {
                                   retType = t;
                               };
                           },
                       },
                       type->type);
        }

        if (!expr_type->equiv(retType)) {
            auto e = TypeError(
                llvm::formatv("RETURN type ({1}) does not match return type for function {0}: {2}",
                              last_proc->name->value, retType->get_name(), expr_type->get_name()),
                ast->get_location());
            errors.add(e);
        }
    }
}

void Inspector::visit_ASTCall(ASTCallPtr ast) {
    debug("Inspector::visit_ASTCall");
    // auto name = ast->name->ident->make_coded_id();
    auto name = get_Qualident(ast->name->ident);
    bool skip_argument_typecheck{false};

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

    // Check if procedure argument is a AnyType - then skip check argument types
    if (procType->params.empty() || procType->params[0].first != TypeTable::AnyType) {

        if (ast->args.size() != procType->params.size()) {
            std::replace(begin(name), end(name), '_', '.');
            throw TypeError(llvm::formatv("calling PROCEDURE {0}, incorrect number of "
                                          "arguments: {1} instead of {2}",
                                          name, ast->args.size(), procType->params.size()),
                            ast->get_location());
        }
    } else {
        debug("Inspector::visit_ASTCall: {0} AnyType args", name);
        skip_argument_typecheck = true;
    }

    // Check argument types
    auto proc_iter = procType->params.begin();
    for (auto call_iter = ast->args.begin(); call_iter != ast->args.end();
         call_iter++, proc_iter++) {

        (*call_iter)->accept(this);

        if ((*proc_iter).second == Attr::var && (!is_lvalue || is_const)) {
            debug("Inspector::visit_ASTCall is_lvalue");
            std::replace(begin(name), end(name), '_', '.');
            auto e = TypeError(llvm::formatv("procedure call {0} does not have a variable "
                                             "reference for VAR parameter {2}",
                                             name, last_type->get_name(),
                                             (*proc_iter).first->get_name()),
                               ast->get_location());
            errors.add(e);
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
            auto e = TypeError(llvm::formatv("procedure call {0} has incorrect "
                                             "type {1} for parameter {2}",
                                             name, last_type->get_name(),
                                             (*proc_iter).first->get_name()),
                               ast->get_location());
            errors.add(e);
        }
    }

    // OK
    res->set(Attr::used);
    last_type = procType->ret;
    ast->set_type(last_type);
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

void Inspector::visit_ASTCaseElement(ASTCaseElementPtr ast) {
    std::for_each(std::begin(ast->stats), end(ast->stats), [this](auto &s) { s->accept(this); });
}

void Inspector::visit_ASTCase(ASTCasePtr ast) {

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
            if (std::holds_alternative<ASTSimpleExprPtr>(expr)) {
                auto casexpr = std::get<ASTSimpleExprPtr>(expr);
                casexpr->accept(this);
                if (!last_type->equiv(case_type)) {
                    auto ex =
                        TypeError(llvm::formatv("CASE expression mismatch type {0} does not "
                                                "match CASE expression type {1}",
                                                std::string(*last_type), std::string(*case_type)),
                                  casexpr->get_location());
                    errors.add(ex);
                }
            } else if (std::holds_alternative<ASTRangePtr>(expr)) {
                auto range = std::get<ASTRangePtr>(expr);
                range->first->accept(this);
                if (!last_type->equiv(case_type)) {
                    auto ex = TypeError(
                        llvm::formatv("CASE expression range mismatch first type {0} does not "
                                      "match CASE expression type {1}",
                                      std::string(*last_type), std::string(*case_type)),
                        range->get_location());
                    errors.add(ex);
                }
                range->last->accept(this);
                if (!last_type->equiv(case_type)) {
                    auto ex = TypeError(
                        llvm::formatv("CASE expression range mismatch last type {0} does not "
                                      "match CASE expression type {1}",
                                      std::string(*last_type), std::string(*case_type)),
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
        ast->by->accept(this);
        if (!last_type->is_numeric()) {
            auto e = TypeError("FOR BY expression must be numeric type", ast->get_location());
            errors.add(e);
        }
    }

    auto res = symboltable.find(ast->ident->value);
    if (!res) {
        auto e = TypeError(llvm::formatv("FOR index variable {0} not defined", ast->ident->value),
                           ast->get_location());
        errors.add(e);
    } else {
        auto resType = types.resolve(res->type->get_name());
        if (!resType || !ast->start->get_type()->equiv(resType)) {
            auto e =
                TypeError(llvm::formatv("FOR index variable {0} wrong type", ast->ident->value),
                          ast->get_location());
            errors.add(e);
        }
    }

    std::for_each(begin(ast->stats), end(ast->stats), [this](auto const &s) { s->accept(this); });
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
        ast->relation_expr->accept(this);
        auto result_type = types.check(*ast->relation, t1, last_type);
        if (!result_type) {
            auto e = TypeError(llvm::formatv("operator {0} doesn't takes types {1} and {2}",
                                             string(*ast->relation), std::string(*t1),
                                             std::string(*last_type)),
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

void Inspector::visit_ASTSimpleExpr(ASTSimpleExprPtr ast) {
    ast->term->accept(this);
    auto t1 = last_type;
    auto c1 = is_const;
    for (auto const &t : ast->rest) {
        is_lvalue = false;
        t.second->accept(this);
        auto result_type = types.check(t.first, t1, last_type);
        if (!result_type) {
            auto e = TypeError(llvm::formatv("operator {0} doesn't takes types {1} and {2}",
                                             string(t.first), std::string(*t1),
                                             std::string(*last_type)),
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

void Inspector::visit_ASTTerm(ASTTermPtr ast) {
    ast->factor->accept(this);
    auto t1 = last_type;
    auto c1 = is_const;
    for (auto const &t : ast->rest) {
        is_lvalue = false;
        t.second->accept(this);
        auto result_type = types.check(t.first, t1, last_type);
        if (!result_type) {
            auto e = TypeError(llvm::formatv("operator {0} doesn't takes types {1} and {2}",
                                             string(t.first), std::string(*t1),
                                             std::string(*last_type)),
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

void Inspector::visit_ASTFactor(ASTFactorPtr ast) {
    std::visit(overloaded{
                   [this, ast](auto factor) {
                       factor->accept(this);
                       ast->set_type(last_type);
                   },
                   [this, ast](ASTCallPtr const &factor) {
                       // need to pass on return type */
                       factor->accept(this);
                       is_lvalue = false;
                       ast->set_type(last_type);
                   },
                   [this, ast](ASTFactorPtr const &arg) {
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

void Inspector::visit_ASTDesignator(ASTDesignatorPtr ast) {
    debug("Inspector::visit_ASTDesignator");
    ast->ident->accept(this);

    // check type array before processing selectors
    if (!last_type) {
        // null - error in identifier
        return;
    }
    bool is_array = last_type->id == TypeId::array;
    bool is_record = last_type->id == TypeId::record;
    bool is_string = last_type->id == TypeId::string;
    bool is_pointer = last_type->id == TypeId::pointer;
    auto b_type = last_type;
    if (!(is_array || is_record || is_string || is_pointer) && !ast->selectors.empty()) {
        auto e = TypeError(
            llvm::formatv("variable {0} is not an indexable type", ast->ident->id->value),
            ast->get_location());
        errors.add(e);
        return;
    }

    // Not array or record type - no more checks.
    if (!(is_array || is_record || is_string || is_pointer)) {
        is_lvalue = true;
        return;
    }

    if (is_array || is_record) {
        is_lvalue = false; // no lvalues for aggregate values
    } else {
        is_lvalue = true; // STRINGs can be passed
    }
    for (auto &ss : ast->selectors) {

        // can't do a std::visit as need to break out this loop
        if (std::holds_alternative<ArrayRef>(ss)) {

            // do ARRAY indexes
            debug("Inspector::visit_ASTDesignator array index");
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
                    auto e = TypeError(
                        llvm::formatv("array indexes don't match array dimensions of {0}",
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
            debug("Inspector::visit_ASTDesignator record field");
            auto &s = std::get<FieldRef>(ss);
            if (!is_record) {
                auto e = TypeError(llvm::formatv("value not RECORD: {0}", last_type->get_name()),
                                   s.first->get_location());
                errors.add(e);
                return;
            }

            auto record_type = std::dynamic_pointer_cast<RecordType>(b_type);
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
            ast->set_type(last_type);
        } else if (std::holds_alternative<PointerRef>(ss)) {
            // Reference
            if (!is_pointer) {
                auto e = TypeError(
                    llvm::formatv("variable {0} is not a pointer ", std::string(*ast->ident)),
                    ast->get_location());
                errors.add(e);
                return;
            }
            auto ptr_type = std::dynamic_pointer_cast<PointerType>(b_type);
            debug("Inspector::visit_ASTDesignator ptr {0} ref: {1}", ptr_type->get_name(),
                  ptr_type->get_reference()->get_name());
            last_type = ptr_type->get_reference();
            debug("Inspector::visit_ASTDesignator {0} ", last_type->get_name());
            ast->set_type(last_type);
        }
        b_type = types.resolve(last_type->get_name());
        is_array = b_type->id == TypeId::array;
        is_record = b_type->id == TypeId::record;
        is_string = b_type->id == TypeId::string;
    }
} // namespace ax

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
                              ast->set_type(result);
                              last_type = result;
                          },
                          [this, ast](ASTArrayPtr const &arg) {
                              arg->accept(this);
                              ast->set_type(last_type);
                          },
                          [this, ast](ASTRecordPtr const &arg) {
                              arg->accept(this);
                              ast->set_type(last_type);
                          },
                          [this, ast](ASTPointerTypePtr const &arg) {
                              arg->accept(this);
                              ast->set_type(last_type);
                          }},
               ast->type);
}

void Inspector::visit_ASTArray(ASTArrayPtr ast) {
    int i = 0;
    for (auto &expr : ast->dimensions) {
        expr->accept(this);
        if (!is_const) {
            auto e = TypeError(
                llvm::formatv("ARRAY expecting constant expression for dimension {0}", i),
                ast->get_location());
            errors.add(e);
        }
        if (!last_type->is_numeric()) {
            auto e = TypeError(llvm::formatv("ARRAY expecting numeric size for dimension {0}", i),
                               ast->get_location());
            errors.add(e);
        }
    }

    ast->type->accept(this);

    auto array_type = std::make_shared<ax::ArrayType>(last_type);
    std::for_each(begin(ast->dimensions), end(ast->dimensions),
                  [&array_type](auto &d) { array_type->dimensions.push_back(d->value); });
    last_type = array_type;
    ast->set_type(last_type);
    types.put(last_type->get_name(), last_type);
}

void Inspector::visit_ASTRecord(ASTRecordPtr ast) {
    debug("Inspector::visit_ASTRecord");

    auto rec_type = std::make_shared<ax::RecordType>();
    if (ast->base) {
        ast->base->accept(this);
        auto baseType_name = std::string(*ast->base);
        debug("Inspector::visit_ASTRecord base {0}", baseType_name);
        auto baseType = types.resolve(baseType_name);
        if (!baseType) {
            auto e = TypeError(llvm::formatv("RECORD base type {0} not found", baseType_name),
                               ast->base->get_location());
            errors.add(e);
        } else if (baseType->id != TypeId::record) {
            auto e =
                TypeError(llvm::formatv("RECORD base type {0} is not a record", baseType_name),
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
            auto e = TypeError(llvm::formatv("RECORD already has field {0}", v.first->value),
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

void Inspector::visit_ASTPointerType(ASTPointerTypePtr ast) {
    auto ref_name = std::string(*ast->reference);
    debug("Inspector::visit_ASTPointerType {0}", ref_name);
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
        ast->set_type(last_type);
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
        ast->set_type(last_type);
    }
}

void Inspector::visit_ASTIdentifier(ASTIdentifierPtr ast) {
    debug("Inspector::visit_ASTIdentifier");
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

            throw CodeGenException(llvm::formatv("undefined identifier {0}", ast->value),
                                   ast->get_location());
        } else {
            qualid_error = true;
            return;
        }
    }
    debug("find type: {0} for {1}", res->type->get_name(), ast->value);
    auto resType = types.resolve(res->type->get_name());
    if (!resType) {
        auto e = TypeError(llvm::formatv("Unknown type: {0} for identifier {1}",
                                         res->type->get_name(), ast->value),
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

void Inspector::visit_ASTSet(ASTSetPtr ast) {
    bool set_const = true;
    std::for_each(cbegin(ast->values), cend(ast->values), [this, &set_const, ast](auto &exp) {
        std::visit(
            overloaded{
                [this, &set_const, ast](ASTSimpleExprPtr const &exp) {
                    debug("Inspector::visit_ASTSet exp");
                    exp->accept(this);
                    if (!TypeTable::IntType->equiv(last_type)) {
                        auto e = TypeError(llvm::formatv("Expression {0} is not a integer type",
                                                         std::string(*exp)),
                                           exp->get_location());
                        errors.add(e);
                    }
                    set_const &= is_const;
                },
                [this, &set_const, ast](ASTRangePtr const &exp) {
                    debug("Inspector::visit_ASTSet range");
                    exp->first->accept(this);
                    if (!TypeTable::IntType->equiv(last_type)) {
                        auto e = TypeError(llvm::formatv("Expression {0} is not a integer type",
                                                         std::string(*exp->first)),
                                           exp->first->get_location());
                        errors.add(e);
                    }
                    set_const &= is_const;
                    exp->last->accept(this);
                    if (!TypeTable::IntType->equiv(last_type)) {
                        auto e = TypeError(llvm::formatv("Expression {0} is not a integer type",
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

void Inspector::visit_ASTInteger(ASTIntegerPtr /* not used */) {
    last_type = TypeTable::IntType;
    is_const = true;
    is_lvalue = false;
}

void Inspector::visit_ASTReal(ASTRealPtr /* not used */) {
    last_type = TypeTable::RealType;
    is_const = true;
    is_lvalue = false;
}

void Inspector::visit_ASTChar(ASTCharPtr /* not used */) {
    last_type = TypeTable::CharType;
    is_const = true;
    is_lvalue = false;
}

void Inspector::visit_ASTString(ASTStringPtr ast) {
    if (ast->value.size() == 1) {
        last_type = TypeTable::Str1Type;
    } else {
        last_type = TypeTable::StrType;
    }
    is_const = true;
    is_lvalue = false;
};

void Inspector::visit_ASTBool(ASTBoolPtr /* not used */) {
    last_type = TypeTable::BoolType;
    is_const = true;
    is_lvalue = false;
}

void Inspector::visit_ASTNil(ASTNilPtr /*not used*/) {
    debug("Inspector::visit_ASTNil");
    last_type = TypeTable::VoidType; // NIL can be assigned to any pointer
    is_const = true;
    is_lvalue = false;
}

void Inspector::check(ASTModulePtr const &ast) {
    ast->accept(this);
}

} // namespace ax