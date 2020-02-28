//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "parser.hh"

#include <fmt/core.h>

#include "error.hh"

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
                             lexer.lineno);
    }
    return tok;
}

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
    get_token(TokenType::semicolon);
    module->decs = parse_declaration();
    get_token(TokenType::begin);

    // statement_seq
    do {
        tok = lexer.peek_token();
        if (tok.type == TokenType::end) {
            lexer.get_token();
            break;
        }

        // Statement
        auto stat = parse_statement();
        module->stats.push_back(stat);
        // ;
        get_token(TokenType::semicolon);

        // check if anymore
        tok = lexer.peek_token();
        if (tok.type == TokenType::eof) {
            break;
        }
    } while (true);
    tok = get_token(TokenType::ident);
    if (tok.val != module->name) {
        throw ParseException(
            fmt::format("END identifier name: {} doesn't match module name: {}",
                        tok.val, module->name),
            lexer.lineno);
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
           tok.type == TokenType::var || tok.type == TokenType::procedure) {

        switch (tok.type) {
        case TokenType::cnst:
            decs->cnst = parse_const();
            break;
        case TokenType::var:
            decs->var = parse_var();
            break;
        case TokenType::procedure:
            decs->procedures.push_back(parse_procedure());
            break;
        default:
            throw ParseException(fmt::format("unimplemented {}", tok.val),
                                 lexer.lineno);
        }
        tok = lexer.peek_token();
    }
    return decs;
};

/**
 * @brief "CONST" (IDENT "=" expr ";")*
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
        dec.indent = parse_identifier();
        get_token(TokenType::equals);
        dec.expr = parse_expr();
        get_token(TokenType::semicolon);
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
        dec.indent = parse_identifier();
        get_token(TokenType::colon);
        tok = get_token(TokenType::ident);
        dec.type = tok.val;
        get_token(TokenType::semicolon);
        var->vars.push_back(dec);
        tok = lexer.peek_token();
    }
    return var;
}

/**
 * @brief "PROCEDURE" ident [formalParameters]
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

    // Parameters
    get_token(TokenType::semicolon);

    // Declarations
    proc->decs = parse_declaration();

    get_token(TokenType::begin);

    // statement_seq
    do {
        tok = lexer.peek_token();
        if (tok.type == TokenType::end) {
            lexer.get_token();
            break;
        }

        // Statement
        auto stat = parse_statement();
        proc->stats.push_back(stat);
        get_token(TokenType::semicolon);

        tok = lexer.peek_token();
    } while (true);
    tok = get_token(TokenType::ident);
    if (tok.val != proc->name) {
        throw ParseException(
            fmt::format("END name: {} doesn't match procedure name: {}",
                        tok.val, proc->name),
            lexer.lineno);
    }
    get_token(TokenType::semicolon);
    return proc;
}

/**
 * @brief assignment
    | RETURN [expr]
 *
 * @return std::shared_ptr<ASTStatement>
 */
std::shared_ptr<ASTStatement> Parser::parse_statement() {
    auto tok = lexer.peek_token();
    switch (tok.type) {
    case TokenType::ret:
        return parse_return();
    case TokenType::ident:
        return parse_assignment();
    default:
        throw ParseException(
            fmt::format("Unexpected token: {}", std::string(tok)),
            lexer.lineno);
    }
}

/**
 * @brief ident ":=" expr
 *
 * @return std::shared_ptr<ASTAssignment>
 */
std::shared_ptr<ASTAssignment> Parser::parse_assignment() {
    debug("Parser::parse_assignment");
    std::shared_ptr<ASTAssignment> assign = std::make_shared<ASTAssignment>();
    assign->indent = parse_identifier();
    get_token(TokenType::assign);
    assign->expr = parse_expr();
    return assign;
};

/**
 * @brief RETURN [expr]
 *
 * @return std::shared_ptr<ASTReturn>
 */
std::shared_ptr<ASTReturn> Parser::parse_return() {
    debug("Parser::parse_return");
    std::shared_ptr<ASTReturn> ret = std::make_shared<ASTReturn>();
    get_token(TokenType::ret);
    ret->expr = parse_expr();
    return ret;
};

/**
 * @brief expr -> ('+' | '-' )? term ( ('+' | '-' ) term)*
 *
 * @return std::shared_ptr<ASTExpr>
 */
std::shared_ptr<ASTExpr> Parser::parse_expr() {
    debug("Parser::parse_expr");
    std::shared_ptr<ASTExpr> expr = std::make_shared<ASTExpr>();

    auto tok = lexer.peek_token();
    debug("expr: {}", std::string(tok));
    if (tok.type == TokenType::plus || tok.type == TokenType::dash) {
        lexer.get_token();
        expr->first_sign = std::optional<TokenType>{tok.type};
    }
    expr->term = parse_term();
    tok = lexer.peek_token();
    while (tok.type == TokenType::plus || tok.type == TokenType::dash) {
        lexer.get_token();
        Expr_add add{tok.type};
        add.term = parse_term();
        expr->rest.push_back(add);
        tok = lexer.peek_token();
    }
    return expr;
}

/**
 * @brief term -> INTEGER ( ( '*' | 'DIV' | 'MOD' ) INTEGER)*
 *
 * @return std::shared_ptr<ASTTerm>
 */
std::shared_ptr<ASTTerm> Parser::parse_term() {
    debug("Parser::parse_term");
    std::shared_ptr<ASTTerm> term = std::make_shared<ASTTerm>();

    term->factor = parse_factor();
    auto tok = lexer.peek_token();
    while (tok.type == TokenType::asterisk || tok.type == TokenType::div ||
           tok.type == TokenType::mod) {
        lexer.get_token();
        Term_mult mult{tok.type};
        mult.factor = parse_factor();
        term->rest.push_back(mult);
        tok = lexer.peek_token();
    }
    return term;
}

/**
 * @brief factor -> IDENT | INTEGER | '(' expr ')'
 *
 * @return std::shared_ptr<ASTFactor>
 */
std::shared_ptr<ASTFactor> Parser::parse_factor() {
    debug("Parser::parse_factor");
    std::shared_ptr<ASTFactor> factor = std::make_shared<ASTFactor>();
    auto                       tok = lexer.peek_token();
    debug("factor: {}\n", std::string(tok));
    switch (tok.type) {
    case TokenType::l_paren:
        lexer.get_token(); // get (
        factor->expr = parse_expr();
        get_token(TokenType::r_paren);
        return factor;
    case TokenType::integer:
        factor->integer = parse_integer();
        return factor;
    case TokenType::ident:
        factor->identifier = parse_identifier();
        return factor;
    default:
        throw ParseException(
            fmt::format("Unexpected token: {} - expecting ( or integer",
                        std::string(tok)),
            lexer.lineno);
    }
    return nullptr; // Not factor
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

std::shared_ptr<ASTModule> Parser::parse() {
    return parse_module();
}

}; // namespace ax
