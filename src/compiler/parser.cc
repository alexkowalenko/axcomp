//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "parser.hh"

#include <format>
#include <memory>
#include <optional>
#include <utility>

#include <llvm/Support/Debug.h>

#include "ast/all.hh"
#include "astvisitor.hh"
#include "error.hh"
#include "symbol.hh"
#include "token.hh"
#include "types/all.hh"
#include "typetable.hh"

namespace ax {

constexpr auto DEBUG_TYPE{"parser"};

template <typename S, typename... Args> static void debug(const S &format, const Args &...msg) {
    LLVM_DEBUG(llvm::dbgs() << DEBUG_TYPE << ' '
                            << std::vformat(format, std::make_format_args(msg...))
                            << '\n'); // NOLINT
}

// module identifier markers
inline const std::set<TokenType> module_markers{TokenType::ASTÉRIX, TokenType::DASH};

void Parser::set_attrs(ASTIdentifier const &ident) const {
    auto tok = lexer.peek_token();
    if (module_markers.contains(tok.type)) {
        lexer.get_token();
        switch (tok.type) {
        case TokenType::ASTÉRIX:
            ident->set(Attr::global);
            return;
        case TokenType::DASH:
            ident->set(Attr::read_only);
            return;
        default:
            return;
        }
    }
}

Token Parser::get_token(TokenType const &t) const {
    auto tok = lexer.get_token();
    if (tok.type != t) {
        throw ParseException(lexer.get_location(), "Unexpected token: {0} - expecting {1}",
                             std::string(tok), string(t));
    }
    return tok;
}

inline const std::set<TokenType> module_ends{TokenType::END};

/**
 * @brief module -> "MODULE" IDENT ";"  declarations "BEGIN" statement_seq "END"
 * IDENT "."
 *
 * @return ASTModulePtr
 */
ASTModule Parser::parse_module() {
    auto module = makeAST<ASTModule_>(lexer);

    // MODULE ident BEGIN (expr)+ END ident.
    get_token(TokenType::MODULE);
    auto tok = get_token(TokenType::IDENT);
    module->name = tok.val;
    symbols.put(module->name, mkSym(std::make_shared<ModuleType>(module->name)));
    get_token(TokenType::SEMICOLON);

    tok = lexer.peek_token();
    if (tok.type == TokenType::IMPORT) {
        module->import = parse_import();
    }

    // Declarations
    module->decs = parse_declaration();

    // Procedures
    tok = lexer.peek_token();
    while (tok.type == TokenType::PROCEDURE) {
        lexer.get_token(); // PROCEDURE
        tok = lexer.peek_token();
        if (tok.type == TokenType::CARET) {
            module->procedures.push_back(parse_procedureForward());
        } else {
            module->procedures.push_back(parse_procedure());
        }
        tok = lexer.peek_token();
    }

    tok = lexer.peek_token();
    if (tok.type == TokenType::BEGIN) {
        get_token(TokenType::BEGIN);

        // statement_seq
        parse_statement_block(module->stats, module_ends);
    }

    // END
    get_token(TokenType::END);
    tok = get_token(TokenType::IDENT);
    if (tok.val != module->name) {
        throw ParseException(lexer.get_location(),
                             "END identifier name: {0} doesn't match module name: {1}", tok.val,
                             module->name);
    }
    get_token(TokenType::PERIOD);
    debug("parse_module finish {0}", module->name);
    return module;
}

/**
 * @brief "IMPORT" Import {"," Import} ";".
 *
 * Import = = [ident ":="] ident.
 *
 * @return ASTImportPtr
 */
ASTImport Parser::parse_import() const {
    debug("parse_import");
    auto ast = makeAST<ASTImport_>(lexer);

    lexer.get_token(); // IMPORT

    while (true) {
        auto ident = parse_identifier();

        auto tok = lexer.peek_token();
        if (tok.type == TokenType::ASSIGN) {
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
        if (tok.type != TokenType::COMMA) {
            break;
        }
        get_token(TokenType::COMMA);
    }
    get_token(TokenType::SEMICOLON);
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
    while (tok.type == TokenType::CONST || tok.type == TokenType::TYPE ||
           tok.type == TokenType::VAR) {

        switch (tok.type) {
        case TokenType::CONST: {
            auto cnsts = parse_const();
            if (!decs->cnst) {
                decs->cnst = cnsts;
            } else {
                decs->cnst->consts.insert(decs->cnst->consts.end(), cnsts->consts.begin(),
                                          cnsts->consts.end());
            }
            break;
        }
        case TokenType::TYPE: {
            auto types = parse_typedec();
            if (!decs->type) {
                decs->type = types;
            } else {
                decs->type->types.insert(decs->type->types.end(), begin(types->types),
                                         end(types->types));
            }
            break;
        }
        case TokenType::VAR: {
            const auto vars = parse_var();
            if (!decs->var) {
                decs->var = vars;
            } else {
                decs->var->vars.insert(decs->var->vars.end(), vars->vars.begin(),
                                       vars->vars.end());
            }
            break;
        }
        default:
            throw ParseException(lexer.get_location(), "unimplemented {0}", tok.val);
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
    while (tok.type == TokenType::IDENT) {
        ConstDec dec;
        dec.ident = parse_identifier();
        set_attrs(dec.ident);
        get_token(TokenType::EQUALS);
        dec.value = parse_expr();
        get_token(TokenType::SEMICOLON);

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
void Parser::parse_identList(std::vector<ASTIdentifier> &list) const {

    auto id = parse_identifier();
    set_attrs(id);
    list.push_back(id);
    auto tok = lexer.peek_token();
    while (tok.type == TokenType::COMMA) {
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
    while (tok.type == TokenType::IDENT) {
        VarDec dec;
        dec.first = parse_identifier();
        set_attrs(dec.first);
        get_token(TokenType::EQUALS);
        dec.second = parse_type();
        get_token(TokenType::SEMICOLON);

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
    while (tok.type == TokenType::IDENT) {
        std::vector<ASTIdentifier> list;
        parse_identList(list);
        get_token(TokenType::COLON);
        auto type = parse_type();
        get_token(TokenType::SEMICOLON);

        for (auto const &ident : list) {
            VarDec dec;
            dec.first = ident;
            dec.second = type;
            // delay type assignment to inspector
            symbols.put(dec.first->value, mkSym(TypeTable::VoidType));
            var->vars.push_back(dec);
        }
        tok = lexer.peek_token();
    }
    return var;
}

/**
 * @brief ( [VAR] identifier : identifier)
 *
 * @return VarDec
 */
RecVar Parser::parse_receiver() const {
    debug("parse_receiver");
    RecVar result;
    lexer.get_token(); // (

    auto tok = lexer.peek_token();
    bool var{false};
    if (tok.type == TokenType::VAR) {
        var = true;
        lexer.get_token();
    }
    result.first = parse_identifier();
    if (var) {
        result.first->set(Attr::var);
    }
    get_token(TokenType::COLON);
    result.second = parse_identifier();
    get_token(TokenType::R_PAREN);
    return result;
}

void Parser::parse_proc(ASTProc_ &proc) {
    debug("parse_proc");

    auto tok = lexer.peek_token();
    if (tok.type == TokenType::L_PAREN) {
        proc.receiver = parse_receiver();
    }

    proc.name = parse_identifier();
    set_attrs(proc.name);

    // Parameters
    tok = lexer.peek_token();
    if (tok.type == TokenType::L_PAREN) {
        parse_parameters(proc.params);
    }

    tok = lexer.peek_token();
    if (tok.type == TokenType::COLON) {
        // Do return type
        lexer.get_token();
        proc.return_type = parse_type();
    }

    get_token(TokenType::SEMICOLON);
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
    while (tok.type == TokenType::PROCEDURE) {
        lexer.get_token(); // PROCEDURE
        tok = lexer.peek_token();
        if (tok.type == TokenType::CARET) {
            proc->procedures.push_back(parse_procedureForward());
        } else {
            proc->procedures.push_back(parse_procedure());
        }
        tok = lexer.peek_token();
    }

    // statement_seq
    tok = lexer.peek_token();
    if (tok.type == TokenType::BEGIN) {
        get_token(TokenType::BEGIN);
        parse_statement_block(proc->stats, module_ends);
    }
    symbols.pop_frame();

    // END
    get_token(TokenType::END);
    tok = get_token(TokenType::IDENT);
    if (tok.val != proc->name->value) {
        throw ParseException(lexer.get_location(),
                             "END name: {0} doesn't match procedure name: {1}", tok.val,
                             proc->name->value);
    }
    get_token(TokenType::SEMICOLON);

    // Put into symbol table as a procedure
    const auto forward = std::make_shared<ProcedureFwdType>();
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
    const auto forward = std::make_shared<ProcedureFwdType>();
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
    while (tok.type != TokenType::R_PAREN) {

        auto attr = Attr::null;

        tok = lexer.peek_token();
        if (tok.type == TokenType::VAR) {
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
        while (tok.type == TokenType::COMMA) {
            lexer.get_token();
            id = parse_identifier();
            if (attr == Attr::var) {
                id->set(attr);
            }
            ids.push_back(id);
            tok = lexer.peek_token();
        }
        get_token(TokenType::COLON);
        auto type = parse_type();
        for (auto const &ident : ids) {
            params.emplace_back(ident, type);
        }
        tok = lexer.peek_token();
        if (tok.type == TokenType::SEMICOLON) {
            lexer.get_token(); // get ;
            tok = lexer.peek_token();
            continue;
        }
        if (tok.type == TokenType::R_PAREN) {
            break;
        }
        throw ParseException(lexer.get_location(), "expecting ; or ) in parameter list");
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
    case TokenType::RETURN:
        return parse_return();
    case TokenType::IF:
        return parse_if();
    case TokenType::CASE:
        return parse_case();
    case TokenType::FOR:
        return parse_for();
    case TokenType::WHILE:
        return parse_while();
    case TokenType::REPEAT:
        return parse_repeat();
    case TokenType::EXIT:
        return parse_exit();
    case TokenType::LOOP:
        return parse_loop();
    case TokenType::BEGIN:
        return parse_block();
    case TokenType::IDENT: {
        // This can be an assignment or function call

        const auto designator = parse_designator();

        tok = lexer.peek_token();
        debug("parse_statement next {0}", std::string(tok));

        if (tok.type == TokenType::L_PAREN || tok.type == TokenType::SEMICOLON ||
            tok.type == TokenType::END) {
            return parse_call(designator);
        }
        return parse_assignment(designator);
    }
    default:
        throw ParseException(lexer.get_location(), "Unexpected token: {0}", std::string(tok));
    }
}

void Parser::parse_statement_block(std::vector<ASTStatement> &stats,
                                   const std::set<TokenType> &end_tokens) {

    while (true) {
        auto tok = lexer.peek_token();
        if (tok.type == TokenType::SEMICOLON) {
            get_token(TokenType::SEMICOLON);
        } else if (end_tokens.contains(tok.type)) {
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
    get_token(TokenType::ASSIGN);
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
    get_token(TokenType::RETURN);
    auto tok = lexer.peek_token();
    if (tok.type != TokenType::SEMICOLON && tok.type != TokenType::END) {
        ret->expr = parse_expr();
    }
    return ret;
}

/**
 * @brief EXIT
 *
 * @return ASTExitPtr
 */
ASTExit Parser::parse_exit() const {
    get_token(TokenType::EXIT);
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
    if (tok.type == TokenType::L_PAREN) {
        // Parse arguments
        get_token(TokenType::L_PAREN);
        tok = lexer.peek_token();
        while (tok.type != TokenType::R_PAREN) {
            auto expr = parse_expr();
            call->args.push_back(expr);
            tok = lexer.peek_token();
            if (tok.type == TokenType::R_PAREN) {
                break;
            }
            if (tok.type == TokenType::COMMA) {
                lexer.get_token(); // get ,
                continue;
            }
            throw ParseException(lexer.get_location(), "Unexpected {0} expecting , or )", tok.val);
        }
        get_token(TokenType::R_PAREN);
    }
    return call;
}

inline const std::set<TokenType> if_ends{TokenType::END, TokenType::ELSIF, TokenType::ELSE};

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
    get_token(TokenType::IF);
    stat->if_clause.expr = parse_expr();

    // THEN
    get_token(TokenType::THEN);
    parse_statement_block(stat->if_clause.stats, if_ends);

    // ELSIF
    auto tok = lexer.peek_token();
    while (tok.type == TokenType::ELSIF) {
        debug("parse_if elsif");
        lexer.get_token();
        ASTIf_::IFClause clause;
        clause.expr = parse_expr();
        get_token(TokenType::THEN);
        parse_statement_block(clause.stats, if_ends);
        stat->elsif_clause.push_back(clause);
        tok = lexer.peek_token();
    }
    // ELSE
    if (tok.type == TokenType::ELSE) {
        debug("parse_if else\n");
        lexer.get_token();
        stat->else_clause = std::make_optional<std::vector<ASTStatement>>();
        parse_statement_block(*stat->else_clause, module_ends);
    }
    // END
    get_token(TokenType::END);
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
    if (lexer.peek_token().type == TokenType::DOTDOT) {
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

inline const std::set<TokenType> case_element_ends{TokenType::BAR, TokenType::ELSE,
                                                   TokenType::END};

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
    if (tok.type == TokenType::BAR) {
        lexer.get_token();
        tok = lexer.peek_token();
    }
    while (tok.type != TokenType::END && tok.type != TokenType::ELSE) {
        debug("parse_caseElement {0}", std::string(tok));
        auto element = makeAST<ASTCaseElement_>(lexer);

        while (tok.type != TokenType::COLON) {
            auto v = parse_caseLabel();
            element->exprs.push_back(v);

            tok = lexer.peek_token();
            if (tok.type != TokenType::COMMA) {
                break;
            }
            lexer.get_token(); // ,
        };

        get_token(TokenType::COLON);
        parse_statement_block(element->stats, case_element_ends);
        elements.push_back(element);

        tok = lexer.peek_token();
        debug("parse_caseElement {0}", std::string(tok));
        if (tok.type == TokenType::BAR) {
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
    get_token(TokenType::CASE);
    ast->expr = parse_simpleexpr();
    get_token(TokenType::OF);

    parse_caseElements(ast->elements);

    // ELSE
    auto tok = lexer.peek_token();
    debug("parse_case {0}", std::string(tok));
    if (tok.type == TokenType::ELSE) {
        lexer.get_token();
        parse_statement_block(ast->else_stats, module_ends);
    }

    // END
    get_token(TokenType::END);
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
    get_token(TokenType::FOR);
    ast->ident = parse_identifier();
    // :=
    get_token(TokenType::ASSIGN);
    ast->start = parse_expr();
    // TO
    get_token(TokenType::TO);
    ast->end = parse_expr();

    // BY
    auto tok = lexer.peek_token();
    if (tok.type == TokenType::BY) {
        lexer.get_token();
        ast->by = parse_expr();
    }

    // DO
    get_token(TokenType::DO);
    parse_statement_block(ast->stats, module_ends);
    // END
    get_token(TokenType::END);
    return ast;
}

ASTWhile Parser::parse_while() {
    auto ast = makeAST<ASTWhile_>(lexer);

    // WHILE
    get_token(TokenType::WHILE);
    ast->expr = parse_expr();

    // DO
    get_token(TokenType::DO);
    parse_statement_block(ast->stats, module_ends);
    // END
    get_token(TokenType::END);
    return ast;
}

inline const std::set<TokenType> repeat_ends{TokenType::UNTIL};

ASTRepeat Parser::parse_repeat() {
    auto ast = makeAST<ASTRepeat_>(lexer);

    // REPEAT
    get_token(TokenType::REPEAT);

    parse_statement_block(ast->stats, repeat_ends);

    // UNTIL
    get_token(TokenType::UNTIL);
    ast->expr = parse_expr();

    return ast;
}

ASTLoop Parser::parse_loop() {
    auto ast = makeAST<ASTLoop_>(lexer);

    // LOOP
    get_token(TokenType::LOOP);

    parse_statement_block(ast->stats, module_ends);

    // END
    get_token(TokenType::END);
    return ast;
}

ASTBlock Parser::parse_block() {
    auto ast = makeAST<ASTBlock_>(lexer);
    // BEGIN
    get_token(TokenType::BEGIN);

    parse_statement_block(ast->stats, module_ends);

    // END
    get_token(TokenType::END);
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
    TokenType::EQUALS,  TokenType::HASH, TokenType::LESS, TokenType::LEQ,
    TokenType::GREATER, TokenType::GTEQ, TokenType::IN};

ASTExpr Parser::parse_expr() {
    debug("parse_expr");
    auto ast = makeAST<ASTExpr_>(lexer);

    ast->expr = parse_simpleexpr();
    auto tok = lexer.peek_token();
    if (relationOps.contains(tok.type)) {
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
    if (tok.type == TokenType::PLUS || tok.type == TokenType::DASH) {
        lexer.get_token();
        expr->first_sign = std::optional<TokenType>{tok.type};
    }
    expr->term = parse_term();
    tok = lexer.peek_token();
    while (tok.type == TokenType::PLUS || tok.type == TokenType::DASH ||
           tok.type == TokenType::OR) {
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

inline const std::set<TokenType> termOps = {TokenType::ASTÉRIX, TokenType::SLASH, TokenType::DIV,
                                            TokenType::MOD, TokenType::AMPERSAND};

ASTTerm Parser::parse_term() {
    debug("parse_term");
    auto term = makeAST<ASTTerm_>(lexer);

    term->factor = parse_factor();
    auto tok = lexer.peek_token();
    debug("term {}", std::string(tok));
    while (termOps.contains(tok.type)) {
        lexer.get_token();
        ASTTerm_::Term_mult const mult{tok.type, parse_factor()};
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

    const auto tok = lexer.peek_token();
    debug("factor {0}", std::string(tok));
    switch (tok.type) {
    case TokenType::L_PAREN:
        // Expression
        lexer.get_token(); // get (
        ast->factor = parse_expr();
        get_token(TokenType::R_PAREN);
        return ast;
    case TokenType::INTEGER:
    case TokenType::HEXINTEGER: {
        ast->factor = parse_integer();
        return ast;
    }
    case TokenType::REAL:
        ast->factor = parse_real();
        return ast;
    case TokenType::HEXCHR:
    case TokenType::CHR:
        ast->factor = parse_char();
        return ast;
    case TokenType::STRING:
        ast->factor = parse_string();
        return ast;
    case TokenType::TRUE:
    case TokenType::FALSE:
        ast->factor = parse_boolean();
        return ast;
    case TokenType::NIL:
        ast->factor = parse_nil();
        return ast;
    case TokenType::L_BRACE:
        ast->factor = parse_set();
        return ast;
    case TokenType::TILDE:
        lexer.get_token(); // get ~
        ast->is_not = true;
        ast->factor = parse_factor();
        return ast;
    case TokenType::IDENT: {

        auto       d = parse_designator();
        auto       nexttok = lexer.peek_token();
        const auto res = symbols.find(std::string(*d));
        debug("parse_factor nexttok: {0} find: {1}", std::string(nexttok), std::string(*d));
        if (nexttok.type == TokenType::L_PAREN ||
            (res && res->type->id == TypeId::PROCEDURE_FWD)) {
            debug("parse_factor call: {0}");
            ast->factor = parse_call(d);
            return ast;
        }
        ast->factor = d;
        return ast;
    }
    default:
        throw ParseException(lexer.get_location(), "Unexpected token: {0}", std::string(tok));
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

inline const std::set<TokenType> designatorOps = {TokenType::L_BRACKET, TokenType::PERIOD,
                                                  TokenType::CARET};

ASTDesignator Parser::parse_designator() {
    debug("parse_designator");
    auto ast = makeAST<ASTDesignator_>(lexer);

    ast->ident = parse_qualident();

    auto tok = lexer.peek_token();
    while (designatorOps.contains(tok.type)) {
        switch (tok.type) {
        case TokenType::L_BRACKET: {
            lexer.get_token(); // [
            ArrayRef ref;
            debug("parse_designator array ref");
            do {
                ref.push_back(parse_simpleexpr());
                tok = lexer.peek_token();
                if (tok.type == TokenType::COMMA) {
                    lexer.get_token();
                    continue;
                }
            } while (tok.type != TokenType::R_BRACKET);
            get_token(TokenType::R_BRACKET);
            ast->selectors.emplace_back(ref);
            break;
        }
        case TokenType::PERIOD:
            lexer.get_token(); // .
            ast->selectors.emplace_back(FieldRef{parse_identifier(), -1});
            break;
        case TokenType::CARET:
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

    switch (lexer.peek_token().type) {
    case TokenType::L_PAREN:
        ast->type = parse_enumeration();
        return ast;
    case TokenType::ARRAY:
        ast->type = parse_array();
        return ast;
    case TokenType::RECORD:
        ast->type = parse_record();
        return ast;
    case TokenType::POINTER:
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

    get_token(TokenType::ARRAY);
    auto tok = lexer.peek_token();
    while (tok.type != TokenType::OF) {
        auto expr = parse_integer();
        ast->dimensions.push_back(expr);
        tok = lexer.peek_token();
        if (tok.type == TokenType::COMMA) {
            lexer.get_token();
            tok = lexer.peek_token();
        } else {
            break;
        }
    }
    get_token(TokenType::OF);
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

    get_token(TokenType::RECORD);
    auto tok = lexer.peek_token();
    if (tok.type == TokenType::L_PAREN) {
        lexer.get_token();
        ast->base = parse_qualident();
        get_token(TokenType::R_PAREN);
        tok = lexer.peek_token();
    }
    while (tok.type == TokenType::IDENT) {
        std::vector<ASTIdentifier> ids;
        parse_identList(ids);

        get_token(TokenType::COLON);
        auto type = parse_type();

        for (auto const &ident : ids) {
            ast->fields.emplace_back(ident, type);
        }
        tok = lexer.peek_token();
        if (tok.type == TokenType::SEMICOLON) {
            lexer.get_token();
            tok = lexer.peek_token();
            continue;
        }
    }
    get_token(TokenType::END);
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

    get_token(TokenType::POINTER);
    get_token(TokenType::TO);
    ast->reference = parse_type();
    return ast;
}

/**
 * @brief "(" ident { [","] ident } ")"
 *
 * @return ASTEnumeration
 */
ASTEnumeration Parser::parse_enumeration() {
    auto ast = makeAST<ASTEnumeration_>(lexer);

    get_token(TokenType::L_PAREN);
    auto tok = lexer.peek_token();
    if (tok.type != TokenType::IDENT) {
        throw ParseException(lexer.get_location(), "Expected identifier in enumeration");
    }

    while (true) {
        ast->values.push_back(parse_identifier());
        tok = lexer.peek_token();
        if (tok.type == TokenType::COMMA) {
            lexer.get_token();
            tok = lexer.peek_token();
        }
        if (tok.type == TokenType::R_PAREN) {
            break;
        }
        if (tok.type != TokenType::IDENT) {
            throw ParseException(lexer.get_location(), "Expected identifier in enumeration");
        }
    }
    get_token(TokenType::R_PAREN);
    return ast;
}

/**
 * @brief Qualident = [ident "."] ident.
 *
 * @return ASTIdentifierPtr
 */
ASTQualident Parser::parse_qualident() const {
    debug("parse_qualident");
    auto ast = makeAST<ASTQualident_>(lexer);

    const auto first = get_token(TokenType::IDENT);
    auto       tok = lexer.peek_token();
    if (tok.type == TokenType::PERIOD) {

        // Check if identifier is a imported module
        // If a module then make a full qualified identifier,
        // else it is a record access.

        lexer.get_token(); // .
        const auto ident = get_token(TokenType::IDENT);

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
ASTIdentifier Parser::parse_identifier() const {
    debug("parse_identifier");
    auto ast = makeAST<ASTIdentifier_>(lexer);

    const auto tok = get_token(TokenType::IDENT);
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
    while (tok.type != TokenType::R_BRACE) {
        ast->values.push_back(parse_caseLabel());
        tok = lexer.peek_token();
        if (tok.type == TokenType::R_BRACE) {
            break;
        }

        get_token(TokenType::COMMA);
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
ASTInteger Parser::parse_integer() const {
    debug("parse_integer");
    auto  ast = makeAST<ASTInteger_>(lexer);
    char *end = nullptr;

    // Either TokenType::HEXINTEGER or TokenType::INTEGER

    int        radix = dec_radix;
    const auto tok = lexer.peek_token();
    if (tok.type == TokenType::HEXINTEGER) {
        lexer.get_token();
        radix = hex_radix;
        ast->hex = true;
        ast->value = std::strtol(tok.val.c_str(), &end, radix);
        debug("parse_integer hex: {0}", ast->value);
        return ast;
    }
    if (tok.type == TokenType::INTEGER) {
        lexer.get_token();
        debug("parse_integer: {0}", std::string(tok));
        ast->value = std::strtol(tok.val.c_str(), &end, radix);
        return ast;
    }
    throw ParseException(lexer.get_location(), "Unexpected token: {0} - expecting integer",
                         std::string(tok));
}

ASTReal Parser::parse_real() const {
    debug("parse_real");
    auto ast = makeAST<ASTReal_>(lexer);

    const auto tok = lexer.get_token();
    ast->style = tok.val;
    auto str = tok.val;
    if (str.contains('D')) {
        str.replace(str.find('D'), 1, "E");
    }
    try {
        ast->value = std::stod(str);
    } catch (std::invalid_argument &) {
        throw ParseException(lexer.get_location(), "REAL number invalid argument: {0} ", tok.val);
    } catch (std::out_of_range &) {
        throw ParseException(lexer.get_location(), "REAL number out of range: {0} ", tok.val);
    }
    return ast;
}

/**
 * @brief digit {hexDigit} "X" | "’" char "’"
 *
 * @return ASTCharPtr
 */
ASTCharPtr Parser::parse_char() const {
    debug("parse_char");
    auto ast = makeAST<ASTChar_>(lexer);

    // Either TokenType::HEXCHR or TokenType::CHR

    const auto tok = lexer.get_token();
    if (tok.type == TokenType::HEXCHR) {
        char *end = nullptr;
        debug("parse_char hex {0}", tok.val);
        ast->value = static_cast<Char>(std::strtol(tok.val.c_str(), &end, hex_radix));
        ast->hex = true;
        return ast;
    }
    ast->value = tok.val_int;
    // debug("parse_char: x {0} {1}", ast->value, ast->str());
    return ast;
}

ASTString Parser::parse_string() const {
    auto ast = makeAST<ASTString_>(lexer);

    const auto tok = lexer.get_token();
    ast->delim = tok.val[0];
    ast->value = tok.val.substr(1);
    return ast;
}

/**
 * @brief TRUE | FALSE
 *
 * @return ASTBoolPtr
 */
ASTBool Parser::parse_boolean() const {
    auto       ast = makeAST<ASTBool_>(lexer);
    const auto tok = lexer.get_token();
    ast->value = (tok.type == TokenType::TRUE);
    return ast;
}

/**
 * @brief NIL
 *
 * @return ASTNilPtr
 */
ASTNil Parser::parse_nil() const {
    auto ast = makeAST<ASTNil_>(lexer);
    get_token(TokenType::NIL);
    return ast;
}

ASTModule Parser::parse() {
    return parse_module();
}

}; // namespace ax
