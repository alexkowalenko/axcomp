//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "parser.hh"

#include <cstddef>
#include <format>
#include <memory>
#include <optional>
#include <utility>

#include <llvm/Support/Debug.h>

#include "ast.hh"
#include "astvisitor.hh"
#include "error.hh"
#include "symbol.hh"
#include "token.hh"
#include "type.hh"
#include "typetable.hh"

namespace ax {

constexpr auto DEBUG_TYPE{"parser "};

template <typename S, typename... Args> static void debug(const S &format, const Args &...msg) {
    LLVM_DEBUG(llvm::dbgs() << DEBUG_TYPE << std::vformat(format, std::make_format_args(msg...))
                            << '\n'); // NOLINT
}

// module identifier markers
inline const std::set<TokenType> module_markers{TokenType::asterisk, TokenType::dash};

void Parser::set_attrs(ASTIdentifier const &ident) {
    auto tok = lexer.peek_token();
    if (module_markers.find(tok.type) != module_markers.end()) {
        lexer.get_token();
        switch (tok.type) {
        case TokenType::asterisk:
            ident->set(Attr::global);
            return;
        case TokenType::dash:
            ident->set(Attr::read_only);
            return;
        default:
            return;
        }
    }
}

Token Parser::get_token(TokenType const &t) {
    auto tok = lexer.get_token();
    if (tok.type != t) {
        throw ParseException(
            std::format("Unexpected token: {0} - expecting {1}", std::string(tok), string(t)),
            lexer.get_location());
    }
    return tok;
}

inline const std::set<TokenType> module_ends{TokenType::end};

/**
 * @brief module -> "MODULE" IDENT ";"  declarations "BEGIN" statement_seq "END"
 * IDENT "."
 *
 * @return ASTModulePtr
 */
ASTModule Parser::parse_module() {
    auto module = makeAST<ASTModule_>(lexer);

    // MODULE ident BEGIN (expr)+ END ident.
    get_token(TokenType::module);
    auto tok = get_token(TokenType::ident);
    module->name = tok.val;
    symbols.put(module->name, mkSym(std::make_shared<ModuleType>(module->name)));
    get_token(TokenType::semicolon);

    tok = lexer.peek_token();
    if (tok.type == TokenType::import) {
        module->import = parse_import();
    }

    // Declarations
    module->decs = parse_declaration();

    // Procedures
    tok = lexer.peek_token();
    while (tok.type == TokenType::procedure) {
        lexer.get_token(); // PROCEDURE
        tok = lexer.peek_token();
        if (tok.type == TokenType::caret) {
            module->procedures.push_back(parse_procedureForward());
        } else {
            module->procedures.push_back(parse_procedure());
        }
        tok = lexer.peek_token();
    }

    tok = lexer.peek_token();
    if (tok.type == TokenType::begin) {
        get_token(TokenType::begin);

        // statement_seq
        parse_statement_block(module->stats, module_ends);
    }

    // END
    get_token(TokenType::end);
    tok = get_token(TokenType::ident);
    if (tok.val != module->name) {
        throw ParseException(std::format("END identifier name: {0} doesn't match module name: {1}",
                                         tok.val, module->name),
                             lexer.get_location());
    }
    get_token(TokenType::period);
    return module;
}

/**
 * @brief "IMPORT" Import {"," Import} ";".
 *
 * Import = = [ident ":="] ident.
 *
 * @return ASTImportPtr
 */
ASTImport Parser::parse_import() {
    debug("parse_import");
    auto ast = makeAST<ASTImport_>(lexer);

    lexer.get_token(); // IMPORT

    while (true) {
        auto ident = parse_identifier();

        auto tok = lexer.peek_token();
        if (tok.type == TokenType::assign) {
            lexer.get_token(); // :=
            auto second = parse_identifier();
            ast->imports.emplace_back(second, ident);
            auto module = std::make_shared<ModuleType>(second->value);
            symbols.put(ident->value, mkSym(module, Attr::global_var));
        } else {
            ast->imports.emplace_back(ident, nullptr);
            auto module = std::make_shared<ModuleType>(ident->value);
            symbols.put(ident->value, mkSym(module, Attr::global_var));
        }
        tok = lexer.peek_token();
        if (tok.type != TokenType::comma) {
            break;
        }
        get_token(TokenType::comma);
    }
    get_token(TokenType::semicolon);
    return ast;
}

/**
 * @brief declarations -> ["CONST" (IDENT "=" expr ";")* ]
                ["TYPE" (IDENT "=" type ";")* ]
                ["VAR" (IDENT ":" type ";")* ]
                ( procedureDeclaration ";")*
 *
 * @return ASTDeclarationPtr
 */
ASTDeclaration Parser::parse_declaration() {
    debug("parse_declaration");
    auto decs = makeAST<ASTDeclaration_>(lexer);

    auto tok = lexer.peek_token();
    while (tok.type == TokenType::cnst || tok.type == TokenType::type ||
           tok.type == TokenType::var) {

        switch (tok.type) {
        case TokenType::cnst: {
            auto cnsts = parse_const();
            if (!decs->cnst) {
                decs->cnst = cnsts;
            } else {
                decs->cnst->consts.insert(decs->cnst->consts.end(), cnsts->consts.begin(),
                                          cnsts->consts.end());
            }
            break;
        }
        case TokenType::type: {
            auto types = parse_typedec();
            if (!decs->type) {
                decs->type = types;
            } else {
                decs->type->types.insert(decs->type->types.end(), begin(types->types),
                                         end(types->types));
            }
            break;
        }
        case TokenType::var: {
            auto vars = parse_var();
            if (!decs->var) {
                decs->var = vars;
            } else {
                decs->var->vars.insert(decs->var->vars.end(), vars->vars.begin(),
                                       vars->vars.end());
            }
            break;
        }
        default:
            throw ParseException(std::format("unimplemented {0}", tok.val), lexer.get_location());
        }
        tok = lexer.peek_token();
    }
    return decs;
};

/**
 * @brief "CONST" (IdentDef "=" INTEGER ";")*
 *
 * @return ASTConstPtr
 */
ASTConst Parser::parse_const() {
    debug("parse_const");
    auto cnst = makeAST<ASTConst_>(lexer);

    lexer.get_token(); // CONST
    auto tok = lexer.peek_token();
    while (tok.type == TokenType::ident) {
        ConstDec dec;
        dec.ident = parse_identifier();
        set_attrs(dec.ident);
        get_token(TokenType::equals);
        dec.value = parse_expr();
        get_token(TokenType::semicolon);

        // Not sure what type this const is yet.
        symbols.put(dec.ident->value, mkSym(TypeTable::VoidType, Attr::cnst));
        cnst->consts.push_back(dec);
        tok = lexer.peek_token();
    }
    return cnst;
}

/**
 * @brief IdentList = IdentDef {"," IdentDef}.
 *
 * @param list
 */
void Parser::parse_identList(std::vector<ASTIdentifier> &list) {

    auto id = parse_identifier();
    set_attrs(id);
    list.push_back(id);
    auto tok = lexer.peek_token();
    while (tok.type == TokenType::comma) {
        lexer.get_token();
        id = parse_identifier();
        set_attrs(id);
        list.push_back(id);
        tok = lexer.peek_token();
    }
}

/**
 * @brief TypeDeclaration = IdentDef "=" type.
 *
 * @return ASTTypeDecPtr
 */
ASTTypeDec Parser::parse_typedec() {
    debug("parse_typedec");
    auto type = makeAST<ASTTypeDec_>(lexer);

    lexer.get_token(); // TYPE
    auto tok = lexer.peek_token();
    while (tok.type == TokenType::ident) {
        VarDec dec;
        dec.first = parse_identifier();
        set_attrs(dec.first);
        get_token(TokenType::equals);
        dec.second = parse_type();
        get_token(TokenType::semicolon);

        type->types.push_back(dec);
        tok = lexer.peek_token();
    }
    return type;
}

/**
 * @brief  "VAR" (identList ":" type ";")*
 *
 * @return ASTVarPtr
 */
ASTVar Parser::parse_var() {
    debug("parse_var");
    auto var = makeAST<ASTVar_>(lexer);

    lexer.get_token(); // VAR
    auto tok = lexer.peek_token();
    while (tok.type == TokenType::ident) {
        std::vector<ASTIdentifier> list;
        parse_identList(list);
        get_token(TokenType::colon);
        auto type = parse_type();
        get_token(TokenType::semicolon);

        std::for_each(begin(list), end(list), [=, this](auto const &i) {
            VarDec dec;
            dec.first = i;
            dec.second = type;
            // delay type assignment to inspector
            symbols.put(dec.first->value, mkSym(TypeTable::VoidType));
            var->vars.push_back(dec);
        });
        tok = lexer.peek_token();
    }
    return var;
}

/**
 * @brief ( [VAR] identifier : identifier)
 *
 * @return VarDec
 */
RecVar Parser::parse_receiver() {
    debug("parse_receiver");
    RecVar result;
    lexer.get_token(); // (

    auto tok = lexer.peek_token();
    bool var{false};
    if (tok.type == TokenType::var) {
        var = true;
        lexer.get_token();
    }
    result.first = parse_identifier();
    if (var) {
        result.first->set(Attr::var);
    }
    get_token(TokenType::colon);
    result.second = parse_identifier();
    get_token(TokenType::r_paren);
    return result;
}

void Parser::parse_proc(ASTProc_ &proc) {
    debug("parse_proc");

    auto tok = lexer.peek_token();
    if (tok.type == TokenType::l_paren) {
        proc.receiver = parse_receiver();
    }

    proc.name = parse_identifier();
    set_attrs(proc.name);

    // Parameters
    tok = lexer.peek_token();
    if (tok.type == TokenType::l_paren) {
        parse_parameters(proc.params);
    }

    tok = lexer.peek_token();
    if (tok.type == TokenType::colon) {
        // Do return type
        lexer.get_token();
        proc.return_type = parse_type();
    }

    get_token(TokenType::semicolon);
}

/**
 * @brief "PROCEDURE" IdentDef [formalParameters] [ ":" type ]
 *         declarations ["BEGIN" statement_seq] "END" ident ";"
 *
 * @return ASTProcedurePtr
 */
ASTProcedure Parser::parse_procedure() {
    debug("parse_procedure");
    auto proc = makeAST<ASTProcedure_>(lexer);

    parse_proc(*proc);
    debug("parse_procedure {0}", proc->name->value);

    // Declarations
    symbols.push_frame(proc->name->value);
    proc->decs = parse_declaration();

    // Procedures
    auto tok = lexer.peek_token();
    while (tok.type == TokenType::procedure) {
        lexer.get_token(); // PROCEDURE
        tok = lexer.peek_token();
        if (tok.type == TokenType::caret) {
            proc->procedures.push_back(parse_procedureForward());
        } else {
            proc->procedures.push_back(parse_procedure());
        }
        tok = lexer.peek_token();
    }

    // statement_seq
    tok = lexer.peek_token();
    if (tok.type == TokenType::begin) {
        get_token(TokenType::begin);
        parse_statement_block(proc->stats, module_ends);
    }
    symbols.pop_frame();

    // END
    get_token(TokenType::end);
    tok = get_token(TokenType::ident);
    if (tok.val != proc->name->value) {
        throw ParseException(std::format("END name: {0} doesn't match procedure name: {1}",
                                         tok.val, proc->name->value),
                             lexer.get_location());
    }
    get_token(TokenType::semicolon);

    // Put into symbol table as a procedure
    auto forward = std::make_shared<ProcedureFwdType>();
    symbols.put(proc->name->value, mkSym(forward));
    return proc;
}

ASTProcedureForward Parser::parse_procedureForward() {
    debug("parse_procedureForward");
    auto proc = makeAST<ASTProcedureForward_>(lexer);

    lexer.get_token(); // ^
    parse_proc(*proc);
    debug("parse_procedureForward {0}", proc->name->value);

    // Put into symbol table as a procedure
    auto forward = std::make_shared<ProcedureFwdType>();
    symbols.put(proc->name->value, mkSym(forward));
    return proc;
}

/**
 * @brief formalParameters
 * = "(" [ ["VAR"] (identList : INDENT)* (";"  ["VAR"] (identList :
 * INDENT)*)]
 * "")"
 *
 * @return std::vector<VarDec>
 */
void Parser::parse_parameters(std::vector<VarDec> &params) {
    lexer.get_token(); // get (
    auto tok = lexer.peek_token();
    while (tok.type != TokenType::r_paren) {

        Attr attr = Attr::null;

        auto tok = lexer.peek_token();
        if (tok.type == TokenType::var) {
            lexer.get_token();
            attr = Attr::var;
        }
        std::vector<ASTIdentifier> ids;
        auto                       id = parse_identifier();
        if (attr == Attr::var) {
            id->set(attr);
        }
        ids.push_back(id);
        tok = lexer.peek_token();
        while (tok.type == TokenType::comma) {
            lexer.get_token();
            id = parse_identifier();
            if (attr == Attr::var) {
                id->set(attr);
            }
            ids.push_back(id);
            tok = lexer.peek_token();
        }
        get_token(TokenType::colon);
        auto type = parse_type();
        std::for_each(begin(ids), end(ids),
                      [&params, type](auto const &id) { params.push_back(VarDec{id, type}); });
        tok = lexer.peek_token();
        if (tok.type == TokenType::semicolon) {
            lexer.get_token(); // get ;
            tok = lexer.peek_token();
            continue;
        }
        if (tok.type == TokenType::r_paren) {
            break;
        }
        throw ParseException("expecting ; or ) in parameter list", lexer.get_location());
    }
    lexer.get_token(); // get )
}

/**
 * @brief assignment
    | procedureCall
    | assignmentStatement
    | ifStatment
    | forStatement
    | whileStatement
    | repeatStatement
    | EXIT
    | loopStatement
    | blockStatement
    | "RETURN" [expr]
 *
 * @return ASTStatementPtr
 */
ASTStatement Parser::parse_statement() {
    auto tok = lexer.peek_token();
    debug("parse_statement {0}", std::string(tok));
    switch (tok.type) {
    case TokenType::ret:
        return parse_return();
    case TokenType::if_k:
        return parse_if();
    case TokenType::cse:
        return parse_case();
    case TokenType::for_k:
        return parse_for();
    case TokenType::while_k:
        return parse_while();
    case TokenType::repeat:
        return parse_repeat();
    case TokenType::exit:
        return parse_exit();
    case TokenType::loop:
        return parse_loop();
    case TokenType::begin:
        return parse_block();
    case TokenType::ident: {
        // This can be an assignment or function call

        auto designator = parse_designator();

        tok = lexer.peek_token();
        debug("parse_statement next {0}", std::string(tok));

        if (tok.type == TokenType::l_paren || tok.type == TokenType::semicolon ||
            tok.type == TokenType::end) {
            return parse_call(designator);
        }
        return parse_assignment(designator);
    }
    default:
        throw ParseException(std::format("Unexpected token: {0}", std::string(tok)),
                             lexer.get_location());
    }
}

void Parser::parse_statement_block(std::vector<ASTStatement> &stats,
                                   const std::set<TokenType> &end_tokens) {

    while (true) {
        auto tok = lexer.peek_token();
        if (tok.type == TokenType::semicolon) {
            get_token(TokenType::semicolon);
        } else if (end_tokens.count(tok.type)) {
            return;
        } else {
            stats.push_back(parse_statement());
        }
    }
}

/**
 * @brief ident ":=" expr
 *
 * @return ASTAssignmentPtr
 */
ASTAssignment Parser::parse_assignment(ASTDesignator d) {
    debug("parse_assignment");
    auto assign = makeAST<ASTAssignment_>(lexer);

    assign->ident = std::move(d);
    get_token(TokenType::assign);
    assign->expr = parse_expr();
    return assign;
}

/**
 * @brief RETURN [expr]
 *
 * @return ASTReturnPtr
 */
ASTReturn Parser::parse_return() {
    debug("parse_return");
    auto ret = makeAST<ASTReturn_>(lexer);
    get_token(TokenType::ret);
    auto tok = lexer.peek_token();
    if (tok.type != TokenType::semicolon && tok.type != TokenType::end) {
        ret->expr = parse_expr();
    }
    return ret;
}

/**
 * @brief EXIT
 *
 * @return ASTExitPtr
 */
ASTExit Parser::parse_exit() {
    get_token(TokenType::exit);
    auto ex = makeAST<ASTExit_>(lexer);
    return ex;
};

/**
 * @brief  IDENT "(" expr ( "," expr )* ")"
 *
 * @return ASTCallPtr
 */
ASTCall Parser::parse_call(ASTDesignator d) {
    auto call = makeAST<ASTCall_>(lexer);

    call->name = std::move(d);
    auto tok = lexer.peek_token();
    if (tok.type == TokenType::l_paren) {
        // Parse arguments
        get_token(TokenType::l_paren);
        auto tok = lexer.peek_token();
        while (tok.type != TokenType::r_paren) {
            auto expr = parse_expr();
            call->args.push_back(expr);
            tok = lexer.peek_token();
            if (tok.type == TokenType::r_paren) {
                break;
            }
            if (tok.type == TokenType::comma) {
                lexer.get_token(); // get ,
                continue;
            }
            throw ParseException(std::format("Unexpected {0} expecting , or )", tok.val),
                                 lexer.get_location());
        }
        get_token(TokenType::r_paren);
    }
    return call;
}

inline const std::set<TokenType> if_ends{TokenType::end, TokenType::elsif, TokenType::else_k};

/**
 * @brief "IF" expression "THEN" statement_seq
    ( "ELSIF" expression "THEN" statement_seq )*
    [ "ELSE" statement_seq "END" ]
 *
 * @return ASTIfPtr
 */
ASTIf Parser::parse_if() {
    debug("parse_if");
    auto stat = makeAST<ASTIf_>(lexer);

    // IF
    get_token(TokenType::if_k);
    stat->if_clause.expr = parse_expr();

    // THEN
    get_token(TokenType::then);
    parse_statement_block(stat->if_clause.stats, if_ends);

    // ELSIF
    auto tok = lexer.peek_token();
    while (tok.type == TokenType::elsif) {
        debug("parse_if elsif");
        lexer.get_token();
        ASTIf_::IFClause clause;
        clause.expr = parse_expr();
        get_token(TokenType::then);
        parse_statement_block(clause.stats, if_ends);
        stat->elsif_clause.push_back(clause);
        tok = lexer.peek_token();
    }
    // ELSE
    if (tok.type == TokenType::else_k) {
        debug("parse_if else\n");
        lexer.get_token();
        stat->else_clause = std::make_optional<std::vector<ASTStatement>>();
        parse_statement_block(*stat->else_clause, module_ends);
    }
    // END
    get_token(TokenType::end);
    return stat;
};

/**
 * @brief CaseLabels = ConstExpression ["..." ConstExpression].
 *
 * ConstExpr = SimpleExpr.
 *
 * @return std::variant<ASTSimpleExprPtr, ASTRangePtr>
 */

std::variant<ASTSimpleExpr, ASTRange> Parser::parse_caseLabel() {
    debug("parse_caseLabel");
    auto e = parse_simpleexpr();
    auto tok = lexer.peek_token();
    if (tok.type == TokenType::dotdot) {
        lexer.get_token(); // ..
        auto range = makeAST<ASTRange_>(lexer);
        range->first = e;
        range->last = parse_simpleexpr();
        debug("parse_caseLabel range {0}..{1}", std::string(*e), std::string(*(range->last)));
        return {range};
    }

    debug("parse_caseLabel {0}", std::string(*e));
    return {e};
}

inline const std::set<TokenType> case_element_ends{TokenType::bar, TokenType::else_k,
                                                   TokenType::end};

/**
 * @brief  = [CaseLabelList ":" StatementSequence].
 *
 * CaseLabelList = CaseLabels {"," CaseLabels}.
 *
 * @return std::vector<ASTCaseElementPtr>
 */
void Parser::parse_caseElements(std::vector<ASTCaseElement> &elements) {
    debug("parse_caseElements");

    auto tok = lexer.peek_token();
    if (tok.type == TokenType::bar) {
        lexer.get_token();
        tok = lexer.peek_token();
    }
    while (tok.type != TokenType::end && tok.type != TokenType::else_k) {
        debug("parse_caseElement {0}", std::string(tok));
        auto element = makeAST<ASTCaseElement_>(lexer);

        while (tok.type != TokenType::colon) {
            auto v = parse_caseLabel();
            element->exprs.push_back(v);

            tok = lexer.peek_token();
            if (tok.type != TokenType::comma) {
                break;
            }
            lexer.get_token(); // ,
        };

        get_token(TokenType::colon);
        parse_statement_block(element->stats, case_element_ends);
        elements.push_back(element);

        tok = lexer.peek_token();
        debug("parse_caseElement {0}", std::string(tok));
        if (tok.type == TokenType::bar) {
            lexer.get_token();
            tok = lexer.peek_token();
        }
    }
}

/**
 * @brief = CASE Expression OF Case {"|" Case} [ELSE StatementSequence] END.
 *
 * @return ASTCasePtr
 */
ASTCase Parser::parse_case() {
    debug("parse_case");
    auto ast = makeAST<ASTCase_>(lexer);

    // CASE
    get_token(TokenType::cse);
    ast->expr = parse_simpleexpr();
    get_token(TokenType::of);

    parse_caseElements(ast->elements);

    // ELSE
    auto tok = lexer.peek_token();
    debug("parse_case {0}", std::string(tok));
    if (tok.type == TokenType::else_k) {
        lexer.get_token();
        parse_statement_block(ast->else_stats, module_ends);
    }

    // END
    get_token(TokenType::end);
    return ast;
}

/**
 * @brief "FOR" IDENT ":=" expr "TO" expr [ "BY" INTEGER ] "DO"
    statement_seq "END"
 *
 * @return ASTForPtr
 */
ASTFor Parser::parse_for() {
    debug("parse_for");
    auto ast = makeAST<ASTFor_>(lexer);

    // FOR
    get_token(TokenType::for_k);
    ast->ident = parse_identifier();
    // :=
    get_token(TokenType::assign);
    ast->start = parse_expr();
    // TO
    get_token(TokenType::to);
    ast->end = parse_expr();

    // BY
    auto tok = lexer.peek_token();
    if (tok.type == TokenType::by) {
        lexer.get_token();
        ast->by = parse_expr();
    }

    // DO
    get_token(TokenType::do_k);
    parse_statement_block(ast->stats, module_ends);
    // END
    get_token(TokenType::end);
    return ast;
}

ASTWhile Parser::parse_while() {
    auto ast = makeAST<ASTWhile_>(lexer);

    // WHILE
    get_token(TokenType::while_k);
    ast->expr = parse_expr();

    // DO
    get_token(TokenType::do_k);
    parse_statement_block(ast->stats, module_ends);
    // END
    get_token(TokenType::end);
    return ast;
}

inline const std::set<TokenType> repeat_ends{TokenType::until};

ASTRepeat Parser::parse_repeat() {
    auto ast = makeAST<ASTRepeat_>(lexer);

    // REPEAT
    get_token(TokenType::repeat);

    parse_statement_block(ast->stats, repeat_ends);

    // UNTIL
    get_token(TokenType::until);
    ast->expr = parse_expr();

    return ast;
}

ASTLoop Parser::parse_loop() {
    auto ast = makeAST<ASTLoop_>(lexer);

    // LOOP
    get_token(TokenType::loop);

    parse_statement_block(ast->stats, module_ends);

    // END
    get_token(TokenType::end);
    return ast;
}

ASTBlock Parser::parse_block() {
    auto ast = makeAST<ASTBlock_>(lexer);
    // BEGIN
    get_token(TokenType::begin);

    parse_statement_block(ast->stats, module_ends);

    // END
    get_token(TokenType::end);
    return ast;
}

/**
 * @brief expr = simpleExpr [ relation simpleExpr]
 *
 * relation = "=" | "#" | "<" | "<=" | ">" | ">="
 *
 * @return ASTExprPtr
 */

inline const std::set<TokenType> relationOps = {
    TokenType::equals,  TokenType::hash, TokenType::less, TokenType::leq,
    TokenType::greater, TokenType::gteq, TokenType::in};

ASTExpr Parser::parse_expr() {
    debug("parse_expr");
    auto ast = makeAST<ASTExpr_>(lexer);

    ast->expr = parse_simpleexpr();
    auto tok = lexer.peek_token();
    if (relationOps.find(tok.type) != relationOps.end()) {
        lexer.get_token(); // get token;
        ast->relation = std::optional<TokenType>(tok.type);
        ast->relation_expr = parse_simpleexpr();
    }
    return ast;
}

/**
 * @brief expr -> ('+' | '-' )? term ( ('+' | '-' | "OR" ) term)*
 *
 * @return ASTSimpleExprPtr
 */
ASTSimpleExpr Parser::parse_simpleexpr() {
    debug("parse_simpleexpr");
    auto expr = makeAST<ASTSimpleExpr_>(lexer);

    auto tok = lexer.peek_token();
    if (tok.type == TokenType::plus || tok.type == TokenType::dash) {
        lexer.get_token();
        expr->first_sign = std::optional<TokenType>{tok.type};
    }
    expr->term = parse_term();
    tok = lexer.peek_token();
    while (tok.type == TokenType::plus || tok.type == TokenType::dash ||
           tok.type == TokenType::or_k) {
        lexer.get_token();
        ASTSimpleExpr_::Expr_add add{tok.type, parse_term()};
        expr->rest.push_back(add);
        tok = lexer.peek_token();
    }
    return expr;
}

/**
 * @brief term -> INTEGER ( ( '*' | 'DIV' | 'MOD' | "&") INTEGER)*
 *
 * @return ASTTermPtr
 */

inline const std::set<TokenType> termOps = {TokenType::asterisk, TokenType::slash, TokenType::div,
                                            TokenType::mod, TokenType::ampersand};

ASTTerm Parser::parse_term() {
    debug("parse_term");
    auto term = makeAST<ASTTerm_>(lexer);

    term->factor = parse_factor();
    auto tok = lexer.peek_token();
    debug("term {}", std::string(tok));
    while (termOps.find(tok.type) != termOps.end()) {
        lexer.get_token();
        ASTTerm_::Term_mult mult{tok.type, parse_factor()};
        term->rest.push_back(mult);
        tok = lexer.peek_token();
        debug("term {}", std::string(tok));
    }
    return term;
}

/**
 * @brief factor -> designator
 *                  | procedureCall
 *                  | INTEGER
 *                  | "TRUE" | "FALSE"
 *                  | char
 *                  | string
 *                  | NIL
 *                  | ~ factor
 *                  | '(' expr ')'
 *
 * @return ASTFactorPtr
 */
ASTFactor Parser::parse_factor() {
    debug("parse_factor");
    auto ast = makeAST<ASTFactor_>(lexer);

    auto tok = lexer.peek_token();
    debug("factor {0}", std::string(tok));
    switch (tok.type) {
    case TokenType::l_paren:
        // Expression
        lexer.get_token(); // get (
        ast->factor = parse_expr();
        get_token(TokenType::r_paren);
        return ast;
    case TokenType::integer:
    case TokenType::hexinteger: {
        ast->factor = parse_integer();
        return ast;
    }
    case TokenType::real:
        ast->factor = parse_real();
        return ast;
    case TokenType::hexchr:
    case TokenType::chr:
        ast->factor = parse_char();
        return ast;
    case TokenType::string:
        ast->factor = parse_string();
        return ast;
    case TokenType::true_k:
    case TokenType::false_k:
        ast->factor = parse_boolean();
        return ast;
    case TokenType::nil:
        ast->factor = parse_nil();
        return ast;
    case TokenType::l_brace:
        ast->factor = parse_set();
        return ast;
    case TokenType::tilde:
        lexer.get_token(); // get ~
        ast->is_not = true;
        ast->factor = parse_factor();
        return ast;
    case TokenType::ident: {

        auto d = parse_designator();
        auto nexttok = lexer.peek_token();
        auto res = symbols.find(std::string(*d));
        debug("parse_factor nexttok: {0} find: {1} {2}", std::string(nexttok), std::string(*d));
        if (nexttok.type == TokenType::l_paren || (res && res->type->id == TypeId::procedureFwd)) {
            debug("parse_factor call: {0}");
            ast->factor = parse_call(d);
            return ast;
        }
        ast->factor = d;
        return ast;
    }
    default:
        throw ParseException(std::format("Unexpected token: {0}", std::string(tok)),
                             lexer.get_location());
    }
    return nullptr; // Not factor
}

/**
 * @brief IDENT selector
 *
 * selector = ( '[' exprList ']' | '.' IDENT )*
 *
 * exprList = simpleExpr {"," simpleExpr}.
 *
 * @return ASTDesignatorPtr
 */

inline const std::set<TokenType> designatorOps = {TokenType::l_bracket, TokenType::period,
                                                  TokenType::caret};

ASTDesignator Parser::parse_designator() {
    debug("parse_designator");
    auto ast = makeAST<ASTDesignator_>(lexer);

    ast->ident = parse_qualident();

    auto tok = lexer.peek_token();
    while (designatorOps.find(tok.type) != designatorOps.end()) {
        switch (tok.type) {
        case TokenType::l_bracket: {
            lexer.get_token(); // [
            ArrayRef ref;
            debug("parse_designator array ref");
            do {
                ref.push_back(parse_simpleexpr());
                tok = lexer.peek_token();
                if (tok.type == TokenType::comma) {
                    lexer.get_token();
                    continue;
                }
            } while (tok.type != TokenType::r_bracket);
            get_token(TokenType::r_bracket);
            ast->selectors.emplace_back(ref);
            break;
        }
        case TokenType::period:
            lexer.get_token(); // .
            ast->selectors.push_back(FieldRef{parse_identifier(), -1});
            break;
        case TokenType::caret:
            lexer.get_token(); // ^
            ast->selectors.emplace_back(PointerRef{true});
            break;
        default:;
        }

        tok = lexer.peek_token();
    }
    return ast;
}

/**
 * @brief  INDENT | arraytype
 *
 * @return ASTTypePtr
 */
ASTType Parser::parse_type() {
    auto ast = makeAST<ASTType_>(lexer);

    auto tok = lexer.peek_token();
    switch (tok.type) {
    case TokenType::array:
        ast->type = parse_array();
        return ast;
    case TokenType::record:
        ast->type = parse_record();
        return ast;
    case TokenType::pointer:
        ast->type = parse_pointer();
        return ast;
    default:
        ast->type = parse_qualident();
        return ast;
    }
}

/**
 * @brief "ARRAY" (expr "[" , expr "]") "OF" type
 *
 * @return ASTArrayPtr
 */
ASTArray Parser::parse_array() {
    auto ast = makeAST<ASTArray_>(lexer);

    get_token(TokenType::array);
    auto tok = lexer.peek_token();
    while (tok.type != TokenType::of) {
        auto expr = parse_integer();
        ast->dimensions.push_back(expr);
        tok = lexer.peek_token();
        if (tok.type == TokenType::comma) {
            lexer.get_token();
            tok = lexer.peek_token();
        } else {
            break;
        }
    }
    get_token(TokenType::of);
    ast->type = parse_type();
    return ast;
}

/**
 * @brief "RECORD" "(" qualident ")" fieldList ( ";" fieldList )* "END"
 *
 * @return ASTRecordPtr
 */
ASTRecord Parser::parse_record() {
    debug("parse_record");
    auto ast = makeAST<ASTRecord_>(lexer);

    get_token(TokenType::record);
    auto tok = lexer.peek_token();
    if (tok.type == TokenType::l_paren) {
        lexer.get_token();
        ast->base = parse_qualident();
        get_token(TokenType::r_paren);
        tok = lexer.peek_token();
    }
    while (tok.type == TokenType::ident) {
        std::vector<ASTIdentifier> ids;
        parse_identList(ids);

        get_token(TokenType::colon);
        auto type = parse_type();

        std::for_each(begin(ids), end(ids),
                      [ast, type](auto const &i) { ast->fields.push_back(VarDec{i, type}); });
        tok = lexer.peek_token();
        if (tok.type == TokenType::semicolon) {
            lexer.get_token();
            tok = lexer.peek_token();
            continue;
        }
    }
    get_token(TokenType::end);
    return ast;
}

/**
 * @brief "POINTER" "TO" type
 *
 * @return ASTPointerTypePtr
 */
ASTPointerType Parser::parse_pointer() {
    debug("parse_record");
    auto ast = makeAST<ASTPointerType_>(lexer);

    get_token(TokenType::pointer);
    get_token(TokenType::to);
    ast->reference = parse_type();
    return ast;
}

/**
 * @brief Qualident = [ident "."] ident.
 *
 * @return ASTIdentifierPtr
 */
ASTQualident Parser::parse_qualident() {
    debug("parse_qualident");
    auto ast = makeAST<ASTQualident_>(lexer);

    auto first = get_token(TokenType::ident);
    auto tok = lexer.peek_token();
    if (tok.type == TokenType::period) {

        // Check if identifier is a imported module
        // If a module then make a full qualified identifier,
        // else it is a record access.

        lexer.get_token(); // .
        auto ident = get_token(TokenType::ident);

        auto res = symbols.find(first.val);
        if (res && std::dynamic_pointer_cast<ModuleType>(res->type)) {
            debug("parse_qualident found module {0}", first.val);
            ast->qual = first.val;
            ast->id = makeAST<ASTIdentifier_>(lexer);
            ast->id->value = ident.val;
        } else {
            debug("parse_qualident not found module {0}", first.val);
            // push back tokens
            lexer.push_token(ident);
            lexer.push_token(tok);
            ast->id = makeAST<ASTIdentifier_>(lexer);
            ast->id->value = first.val;
        }
    } else {
        ast->id = makeAST<ASTIdentifier_>(lexer);
        ast->id->value = first.val;
    }
    debug("parse_qualident: {0}", std::string(*ast));
    return ast;
}

/**
 * @brief IDENT
 *
 * @return ASTIdentifierPtr
 */
ASTIdentifier Parser::parse_identifier() {
    debug("parse_identifier");
    auto ast = makeAST<ASTIdentifier_>(lexer);

    auto tok = get_token(TokenType::ident);
    ast->value = tok.val;
    return ast;
}

/**
 * @brief "{"" [ element {, element}] "}""
 *
 * element = expr [".." expr]
 *
 */

ASTSet Parser::parse_set() {
    debug("parse_set");
    auto ast = makeAST<ASTSet_>(lexer);

    lexer.get_token(); // {
    auto tok = lexer.peek_token();
    while (tok.type != TokenType::r_brace) {
        ast->values.push_back(parse_caseLabel());
        tok = lexer.peek_token();
        if (tok.type == TokenType::r_brace) {
            break;
        }

        get_token(TokenType::comma);
        tok = lexer.peek_token();
    }
    lexer.get_token(); // }
    return ast;
}

constexpr int dec_radix = 10;
constexpr int hex_radix = 16;

/**
 * @brief digit {digit} | digit {hexDigit} "H".
 *
 * @return ASTIntegerPtr
 */
ASTInteger Parser::parse_integer() {
    debug("parse_integer");
    auto  ast = makeAST<ASTInteger_>(lexer);
    char *end = nullptr;

    // Either TokenType::hexinteger or TokenType::integer

    int  radix = dec_radix;
    auto tok = lexer.peek_token();
    if (tok.type == TokenType::hexinteger) {
        lexer.get_token();
        radix = hex_radix;
        ast->hex = true;
        ast->value = std::strtol(tok.val.c_str(), &end, radix);
        return ast;
    }
    if (tok.type == TokenType::integer) {
        lexer.get_token();
        debug("parse_integer: {0}", std::string(tok));
        ast->value = std::strtol(tok.val.c_str(), &end, radix);
        return ast;
    }
    throw ParseException(
        std::format("Unexpected token: {0} - expecting integer", std::string(tok)),
        lexer.get_location());
}

ASTReal Parser::parse_real() {
    debug("parse_real");
    auto ast = makeAST<ASTReal_>(lexer);

    auto tok = lexer.get_token();
    ast->style = tok.val;
    auto str = tok.val;
    if (str.find('D') != std::string::npos) {
        str.replace(str.find('D'), 1, "E");
    }
    try {
        ast->value = std::stod(str);
    } catch (std::invalid_argument &) {
        throw ParseException(std::format("REAL number invalid argument: {0} ", tok.val),
                             lexer.get_location());
    } catch (std::out_of_range &) {
        throw ParseException(std::format("REAL number out of range: {0} ", tok.val),
                             lexer.get_location());
    }
    return ast;
}

/**
 * @brief digit {hexDigit} "X" | "’" char "’"
 *
 * @return ASTCharPtr
 */
ASTCharPtr Parser::parse_char() {
    debug("parse_char");
    auto ast = makeAST<ASTChar_>(lexer);

    // Either TokenType::hexchr or TokenType::chr

    auto tok = lexer.get_token();
    if (tok.type == TokenType::hexchr) {
        char *end = nullptr;
        debug("parse_char hex {0}", tok.val);
        ast->value = Char(std::strtol(tok.val.c_str(), &end, hex_radix));
        ast->hex = true;
        return ast;
    }
    ast->value = tok.val_int;
    // debug("parse_char: x {0} {1}", ast->value, ast->str());
    return ast;
}

ASTString Parser::parse_string() {
    auto ast = makeAST<ASTString_>(lexer);

    auto tok = lexer.get_token();
    ast->delim = tok.val[0];
    ast->value = tok.val.substr(1);
    return ast;
}

/**
 * @brief TRUE | FALSE
 *
 * @return ASTBoolPtr
 */
ASTBool Parser::parse_boolean() {
    auto ast = makeAST<ASTBool_>(lexer);
    auto tok = lexer.get_token();
    ast->value = (tok.type == TokenType::true_k);
    return ast;
}

/**
 * @brief NIL
 *
 * @return ASTNilPtr
 */
ASTNil Parser::parse_nil() {
    auto ast = makeAST<ASTNil_>(lexer);
    get_token(TokenType::nil);
    return ast;
}

ASTModule Parser::parse() {
    return parse_module();
}

}; // namespace ax
