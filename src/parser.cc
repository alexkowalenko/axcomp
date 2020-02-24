//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "parser.hh"

#include <fmt/core.h>

#include "error.hh"

namespace ax {

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
    std::shared_ptr<ASTExpr> expr = std::make_shared<ASTExpr>();

    auto tok = lexer.peek_token();
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
    std::shared_ptr<ASTTerm> term = std::make_shared<ASTTerm>();

    term->integer = parse_integer();
    auto tok = lexer.peek_token();
    while (tok.type == TokenType::asterisk || tok.type == TokenType::div ||
           tok.type == TokenType::mod) {
        lexer.get_token();
        Term_mult mult{tok.type};
        mult.integer = parse_integer();
        term->rest.push_back(mult);
        tok = lexer.peek_token();
    }
    return term;
}

/**
 * @brief INTEGER
 *
 * @return std::shared_ptr<ASTInteger>
 */
std::shared_ptr<ASTInteger> Parser::parse_integer() {
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
