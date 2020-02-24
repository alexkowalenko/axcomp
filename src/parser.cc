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
                                         std::string(tok), to_string(t)),
                             lexer.lineno);
    }
    return tok;
}

/**
 * @brief module -> "MODULE" IDENT ";" "BEGIN" statement_seq "END" IDENT "."
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
    get_token(TokenType::begin);

    // statement_seq
    do {
        tok = lexer.peek_token();
        if (tok.type == TokenType::end) {
            lexer.get_token();
            break;
        }

        // Expr
        auto expr = parse_expr();
        module->exprs.push_back(expr);

        // ;
        tok = lexer.get_token();
        if (tok.type == TokenType::eof) {
            return module;
        }
        if (tok.type != TokenType::semicolon) {
            throw ParseException(
                fmt::format("Unexpected token <{}> : {}", tok.val, tok.type),
                lexer.lineno);
        }

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
 * @brief factor -> INTEGER | '(' expr ')'
 *
 * @return std::shared_ptr<ASTFactor>
 */
std::shared_ptr<ASTFactor> Parser::parse_factor() {
    debug("Parser::parse_factor");
    std::shared_ptr<ASTFactor> factor = std::make_shared<ASTFactor>();
    auto                       tok = lexer.peek_token();
    debug("factor: {}\n", std::string(tok));
    if (tok.type == TokenType::l_paren) {
        lexer.get_token(); // get (
        factor->expr = parse_expr();
        get_token(TokenType::r_paren);
        factor->integer = nullptr;
        return factor;
    } else if (tok.type == TokenType::integer) {
        factor->integer = parse_integer();
        factor->expr = nullptr;
        return factor;
    }
    debug("not factor: {}", to_string(tok.type));
    throw ParseException(
        fmt::format("Unexpected token: {} - expecting ( or integer",
                    std::string(tok)),
        lexer.lineno);
    return nullptr; // Not factor
};

/**
 * @brief INTEGER
 *
 * @return std::shared_ptr<ASTInteger>
 */
std::shared_ptr<ASTInteger> Parser::parse_integer() {
    debug("Parser::parse_integer");
    auto tok = lexer.get_token();
    if (tok.type != TokenType::integer) {
        throw ParseException("Expecting an integer: " + tok.val, lexer.lineno);
    }
    std::shared_ptr<ASTInteger> ast = std::make_shared<ASTInteger>();
    ast->value = atol(tok.val.c_str());
    return ast;
}

std::shared_ptr<ASTModule> Parser::parse() {
    return parse_module();
}

}; // namespace ax
