//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "parser.hh"

#include <optional>
#include <set>

#include <fmt/core.h>

#include "error.hh"
#include "typetable.hh"

namespace ax {

inline constexpr bool debug_parser{false};

template <typename... T> inline void debug(const T &... msg) {
    if constexpr (debug_parser) {
        std::cerr << fmt::format(msg...) << std::endl;
    }
}

Token Parser::get_token(TokenType t) {
    auto tok = lexer.get_token();
    if (tok.type != t) {
        throw ParseException(fmt::format("Unexpected token: {} - expecting {}",
                                         std::string(tok), string(t)),
                             lexer.get_lineno());
    }
    return tok;
}

inline std::set<TokenType> module_ends{TokenType::end};

/**
 * @brief module -> "MODULE" IDENT ";"  declarations "BEGIN" statement_seq "END"
 * IDENT "."
 *
 * @return std::shared_ptr<ASTModule>
 */
std::shared_ptr<ASTModule> Parser::parse_module() {
    std::shared_ptr<ASTModule> module = std::make_shared<ASTModule>();

    // MODULE ident BEGIN (expr)+ END ident.
    get_token(TokenType::module);
    auto tok = get_token(TokenType::ident);
    module->name = tok.val;
    symbols->put(
        module->name,
        Symbol(module->name, std::string(*TypeTable::ModuleType.get())));
    get_token(TokenType::semicolon);
    module->decs = parse_declaration();

    // Procedures
    tok = lexer.peek_token();
    while (tok.type == TokenType::procedure) {
        module->procedures.push_back(parse_procedure());
        tok = lexer.peek_token();
    }

    get_token(TokenType::begin);

    // statement_seq
    parse_statement_block(module->stats, module_ends);

    // END
    get_token(TokenType::end);
    tok = get_token(TokenType::ident);
    if (tok.val != module->name) {
        throw ParseException(
            fmt::format("END identifier name: {} doesn't match module name: {}",
                        tok.val, module->name),
            lexer.get_lineno());
    }
    get_token(TokenType::period);
    return module;
}

/**
 * @brief declarations -> ["CONST" (IDENT "=" expr ";")* ]
                ["TYPE" (IDENT "=" type ";")* ]
                ["VAR" (IDENT ":" type ";")* ]
                ( procedureDeclaration ";")*
 *
 * @return std::shared_ptr<ASTDeclaration>
 */
std::shared_ptr<ASTDeclaration> Parser::parse_declaration() {
    debug("Parser::parse_declaration");
    std::shared_ptr<ASTDeclaration> decs = std::make_shared<ASTDeclaration>();
    auto                            tok = lexer.peek_token();
    while (tok.type == TokenType::cnst || tok.type == TokenType::type ||
           tok.type == TokenType::var) {

        switch (tok.type) {
        case TokenType::cnst:
            decs->cnst = parse_const();
            break;
        case TokenType::var:
            decs->var = parse_var();
            break;
        default:
            throw ParseException(fmt::format("unimplemented {}", tok.val),
                                 lexer.get_lineno());
        }
        tok = lexer.peek_token();
    }
    return decs;
};

/**
 * @brief "CONST" (IDENT "=" INTEGER ";")*
 *
 * @return std::shared_ptr<ASTConst>
 */
std::shared_ptr<ASTConst> Parser::parse_const() {
    debug("Parser::parse_const");
    lexer.get_token(); // CONST
    std::shared_ptr<ASTConst> cnst = std::make_shared<ASTConst>();
    auto                      tok = lexer.peek_token();
    while (tok.type == TokenType::ident) {
        ConstDec dec;
        dec.ident = parse_identifier();
        get_token(TokenType::equals);
        dec.value = parse_integer();
        get_token(TokenType::semicolon);

        // Assume all consts are INTEGER;
        symbols->put(
            dec.ident->value,
            Symbol(dec.ident->value, std::string(*TypeTable::IntType.get())));
        cnst->consts.push_back(dec);
        tok = lexer.peek_token();
    }
    return cnst;
}

/**
 * @brief  "VAR" (IDENT ":" type ";")*
 *
 * @return std::shared_ptr<ASTVar>
 */
std::shared_ptr<ASTVar> Parser::parse_var() {
    debug("Parser::parse_var");
    lexer.get_token(); // VAR
    std::shared_ptr<ASTVar> var = std::make_shared<ASTVar>();
    auto                    tok = lexer.peek_token();
    while (tok.type == TokenType::ident) {
        VarDec dec;
        dec.first = parse_identifier();
        get_token(TokenType::colon);
        tok = get_token(TokenType::ident);
        dec.second = tok.val;
        get_token(TokenType::semicolon);

        symbols->put(dec.first->value, Symbol(dec.first->value, dec.second));

        var->vars.push_back(dec);
        tok = lexer.peek_token();
    }
    return var;
}

/**
 * @brief "PROCEDURE" ident [formalParameters] [ ":" IDENT ]
 *         declarations ["BEGIN" statement_seq] "END" ident ";"
 *
 * @return std::shared_ptr<ASTProcedure>
 */
std::shared_ptr<ASTProcedure> Parser::parse_procedure() {
    debug("Parser::parse_procedure");
    std::shared_ptr<ASTProcedure> proc = std::make_shared<ASTProcedure>();

    lexer.get_token(); // PROCEDURE
    auto tok = get_token(TokenType::ident);
    proc->name = tok.val;
    symbols->put(proc->name, Symbol(proc->name, "PROCEDURE"));

    // Parameters
    tok = lexer.peek_token();
    if (tok.type == TokenType::l_paren) {
        parse_parameters(proc->params);
    }

    tok = lexer.peek_token();
    if (tok.type == TokenType::colon) {
        // Do return
        lexer.get_token();
        tok = get_token(TokenType::ident);
        proc->return_type = tok.val;
    }

    get_token(TokenType::semicolon);

    // Declarations
    proc->decs = parse_declaration();

    get_token(TokenType::begin);

    // statement_seq
    parse_statement_block(proc->stats, module_ends);

    // END
    get_token(TokenType::end);
    tok = get_token(TokenType::ident);
    if (tok.val != proc->name) {
        throw ParseException(
            fmt::format("END name: {} doesn't match procedure name: {}",
                        tok.val, proc->name),
            lexer.get_lineno());
    }
    get_token(TokenType::semicolon);
    return proc;
}

/**
 * @brief formalParameters
 * = "(" [(identList : INDENT)* (";"  (identList : INDENT)*)] "")"
 *
 * @return std::vector<VarDec>
 */
void Parser::parse_parameters(std::vector<VarDec> &params) {
    lexer.get_token(); // get (
    auto tok = lexer.peek_token();
    while (tok.type != TokenType::r_paren) {
        VarDec dec;
        dec.first = parse_identifier();
        get_token(TokenType::colon);
        dec.second = parse_identifier()->value;
        params.push_back(dec);
        tok = lexer.peek_token();
        if (tok.type == TokenType::semicolon) {
            lexer.get_token(); // get ;
            tok = lexer.peek_token();
            continue;
        }
        if (tok.type == TokenType::r_paren) {
            break;
        }
        throw ParseException("expecting ; or ) in parameter list",
                             lexer.get_lineno());
    }
    lexer.get_token(); // get )
}

/**
 * @brief assignment
    | procedureCall
    | ifStatment
    | forStatement
    | "RETURN" [expr]
 *
 * @return std::shared_ptr<ASTStatement>
 */
std::shared_ptr<ASTStatement> Parser::parse_statement() {
    auto tok = lexer.peek_token();
    debug("Parser::parse_statement {}", std::string(tok));
    switch (tok.type) {
    case TokenType::ret:
        return parse_return();
    case TokenType::if_k:
        return parse_if();
    case TokenType::for_k:
        return parse_for();
    case TokenType::ident: {
        // This can be an assignment or function call
        auto ident = lexer.get_token();
        tok = lexer.peek_token();
        debug("Parser::parse_statement next {}", std::string(tok));
        if (tok.type == TokenType::assign) {
            return parse_assignment(ident);
        }
        if (tok.type == TokenType::l_paren) {
            return parse_call(ident);
        }
        [[fallthrough]];
    }
    default:
        throw ParseException(
            fmt::format("Unexpected token: {}", std::string(tok)),
            lexer.get_lineno());
    }
}

void Parser::parse_statement_block(
    std::vector<std::shared_ptr<ASTStatement>> &stats,
    std::set<TokenType>                         end_tokens) {
    auto tok = lexer.peek_token();
    while (true) {
        if (end_tokens.find(tok.type) != end_tokens.end()) {
            return;
        }

        // Statement
        auto s = parse_statement();
        stats.push_back(s);
        get_token(TokenType::semicolon);

        tok = lexer.peek_token();
    }
}

/**
 * @brief ident ":=" expr
 *
 * @return std::shared_ptr<ASTAssignment>
 */
std::shared_ptr<ASTAssignment> Parser::parse_assignment(Token const &ident) {
    debug("Parser::parse_assignment");
    std::shared_ptr<ASTAssignment> assign = std::make_shared<ASTAssignment>();
    assign->ident = std::make_shared<ASTIdentifier>();
    assign->ident->value = ident.val;
    get_token(TokenType::assign);
    assign->expr = parse_expr();
    return assign;
}

/**
 * @brief RETURN [expr]
 *
 * @return std::shared_ptr<ASTReturn>
 */
std::shared_ptr<ASTReturn> Parser::parse_return() {
    debug("Parser::parse_return");
    std::shared_ptr<ASTReturn> ret = std::make_shared<ASTReturn>();
    get_token(TokenType::ret);
    auto tok = lexer.peek_token();
    if (tok.type != TokenType::semicolon) {
        ret->expr = parse_expr();
    }
    return ret;
}

/**
 * @brief  IDENT "(" expr ( "," expr )* ")"
 *
 * @return std::shared_ptr<ASTCall>
 */
std::shared_ptr<ASTCall> Parser::parse_call(Token const &ident) {
    std::shared_ptr<ASTCall> call = std::make_shared<ASTCall>();
    call->name = std::make_shared<ASTIdentifier>();
    call->name->value = ident.val;
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
        throw ParseException(
            fmt::format("Unexpected {} expecting , or )", tok.val),
            lexer.get_lineno());
    }
    get_token(TokenType::r_paren);
    return call;
}

