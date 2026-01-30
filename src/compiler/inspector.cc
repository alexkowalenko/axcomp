//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "inspector.hh"

#include <algorithm>
#include <format>
#include <memory>

#include <llvm/Support/Debug.h>

#include "ast/all.hh"
#include "astvisitor.hh"
#include "error.hh"
#include "symbol.hh"
#include "token.hh"
#include "types/all.hh"
#include "typetable.hh"

#include <ranges>

namespace ax {

namespace {
constexpr auto closure_arg{"_closure"};

#define DEBUG_TYPE "inspector"
template <typename S, typename... Args> void debug(const S &format, const Args &...msg) {
    LLVM_DEBUG(llvm::dbgs() << DEBUG_TYPE << ' '
                            << std::vformat(format, std::make_format_args(msg...)) << '\n');
}

} // namespace

Inspector::Inspector(SymbolFrameTable &s, TypeTable &t, ErrorManager &e, Importer &i)
    : symboltable(s), types(t), errors(e), importer(i) {};

void Inspector::visit(ASTModule const &ast) {
    debug("ASTModule");
    if (ast->import) {
        visit(ast->import);
    }

    variable_type = Attr::global_var;
    visit(ast->decs);

    variable_type = Attr::local_var;
    for (auto const &proc : ast->procedures) {
        proc->accept(this);
    }

    last_proc = nullptr;

    for (auto const &stat : ast->stats) {
        stat->accept(this);
    }
    debug("ASTModule done");
}

void Inspector::visit(ASTDeclaration const &ast) {
    if (ast->type) {
        visit(ast->type);
    }
    if (ast->cnst) {
        visit(ast->cnst);
    }
    if (ast->var) {
        visit(ast->var);
    }
}

void Inspector::visit(ASTImport const &ast) {
    debug("ASTImport");
    for (const auto &name : ast->imports | std::views::keys) {
        if (!importer.find_module(name->value, symboltable, types)) {
            throw TypeError(name->get_location(), "MODULE {0} not found", name->value);
        }
    }
}

void Inspector::visit(ASTConst const &ast) {
    debug("ASTConst");
    for (auto &[ident, value, type] : ast->consts) {
        visit(value);
        if (!is_const) {
            auto e = TypeError(ast->get_location(), "CONST {0} is not a constant expression",
                               ident->value);
            errors.add(e);
        }
        if (ident->is(Attr::read_only)) {
            auto e = TypeError(ast->get_location(), "CONST {0} is always read only", ident->value);
            errors.add(e);
        }
        type = std::make_shared<ASTType_>();
        type->set_type(last_type);
        type->type = make<ASTQualident_>(last_type->get_name());
        debug("ASTConst type: {0}", last_type->get_name());
        auto sym = mkSym(last_type, Attr::cnst);
        sym->set(variable_type);
        symboltable.put(ident->value, sym);
    }
}

void Inspector::visit(ASTTypeDec const &ast) {
    debug("ASTTypeDec");
    for (auto const &[name, type_expr] : ast->types) {
        if (types.find(name->value)) {
            auto e = TypeError(ast->get_location(), "TYPE {0} already defined", name->value);
            errors.add(e);
            continue;
        }
        visit(type_expr);
        if (last_type && last_type->id == TypeId::ENUMERATION) {
            if (const auto enum_type = std::dynamic_pointer_cast<EnumType>(last_type)) {
                enum_type->name = name->value;
            }
        }
        if (last_type->id == TypeId::RECORD) {
            std::dynamic_pointer_cast<RecordType>(last_type)->set_identified(name->value);
        }
        auto type = std::make_shared<TypeAlias>(name->value, last_type);
        debug("ASTTypeDec put type {0}", name->value);
        types.put(name->value, type);
    };
}

void Inspector::visit(ASTVar const &ast) {
    debug("ASTVar");

    for (auto const &[name, expr] : ast->vars) {
        visit(expr);
        // Update VAR declaration symbols with type
        debug("var type: {0}", last_type->get_name());
        symboltable.put(name->value, mkSym(last_type, variable_type));
    }

    // Post process all pointer definitions
    for (const auto &ptr_type : pointer_types) {
        if (!ptr_type->get_reference()) {
            debug("ASTVar resolve pointer type: {0}", ptr_type->get_ref_name());
            auto ref = types.resolve(ptr_type->get_ref_name());
            if (!ref) {
                auto e =
                    TypeError(ast->get_location(), "TYPE {0} not found", ptr_type->get_ref_name());
                errors.add(e);
            }
            ptr_type->set_reference(ref);
        }
    };
    pointer_types.clear();
}

void Inspector::do_receiver(const RecVar &r) const {
    if (!r.first) {
        return;
    }
    const auto t = types.resolve(r.second->value);
    if (!t) {
        const auto e =
            TypeError(r.second->get_location(),
                      "bound type {0} not found for type-bound PROCEDURE", r.second->value);
        errors.add(e);
        return;
    }
    if (t->id != TypeId::RECORD && !is_ptr_to_record(t)) {
        const auto e = TypeError(
            r.second->get_location(),
            "bound type {0} must be a RECORD or POINTER TO RECORD in type-bound PROCEDURE",
            r.second->value);
        errors.add(e);
        return;
    }
    r.second->set_type(t);
}

std::pair<Type, ProcedureType::ParamsList> Inspector::do_proc(const ASTProc_ &ast) {
    // Check name if not defined;
    if (const auto name_check = symboltable.find(ast.name->value);
        name_check && name_check->type->id != TypeId::PROCEDURE_FWD) {
        const auto e = TypeError(ast.get_location(),
                                 "PROCEDURE {0}, identifier is already defined", ast.name->value);
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
        const auto e =
            TypeError(ast.get_location(), "PROCEDURE {0} is always read only", ast.name->value);
        errors.add(e);
    }

    // Check receiver
    do_receiver(ast.receiver);

    // Check parameter types
    debug("ASTProcedure check parameter types");
    ProcedureType::ParamsList argTypes;
    for (const auto &[fst, snd] : ast.params) {
        visit(snd);
        auto attr = fst->is(Attr::var) ? Attr::var : Attr::null;
        argTypes.emplace_back(last_type, attr);
    }
    return {retType, argTypes};
}

void Inspector::visit(ASTProcedure const &ast) {
    debug("ASTProcedure: {0}", ast->name->value);

    auto [retType, argTypes] = do_proc(*ast);

    const auto proc_type = std::make_shared<ProcedureType>(retType, argTypes);
    const auto sym = mkSym(proc_type, Attr::global_var);
    symboltable.put(ast->name->value, sym);

    last_proc = ast;

    // new symbol table
    symboltable.push_frame(ast->name->value);

    // do receiver
    if (ast->receiver.first) {
        const auto attr = ast->receiver.first->is(Attr::var) ? Attr::var : Attr::null;
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
            overloaded{[this](auto arg) { visit(arg); }, // lambda arg can't be reference here
                       [this, param](ASTQualident const &tname) {
                           debug("ASTProcedure param type ident");
                           const auto type = types.find(tname->id->value);
                           symboltable.put(
                               param->value,
                               mkSym(type, param->is(Attr::var) ? Attr::var : Attr::null));
                       }},
            type->type);
        symboltable.put(param->value, mkSym(argTypes[count].first,
                                            param->is(Attr::var) ? Attr::var : Attr::null));
        count++;
    };
    if (ast->decs) {
        visit(ast->decs);
    }

