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

        // Expr
        auto expr = parse_expr();
        module->exprs.push_back(expr);

        // ;
        tok = lexer.get_token();
        if (tok.type == TokenType::eof) {
            return module;
        }
        if (tok.type != TokenType::semicolon) {
            ParseException(fmt::format("Unexpected token: {} - expecting ;",
                                       std::string(tok)),
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
 * @brief declarations -> ("CONST" (IDENT "=" expr ";")* )?
                ("TYPE" (IDENT "=" type ";")* )?
                ("VAR" (IDENT ":" type ";")* )?
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
        default:
            throw ParseException(fmt::format("unimplemented {}", tok.val),
                                 lexer.lineno);
        }
        tok = lexer.peek_token();
    }
    return decs;
};

/**
 * @brief ("CONST" (IDENT "=" expr ";")* )?
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
    auto                        tok = get_token(TokenType::integer);
    std::shared_ptr<ASTInteger> ast = std::make_shared<ASTInteger>();
    ast->value = atol(tok.val.c_str());
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