inline std::set<TokenType> if_ends{TokenType::end, TokenType::elsif,
                                   TokenType::else_k};

/**
 * @brief "IF" expression "THEN" statement_seq
    ( "ELSIF" expression "THEN" statement_seq )*
    [ "ELSE" statement_seq "END" ]
 *
 * @return std::shared_ptr<ASTIf>
 */
std::shared_ptr<ASTIf> Parser::parse_if() {
    debug("Parser::parse_if");
    std::shared_ptr<ASTIf> stat = std::make_shared<ASTIf>();

    // IF
    get_token(TokenType::if_k);
    stat->if_clause.expr = parse_expr();

    // THEN
    get_token(TokenType::then);
    parse_statement_block(stat->if_clause.stats, if_ends);

    // ELSIF
    auto tok = lexer.peek_token();
    while (tok.type == TokenType::elsif) {
        debug("Parser::parse_if elsif");
        lexer.get_token();
        ASTIf::IFClause clause;
        clause.expr = parse_expr();
        get_token(TokenType::then);
        parse_statement_block(clause.stats, if_ends);
        stat->elsif_clause.push_back(clause);
        tok = lexer.peek_token();
    }
    // ELSE
    if (tok.type == TokenType::else_k) {
        debug("Parser::parse_if else\n");
        lexer.get_token();
        stat->else_clause =
            std::make_optional<std::vector<std::shared_ptr<ASTStatement>>>();
        parse_statement_block(*stat->else_clause, module_ends);
    }
    // END
    get_token(TokenType::end);
    return stat;
};