    // check local procedures
    for (auto const &proc : ast->procedures) {
        proc->accept(this);
    }

    // check statements
    symboltable.reset_free_variables();
    for (auto const &stat : ast->stats) {
        stat->accept(this);
    }
    auto free_variables = symboltable.get_free_variables();
    for (auto const &f : free_variables) {
        if (f == ast->name->value) {
            // skip recursive definitions
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
        const auto v = std::make_pair(std::make_shared<ASTIdentifier_>(closure_arg), type);
        ast->params.insert(ast->params.begin(), v);
        symboltable.put(closure_arg, mkSym(proc_type->get_closure_struct(), Attr::closure));
    }
    symboltable.pop_frame();
}

void Inspector::visit(ASTProcedureForward const &ast) {
    debug("ASTProcedureForward: {0}", ast->name->value);
    auto [retType, argTypes] = do_proc(*ast);

    auto proc_type = std::make_shared<ProcedureFwdType>();
    proc_type->ret = retType;
    proc_type->params = argTypes;
    // Override the definition placed in the symbol table by the parser
    symboltable.put(ast->name->value, mkSym(proc_type, Attr::global_var));
};

void Inspector::visit(ASTAssignment const &ast) {
    debug("ASTAssignment");
    visit(ast->expr);
    const auto expr_type = last_type;

    auto res = symboltable.find(ast->ident->ident->make_coded_id());
    if (res) {
        if (res->is(Attr::cnst)) {
            const auto e = TypeError(ast->get_location(), "Can't assign to CONST variable {0}",
                                     std::string(*ast->ident));
            errors.add(e);
            return;
        }
        if (res->is(Attr::read_only)) {
            const auto e =
                TypeError(ast->get_location(), "Can't assign to read only (-) variable {0}",
                          std::string(*ast->ident));
            errors.add(e);
            return;
        }
        res->set(Attr::modified);
    };

    visit(ast->ident);
    if (!last_type) {
        // error return;
        return;
    }
    const auto alias = types.resolve(last_type->get_name());
    assert(alias); // NOLINT
    debug("ASTAssignment type of ident: {0} -> {1}", last_type->get_name(), alias->get_name());
    last_type = alias;
    if (!(types.check(TokenType::ASSIGN, last_type, expr_type) ||
          (last_type->is_assignable() && last_type->equiv(expr_type)))) {
        const auto e = TypeError(ast->get_location(), "Can't assign expression of type {0} to {1}",
                                 string(expr_type), std::string(*ast->ident));
        errors.add(e);
    }
}

void Inspector::visit(ASTReturn const &ast) {
    debug("ASTReturn");
    Type expr_type = TypeTable::VoidType;
    if (ast->expr) {
        visit(ast->expr);
        if (last_type) {
            expr_type = types.resolve(last_type->get_name());
        }
    }

    // check return type
    if (last_proc) {
        const auto type = last_proc->return_type;
        auto       retType = TypeTable::VoidType;
        if (type != nullptr) {
            std::visit(overloaded{
                           [this, &retType](auto arg) {
                               visit(arg);
                               retType = types.resolve(last_type->get_name());
                           },
                           [this, &retType](ASTQualident const &type) {
                               if (const auto t = types.resolve(type->id->value); t) {
                                   retType = t;
                               };
                           },
                       },
                       type->type);
        }

        if (!expr_type->equiv(retType)) {
            const auto e =
                TypeError(ast->get_location(),
                          "RETURN type ({1}) does not match return type for function {0}: {2}",
                          last_proc->name->value, retType->get_name(), expr_type->get_name());
            errors.add(e);
        }
    }
}

void Inspector::visit(ASTCall const &ast) {
    auto name = get_Qualident(ast->name->ident);
    bool skip_argument_typecheck{false};

    debug("ASTCall: {0}", name);
    auto res = symboltable.find(name);
    if (!res) {
        std::ranges::replace(name, '_', '.');
        auto e = CodeGenException(ast->get_location(), "undefined PROCEDURE {0}", name);
        errors.add(e);
        return;
    }
    auto procType = std::dynamic_pointer_cast<ProcedureType>(res->type);
    if (!procType) {

        // check bound procedure
        auto field = ast->name->first_field();
        if (!field) {
            std::ranges::replace(name, '_', '.');
            auto e = TypeError(ast->get_location(), "{0} is not a PROCEDURE", name);
            errors.add(e);
            return;
        }

        name = field->value;
        if (res = symboltable.find(name); !res) {
            std::ranges::replace(name, '_', '.');
            auto e = TypeError(ast->get_location(), "{0} is not a PROCEDURE", name);
            errors.add(e);
            return;
        }

        procType = std::dynamic_pointer_cast<ProcedureType>(res->type);
        if (!procType) {
            auto e = TypeError(ast->get_location(), "{0} is not a PROCEDURE", name);
            errors.add(e);
            return;
        }

        debug("ASTCall found bound procedure {0}", name);
        visit(ast->name->ident); // type of only the identifier
        auto base_type = last_type;
        ast->name->ident->set_type(base_type);
        debug("ASTCall type {0}", string(base_type));
        if (!base_type->equiv(procType->receiver)) {
            auto e = TypeError(ast->get_location(),
                               "base type: {0} does not match bound procedure {1}, type: {2}",
                               string(base_type), name, string(procType->receiver));
            errors.add(e);
            return;
        }
    }

    // Check if procedure argument is a AnyType - then skip check argument types
    if (procType->params.empty() || procType->params[0].first != TypeTable::AnyType) {

        if (ast->args.size() != procType->params.size()) {
            std::ranges::replace(name, '_', '.');
            auto e = TypeError(
                ast->get_location(),
                "calling PROCEDURE {0}, incorrect number of arguments: {1} instead of {2}", name,
                ast->args.size(), procType->params.size());
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
         ++call_iter, ++proc_iter) {

        (*call_iter)->accept(this);

        if (proc_iter->second == Attr::var) {
            if (!is_lvalue || is_const) {
                debug("ASTCall is_lvalue: {0} is_const: {0}", is_lvalue, is_const);
                std::ranges::replace(name, '_', '.');
                auto e = TypeError(ast->get_location(),
                                   "procedure call {0} does not have a variable "
                                   "reference for VAR parameter {2}",
                                   name, last_type->get_name(), (*proc_iter).first->get_name());
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

        if (proc_base->id == TypeId::VOID || base_last->id == TypeId::ANY) {
            // Void type - accepts any type, used for builtin compile time functions
            continue;
        }

        debug("check parameter {0}: {1} with {2}", name, base_last->get_name(),
              proc_base->get_name());
        if (!proc_base->equiv(base_last)) {
            std::ranges::replace(name, '_', '.');
            debug("incorrect parameter");
            auto e = TypeError(ast->get_location(),
                               "procedure call {0} has incorrect "
                               "type {1} for parameter {2}",
                               name, last_type->get_name(), (*proc_iter).first->get_name());
            errors.add(e);
        }
    }

    // OK
    res->set(Attr::used);
    last_type = procType->ret;
    ast->set_type(last_type);
}

void Inspector::visit(ASTIf const &ast) {
    ast->if_clause.expr->accept(this);
    if (last_type != TypeTable::BoolType) {
        const auto e = TypeError(ast->get_location(), "IF expression must be type BOOLEAN");
        errors.add(e);
    }

    for (auto const &stat : ast->if_clause.stats) {
        stat->accept(this);
    }

    for (auto const &elsif_clause : ast->elsif_clause) {
        elsif_clause.expr->accept(this);
        if (last_type != TypeTable::BoolType) {
            auto e = TypeError(ast->get_location(), "ELSIF expression must be type BOOLEAN");
            errors.add(e);
        }
        for (auto const &stmt : elsif_clause.stats) {
            stmt->accept(this);
        }
    }
    if (ast->else_clause) {
        auto elses = *ast->else_clause;
        for (auto const &stmt : elses) {
            stmt->accept(this);
        }
    }
}

void Inspector::visit(ASTCaseElement const &ast) {
    for (const auto &stmt : ast->stats) {
        stmt->accept(this);
    }
}

void Inspector::visit(ASTCase const &ast) {

    visit(ast->expr);
    if (!last_type->equiv(TypeTable::IntType) && !last_type->equiv(TypeTable::CharType)) {
        auto ex =
            TypeError(ast->expr->get_location(), "CASE expression has to be INTEGER or CHAR");
        errors.add(ex);
    }
    Type const case_type = last_type;

    // elements
    for (auto &e : ast->elements) {
        for (auto expr : e->exprs) {
            if (std::holds_alternative<ASTSimpleExpr>(expr)) {
                auto casexpr = std::get<ASTSimpleExpr>(expr);
                visit(casexpr);
                if (!last_type->equiv(case_type)) {
                    auto ex = TypeError(casexpr->get_location(),
                                        "CASE expression mismatch type {0} does not "
                                        "match CASE expression type {1}",
                                        string(last_type), string(case_type));
                    errors.add(ex);
                }
            } else if (std::holds_alternative<ASTRange>(expr)) {
                auto range = std::get<ASTRange>(expr);
                visit(range->first);
                if (!last_type->equiv(case_type)) {
                    auto ex = TypeError(range->get_location(),
                                        "CASE expression range mismatch first type {0} does not "
                                        "match CASE expression type {1}",
                                        string(last_type), string(case_type));
                    errors.add(ex);
                }
                visit(range->last);
                if (!last_type->equiv(case_type)) {
                    auto ex = TypeError(range->get_location(),
                                        "CASE expression range mismatch last type {0} does not "
                                        "match CASE expression type {1}",
                                        string(last_type), string(case_type));
                    errors.add(ex);
                }
            }
        }
        visit(e);
    }

    // else
    for (auto &stmt : ast->else_stats) {
        stmt->accept(this);
    }
}

void Inspector::visit(ASTFor const &ast) {
    visit(ast->start);
    if (!last_type->is_numeric()) {
        auto e = TypeError(ast->get_location(), "FOR start expression must be numeric type");
        errors.add(e);
    }
    visit(ast->end);
    if (!last_type->is_numeric()) {
        auto e = TypeError(ast->get_location(), "FOR end expression must be numeric type");
        errors.add(e);
    }
    if (ast->by) {
        visit(ast->by);
        if (!last_type->is_numeric()) {
            auto e = TypeError(ast->get_location(), "FOR BY expression must be numeric type");
            errors.add(e);
        }
    }

    if (auto res = symboltable.find(ast->ident->value); !res) {
        auto e = TypeError(ast->get_location(), "FOR index variable {0} not defined",
                           ast->ident->value);
        errors.add(e);
    } else {
        auto resType = types.resolve(res->type->get_name());
        if (!resType || !ast->start->get_type()->equiv(resType)) {
            auto e = TypeError(ast->get_location(), "FOR index variable {0} wrong type",
                               ast->ident->value);
            errors.add(e);
        }
    }

    for (auto const &stat : ast->stats) {
        stat->accept(this);
    }
}

void Inspector::visit(ASTWhile const &ast) {
    visit(ast->expr);
    if (last_type != TypeTable::BoolType) {
        const auto e = TypeError(ast->get_location(), "WHILE expression must be type BOOLEAN");
        errors.add(e);
    }

    for (auto const &stat : ast->stats) {
        stat->accept(this);
    }
}

void Inspector::visit(ASTRepeat const &ast) {
    for (auto const &stat : ast->stats) {
        stat->accept(this);
    }
    visit(ast->expr);
    if (last_type != TypeTable::BoolType) {
        const auto e = TypeError(ast->get_location(), "REPEAT expression must be type BOOLEAN");
        errors.add(e);
    }
}

void Inspector::visit(ASTLoop const &ast) {
    for (auto const &stat : ast->stats) {
        stat->accept(this);
    }
}

void Inspector::visit(ASTBlock const &ast) {
    for (auto const &stat : ast->stats) {
        stat->accept(this);
    }
}

void Inspector::visit(ASTExpr const &ast) {
    visit(ast->expr);
    auto c1 = is_const;
    if (ast->relation) {
        is_lvalue = false;
        const auto t1 = last_type;
        visit(ast->relation_expr);
        const auto result_type = types.check(*ast->relation, t1, last_type);
        if (!result_type) {
            const auto e =
                TypeError(ast->get_location(), "operator {0} doesn't takes types {1} and {2}",
                          string(*ast->relation), string(t1), string(last_type));
            errors.add(e);
            return;
        }
        last_type = result_type;
        c1 = c1 && is_const; // if both are const
        is_const = c1;       // return the const value
    }
    ast->set_type(last_type);
}

void Inspector::visit(ASTSimpleExpr const &ast) {
    visit(ast->term);
    auto t1 = last_type;
    auto c1 = is_const;
    for (auto const &t : ast->rest) {
        is_lvalue = false;
        t.second->accept(this);
        const auto result_type = types.check(t.first, t1, last_type);
        if (!result_type) {
            const auto e =
                TypeError(ast->get_location(), "operator {0} doesn't takes types {1} and {2}",
                          string(t.first), string(t1), string(last_type));
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

void Inspector::visit(ASTTerm const &ast) {
    visit(ast->factor);
    auto t1 = last_type;
    auto c1 = is_const;
    for (auto const &t : ast->rest) {
        is_lvalue = false;
        t.second->accept(this);
        const auto result_type = types.check(t.first, t1, last_type);
        if (!result_type) {
            const auto e =
                TypeError(ast->get_location(), "operator {0} doesn't takes types {1} and {2}",
                          string(t.first), string(t1), string(last_type));
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

void Inspector::visit(ASTFactor const &ast) {
    std::visit(overloaded{
                   [this, ast](auto factor) {
                       visit(factor);
                       ast->set_type(last_type);
                   },
                   [this, ast](ASTCall const &factor) {
                       // need to pass on return type */
                       visit(factor);
                       is_lvalue = false;
                       ast->set_type(last_type);
                   },
                   [this, ast](ASTFactor const &arg) {
                       if (ast->is_not) {
                           visit(arg);
                           const auto result_type = types.check(TokenType::TILDE, last_type);
                           if (!result_type) {
                               auto e = TypeError(ast->get_location(),
                                                  "type in ~ expression must be BOOLEAN");
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

void Inspector::visit(ASTDesignator const &ast) {
    debug("ASTDesignator");
    visit(ast->ident);

    // check type array before processing selectors
    if (!last_type) {
        // null - error in identifier
        return;
    }
    bool       is_array = last_type->is_array();
    bool       is_record = last_type->id == TypeId::RECORD;
    bool       is_string = last_type->id == TypeId::STRING;
    bool const is_pointer = last_type->id == TypeId::POINTER;
    auto       b_type = last_type;
    if (!(is_array || is_record || is_string || is_pointer) && !ast->selectors.empty()) {
        auto e = TypeError(ast->get_location(), "variable {0} is not an indexable type",
                           ast->ident->id->value);
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
                auto e = TypeError(ast->get_location(), "value not indexable type");
                errors.add(e);
                return;
            }

            for (auto &expr : s) {
                visit(expr);
                if (!last_type->is_numeric()) {
                    auto e = TypeError(ast->get_location(),
                                       "expression in array index must be numeric");
                }
            }

            if (is_array) {
                auto array_type = std::dynamic_pointer_cast<ArrayType>(b_type);

                // check index count, zero array dimensions means open array
                if (!array_type->dimensions.empty() && array_type->dimensions.size() != s.size()) {
                    auto e = TypeError(ast->get_location(),
                                       "array indexes don't match array dimensions of {0}",
                                       ast->ident->id->value);
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
                auto e = TypeError(s.first->get_location(), "value not RECORD: {0}",
                                   last_type->get_name());
                errors.add(e);
                return;
            }

            auto record_type = std::dynamic_pointer_cast<RecordType>(b_type);
            auto field = record_type->get_type(s.first->value);
            if (!field) {
                auto e =
                    TypeError(ast->get_location(), "no field <{0}> in RECORD", s.first->value);
                errors.add(e);
                return;
            }

            // Store field index with identifier, for use later in the code
            // generator.

            s.second = record_type->get_index(s.first->value);
            debug("ASTDesignator record index {0} for {1}", s.second, s.first->value);

            last_type = *field;
            ast->set_type(last_type);
        } else if (std::holds_alternative<PointerRef>(ss)) {
            // Reference
            if (!is_pointer) {
                auto e = TypeError(ast->get_location(), "variable {0} is not a pointer ",
                                   std::string(*ast->ident));
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
        is_record = b_type->id == TypeId::RECORD;
        is_string = b_type->id == TypeId::STRING;
    }
} // namespace ax

void Inspector::visit(ASTType const &ast) {
    std::visit(overloaded{[this, ast](ASTQualident const &type) {
                              debug("ASTType {0}", type->id->value);
                              const auto result = types.find(type->id->value);
                              if (!result) {
                                  throw TypeError(ast->get_location(), "Unknown type: {0}",
                                                  type->id->value);
                              }
                              debug("ASTType 2 {0}", type->id->value);
                              ast->set_type(result);
                              last_type = result;
                          },
                          [this, ast](ASTArray const &arg) {
                              visit(arg);
                              ast->set_type(last_type);
                          },
                          [this, ast](ASTRecord const &arg) {
                              visit(arg);
                              ast->set_type(last_type);
                          },
                          [this, ast](ASTPointerType const &arg) {
                              visit(arg);
                              ast->set_type(last_type);
                          },
                          [this, ast](ASTEnumeration const &arg) {
                              visit(arg);
                              ast->set_type(last_type);
                          }},
               ast->type);
}

void Inspector::visit(ASTArray const &ast) {
    int i = 0;
    for (auto &expr : ast->dimensions) {
        visit(expr);
        if (!is_const) {
            auto e = TypeError(ast->get_location(),
                               "ARRAY expecting constant expression for dimension {0}", i);
            errors.add(e);
        }
        if (!last_type->is_numeric()) {
            auto e = TypeError(ast->get_location(),
                               "ARRAY expecting numeric size for dimension {0}", i);
            errors.add(e);
        }
        i++;
    }

    visit(ast->type);

    Type array_type{nullptr};
    if (ast->dimensions.empty()) {
        array_type = std::make_shared<ax::OpenArrayType>(last_type);
    } else {
        const auto at = std::make_shared<ax::ArrayType>(last_type);
        for (const auto &dim_expr : ast->dimensions) {
            at->dimensions.push_back(dim_expr->value);
        }
        array_type = at;
    }
    last_type = array_type;
    ast->set_type(last_type);
    types.put(last_type->get_name(), last_type);
}

void Inspector::visit(ASTRecord const &ast) {
    debug("ASTRecord");

    const auto rec_type = std::make_shared<ax::RecordType>();
    if (ast->base) {
        visit(ast->base);
        const auto baseType_name = std::string(*ast->base);
        debug("ASTRecord base {0}", baseType_name);
        auto baseType = types.resolve(baseType_name);
        if (!baseType) {
            const auto e = TypeError(ast->base->get_location(), "RECORD base type {0} not found",
                                     baseType_name);
            errors.add(e);
        } else if (baseType->id != TypeId::RECORD) {
            const auto e = TypeError(ast->base->get_location(),
                                     "RECORD base type {0} is not a record", baseType_name);
            errors.add(e);
        } else {
            const auto base_rec = std::dynamic_pointer_cast<RecordType>(baseType);
            rec_type->set_baseType(base_rec);
        }
    }

    for (auto const &field : ast->fields) {
        field.second->accept(this);

        // check if not already defined
        if (rec_type->has_field(field.first->value)) {
            auto e = TypeError(field.first->get_location(), "RECORD already has field {0}",
                               field.first->value);
            errors.add(e);
        } else {
            rec_type->insert(field.first->value, last_type);
        }
    }
    last_type = rec_type;
    ast->set_type(last_type);
    types.put(last_type->get_name(), last_type);
}

void Inspector::visit(ASTPointerType const &ast) {
    auto ref_name = std::string(*ast->reference);
    debug("ASTPointerType {0}", ref_name);
    std::shared_ptr<ax::PointerType> ptr_type;
    try {
        visit(ast->reference);
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

void Inspector::visit(ASTEnumeration const &ast) {
    debug("ASTEnumeration");
    auto enum_type = std::make_shared<EnumType>("");
    for (auto const &ident : ast->values) {
        if (symboltable.find(ident->value)) {
            const auto e = TypeError(ast->get_location(),
                                     "Enumeration identifier {0} already defined", ident->value);
            errors.add(e);
            continue;
        }
        enum_type->add_value(ident->value);
        symboltable.put(ident->value, mkSym(enum_type, Attr::cnst));
    }
    last_type = enum_type;
    ast->set_type(last_type);
}

std::string Inspector::get_Qualident(ASTQualident const &ast) const {
    std::string result;
    const auto  res = symboltable.find(ast->qual);

    if (ast->qual.empty()) {
        return ast->id->value;
    }
    if (res && res->type->id == TypeId::MODULE) {
        const auto module_name = std::dynamic_pointer_cast<ModuleType>(res->type)->module_name();
        result = ASTQualident_::make_coded_id(module_name, ast->id->value);
        // Rewrite AST with real module name
        ast->qual = module_name;
    }
    return result;
}

void Inspector::visit(ASTQualident const &ast) {
    debug("ASTQualident");
    if (ast->qual.empty()) {
        visit(ast->id);
        ast->set_type(last_type);
    } else {
        debug("ASTQualident {0}", ast->qual);

        const auto new_ast = make<ASTIdentifier_>();
        new_ast->value = get_Qualident(ast);
        is_qualid = true;
        qualid_error = false;
        visit(new_ast);
        if (qualid_error) {
            const auto e =
                CodeGenException(ast->get_location(), "undefined identifier {0} in MODULE {1}",
                                 ast->id->value, ast->qual);
            errors.add(e);
        }
        ast->set_type(last_type);
    }
}

void Inspector::visit(ASTIdentifier const &ast) {
    debug("ASTIdentifier");
    const auto res = symboltable.find(ast->value);
    if (!res) {
        if (!is_qualid) {

            // Check is type name and accept, can only then be passed to objects of
            // type VOID
            if (auto typep = types.find(ast->value)) {
                debug("type: {0}", ast->value);
                return;
            }

            const auto e =
                CodeGenException(ast->get_location(), "undefined identifier {0}", ast->value);
            errors.add(e);
            return;
        }
        qualid_error = true;
        return;
    }
    debug("find type: {0} for {1}", res->type->get_name(), ast->value);
    const auto resType = types.resolve(res->type->get_name());
    if (!resType) {
        const auto e = TypeError(ast->get_location(), "Unknown type: {0} for identifier {1}",
                                 res->type->get_name(), ast->value);
        errors.add(e);
        return;
    }
    last_type = resType;
    ast->set_type(last_type);
    is_const = res->is(Attr::cnst);
    if (last_type->id == TypeId::STRING) {
        ast->set(Attr::ptr);
    }
    is_lvalue = true;
}

// Constant literals

void Inspector::visit(ASTSet const &ast) {
    bool set_const = true;
    for (auto &e : ast->values) {
        std::visit(
            overloaded{[this, &set_const, ast](ASTSimpleExpr const &exp) {
                           debug("ASTSet exp");
                           visit(exp);
                           if (!TypeTable::IntType->equiv(last_type)) {
                               const auto e = TypeError(exp->get_location(),
                                                        "Expression {0} is not a integer type",
                                                        std::string(*exp));
                               errors.add(e);
                           }
                           set_const &= is_const;
                       },
                       [this, &set_const, ast](ASTRange const &exp) {
                           debug("ASTSet range");
                           visit(exp->first);
                           if (!TypeTable::IntType->equiv(last_type)) {
                               const auto e = TypeError(exp->first->get_location(),
                                                        "Expression {0} is not a integer type",
                                                        std::string(*exp->first));
                               errors.add(e);
                           }
                           set_const &= is_const;
                           visit(exp->last);
                           if (!TypeTable::IntType->equiv(last_type)) {
                               const auto e = TypeError(exp->last->get_location(),
                                                        "Expression {0} is not a integer type",
                                                        std::string(*exp->last));
                               errors.add(e);
                           }
                           set_const &= is_const;
                       }},
            e);
    }
    is_const = set_const;
    last_type = TypeTable::SetType;
    is_lvalue = false;
}

void Inspector::visit(ASTInteger const & /* not used */) {
    last_type = TypeTable::IntType;
    is_const = true;
    is_lvalue = false;
}

void Inspector::visit(ASTReal const & /* not used */) {
    last_type = TypeTable::RealType;
    is_const = true;
    is_lvalue = false;
}

void Inspector::visit(ASTCharPtr const & /* not used */) {
    last_type = TypeTable::CharType;
    is_const = true;
    is_lvalue = false;
}

void Inspector::visit(ASTString const &ast) {
    if (ast->value.size() == 1) {
        last_type = TypeTable::Str1Type;
    } else {
        last_type = TypeTable::StrType;
    }
    is_const = true;
    is_lvalue = false;
};

void Inspector::visit(ASTBool const & /* not used */) {
    last_type = TypeTable::BoolType;
    is_const = true;
    is_lvalue = false;
}

void Inspector::visit(ASTNil const & /*not used*/) {
    debug("ASTNil");
    last_type = TypeTable::VoidType; // NIL can be assigned to any pointer
    is_const = true;
    is_lvalue = false;
}

void Inspector::check(ASTModule const &ast) {
    visit(ast);
}

} // namespace ax
