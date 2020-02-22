//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "parser.hh"

#include <fmt/core.h>

#include "error.hh"

namespace ax {

std::shared_ptr<ASTModule> Parser::parse_module() {
    std::shared_ptr<ASTModule> module = std::make_shared<ASTModule>();

    do {
        // Expr
        auto expr = parse_expr();
        module->exprs.push_back(expr);

        // ;
        auto tok = lexer.get_token();
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
    return module;
}

std::shared_ptr<ASTExpr> Parser::parse_expr() {
    std::shared_ptr<ASTExpr> expr = std::make_shared<ASTExpr>();
    expr->integer = parse_integer();
    return expr;
}

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