/**
 * @brief "FOR" IDENT ":=" expr "TO" expr [ "BY" INTEGER ] "DO"
    statement_seq "END"
 *
 * @return std::shared_ptr<ASTFor>
 */
std::shared_ptr<ASTFor> Parser::parse_for() {
    debug("Parser::parse_for");
    std::shared_ptr<ASTFor> ast = std::make_shared<ASTFor>();

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
        auto incr = parse_integer();
        ast->by = incr->value;
    }

    // DO
    get_token(TokenType::do_k);
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
 * @return std::shared_ptr<ASTExpr>
 */

inline std::set<TokenType> relationOps = {TokenType::equals,  TokenType::hash,
                                          TokenType::less,    TokenType::leq,
                                          TokenType::greater, TokenType::gteq};

std::shared_ptr<ASTExpr> Parser::parse_expr() {
    debug("Parser::parse_expr");
    std::shared_ptr<ASTExpr> ast = std::make_shared<ASTExpr>();

    ast->expr = parse_simpleexpr();
    auto tok = lexer.peek_token();
    if (relationOps.find(tok.type) != relationOps.end()) {
        lexer.get_token(); // get token;
        ast->relation = std::optional<TokenType>(tok.type);
        ast->relation_expr =
            std::optional<std::shared_ptr<ASTSimpleExpr>>(parse_simpleexpr());
    }
    return ast;
}

/**
 * @brief expr -> ('+' | '-' )? term ( ('+' | '-' | "OR" ) term)*
 *
 * @return std::shared_ptr<ASTSimpleExpr>
 */
