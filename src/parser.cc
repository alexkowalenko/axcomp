//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "parser.hh"

#include <fmt/core.h>

#include "error.hh"

namespace ax {

ASTModule *Parser::parse_module() {
    ASTModule *module = new ASTModule();

    do {
        // Expr
        ASTExpr *expr = parse_expr();
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

ASTExpr *Parser::parse_expr() {
    ASTExpr *expr = new ASTExpr();
    expr->integer = parse_integer();
    return expr;
}

ASTInteger *Parser::parse_integer() {
    auto tok = lexer.get_token();
    if (tok.type != TokenType::integer) {
        throw ParseException("Expecting an integer: " + tok.val, lexer.lineno);
    }
    ASTInteger *ast = new ASTInteger();
    ast->value = atol(tok.val.c_str());
    return ast;
}

ASTModule *Parser::parse() {
    return parse_module();
}

}; // namespace ax
