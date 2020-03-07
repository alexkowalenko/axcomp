//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "parser.hh"

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
    tok = lexer.peek_token();
    do {
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
    | RETURN [expr]
 *
 * @return std::shared_ptr<ASTStatement>
 */
std::shared_ptr<ASTStatement> Parser::parse_statement() {
    auto tok = lexer.peek_token();
    debug("Parser::parse_statement {}", std::string(tok));
    switch (tok.type) {
    case TokenType::ret:
        return parse_return();
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

/**
 * @brief expr -> ('+' | '-' )? term ( ('+' | '-' ) term)*
 *
 * @return std::shared_ptr<ASTExpr>
 */
std::shared_ptr<ASTExpr> Parser::parse_expr() {
    debug("Parser::parse_expr");
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
        ASTExpr::Expr_add add{tok.type, parse_term()};
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
    debug("term {}", std::string(tok));
    while (tok.type == TokenType::asterisk || tok.type == TokenType::div ||
           tok.type == TokenType::mod) {
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
    std::shared_ptr<ASTFactor> factor = std::make_shared<ASTFactor>();
    auto                       tok = lexer.peek_token();
    debug("factor {}", std::string(tok));
    switch (tok.type) {
    case TokenType::l_paren:
        // Expression
        lexer.get_token(); // get (
        factor->factor = parse_expr();
        get_token(TokenType::r_paren);
        return factor;
    case TokenType::integer:
        // Integer
        factor->factor = parse_integer();
        return factor;
    case TokenType::true_k:
    case TokenType::false_k:
        factor->factor = parse_boolean();
        return factor;
    case TokenType::ident: {
        // Identifier
        lexer.get_token(); // get tok
        auto nexttok = lexer.peek_token();
        debug("factor nexttok: {}", std::string(nexttok));
        if (nexttok.type == TokenType::l_paren) {
            factor->factor = parse_call(tok);
            return factor;
        }
        lexer.push_token(tok);
        factor->factor = parse_identifier();
        return factor;
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
    if (auto tok = lexer.get_token(); tok.type == TokenType::true_k) {
        ast->value = true;
    } else {
        ast->value = false;
    }
    return ast;
}

std::shared_ptr<ASTModule> Parser::parse() {
    return parse_module();
}

}; // namespace ax