std::shared_ptr<ASTSimpleExpr> Parser::parse_simpleexpr() {
    debug("Parser::parse_simpleexpr");
    std::shared_ptr<ASTSimpleExpr> expr = std::make_shared<ASTSimpleExpr>();

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
        ASTSimpleExpr::Expr_add add{tok.type, parse_term()};
        expr->rest.push_back(add);
        tok = lexer.peek_token();
    }
    return expr;
}

/**
 * @brief term -> INTEGER ( ( '*' | 'DIV' | 'MOD' | "&") INTEGER)*
 *
 * @return std::shared_ptr<ASTTerm>
 */

inline std::set<TokenType> termOps = {TokenType::asterisk, TokenType::div,
                                      TokenType::mod, TokenType::ampersand};

std::shared_ptr<ASTTerm> Parser::parse_term() {
    debug("Parser::parse_term");
    std::shared_ptr<ASTTerm> term = std::make_shared<ASTTerm>();

    term->factor = parse_factor();
    auto tok = lexer.peek_token();
    debug("term {}", std::string(tok));
    while (termOps.find(tok.type) != termOps.end()) {
        lexer.get_token();
        ASTTerm::Term_mult mult{tok.type, parse_factor()};
        term->rest.push_back(mult);
        tok = lexer.peek_token();
        debug("term {}", std::string(tok));
    }
    return term;
}

/**
 * @brief factor -> IDENT
 *                  | procedureCall
 *                  | INTEGER
 *                  | "TRUE" | "FALSE"
 *                  | '('expr ')'
 *
 * @return std::shared_ptr<ASTFactor>
 */
std::shared_ptr<ASTFactor> Parser::parse_factor() {
    debug("Parser::parse_factor");
    std::shared_ptr<ASTFactor> ast = std::make_shared<ASTFactor>();
    auto                       tok = lexer.peek_token();
    debug("factor {}", std::string(tok));
    switch (tok.type) {
    case TokenType::l_paren:
        // Expression
        lexer.get_token(); // get (
        ast->factor = parse_expr();
        get_token(TokenType::r_paren);
        return ast;
    case TokenType::integer:
        // Integer
        ast->factor = parse_integer();
        return ast;
    case TokenType::true_k:
    case TokenType::false_k:
        ast->factor = parse_boolean();
        return ast;
    case TokenType::tilde:
        lexer.get_token(); // get ~
        ast->is_not = true;
        ast->factor = parse_factor();
        return ast;
    case TokenType::ident: {
        // Identifier
        lexer.get_token(); // get tok
        auto nexttok = lexer.peek_token();
        debug("factor nexttok: {}", std::string(nexttok));
        if (nexttok.type == TokenType::l_paren) {
            ast->factor = parse_call(tok);
            return ast;
        }
        lexer.push_token(tok);
        ast->factor = parse_identifier();
        return ast;
    }
    default:
        throw ParseException(
            fmt::format("Unexpected token: {}", std::string(tok)),
            lexer.get_lineno());
    }
    return nullptr; // Not factor
};

/**
 * @brief IDENT
 *
 * @return std::shared_ptr<ASTIdentifier>
 */
std::shared_ptr<ASTIdentifier> Parser::parse_identifier() {
    debug("Parser::parse_identifier");
    auto                           tok = get_token(TokenType::ident);
    std::shared_ptr<ASTIdentifier> ast = std::make_shared<ASTIdentifier>();
    ast->value = tok.val;
    return ast;
};

/**
 * @brief INTEGER
 *
 * @return std::shared_ptr<ASTInteger>
 */
std::shared_ptr<ASTInteger> Parser::parse_integer() {
    debug("Parser::parse_integer");
    auto                        tok = get_token(TokenType::integer);
    std::shared_ptr<ASTInteger> ast = std::make_shared<ASTInteger>();
    char *                      end;
    ast->value = std::strtol(tok.val.c_str(), &end, 10);
    return ast;
}

/**
 * @brief TRUE | FALSE
 *
 * @return std::shared_ptr<ASTBool>
 */
std::shared_ptr<ASTBool> Parser::parse_boolean() {
    std::shared_ptr<ASTBool> ast = std::make_shared<ASTBool>();
    auto                     tok = lexer.get_token();
    ast->value = (tok.type == TokenType::true_k);
    return ast;
}

std::shared_ptr<ASTModule> Parser::parse() {
    return parse_module();
}
}; // namespace ax
