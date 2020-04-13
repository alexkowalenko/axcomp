//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "parser.hh"

#include <cstddef>
#include <optional>

#include <llvm/Support/FormatVariadic.h>

#include "ast.hh"
#include "error.hh"
#include "token.hh"
#include "type.hh"
#include "typetable.hh"

namespace ax {

inline constexpr bool debug_parser{false};

template <typename... T> inline void debug(const T &... msg) {
    if constexpr (debug_parser) {
        std::cerr << std::string(llvm::formatv(msg...)) << std::endl;
    }
}

// builtin procedures
std::vector<std::pair<std::string, std::shared_ptr<ProcedureType>>> builtins;

// module identifier markers
inline const std::set<TokenType> module_markers{TokenType::asterisk,
                                                TokenType::dash};

void Parser::set_attrs(std::shared_ptr<ASTIdentifier> const &ident) {
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
            llvm::formatv("Unexpected token: {0} - expecting {1}",
                          std::string(tok), string(t)),
            lexer.get_location());
    }
    return tok;
}

inline const std::set<TokenType> module_ends{TokenType::end};

/**
 * @brief module -> "MODULE" IDENT ";"  declarations "BEGIN" statement_seq "END"
 * IDENT "."
 *
 * @return std::shared_ptr<ASTModule>
 */
std::shared_ptr<ASTModule> Parser::parse_module() {
    auto module = makeAST<ASTModule>(lexer);

    // MODULE ident BEGIN (expr)+ END ident.
    get_token(TokenType::module);
    auto tok = get_token(TokenType::ident);
    module->name = tok.val;
    symbols->put(module->name,
                 {std::make_shared<ModuleType>(module->name), Attr::null});
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
            llvm::formatv(
                "END identifier name: {0} doesn't match module name: {1}",
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
 * @return std::shared_ptr<ASTImport>
 */
std::shared_ptr<ASTImport> Parser::parse_import() {
    debug("Parser::parse_import");
    auto ast = makeAST<ASTImport>(lexer);

    lexer.get_token(); // IMPORT

    while (true) {
        auto ident = parse_identifier();

        auto tok = lexer.peek_token();
        if (tok.type == TokenType::assign) {
            lexer.get_token(); // :=
            auto second = parse_identifier();
            ast->imports.emplace_back(ASTImport::Pair{second, ident});
            auto module = std::make_shared<ModuleType>(second->value);
            symbols->put(ident->value, {module, Attr::null});
        } else {
            ast->imports.emplace_back(ASTImport::Pair{ident, nullptr});
            auto module = std::make_shared<ModuleType>(ident->value);
            symbols->put(ident->value, {module, Attr::null});
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
 * @return std::shared_ptr<ASTDeclaration>
 */
std::shared_ptr<ASTDeclaration> Parser::parse_declaration() {
    debug("Parser::parse_declaration");
    auto decs = makeAST<ASTDeclaration>(lexer);

    auto tok = lexer.peek_token();
    while (tok.type == TokenType::cnst || tok.type == TokenType::type ||
           tok.type == TokenType::var) {

        switch (tok.type) {
        case TokenType::cnst: {
            auto cnsts = parse_const();
            if (!decs->cnst) {
                decs->cnst = cnsts;
            } else {
                decs->cnst->consts.insert(decs->cnst->consts.end(),
                                          cnsts->consts.begin(),
                                          cnsts->consts.end());
            }
            break;
        }
        case TokenType::type: {
            auto types = parse_typedec();
            if (!decs->type) {
                decs->type = types;
            } else {
                decs->type->types.insert(decs->var->vars.end(),
                                         begin(types->types),
                                         end(types->types));
            }
            break;
        }
        case TokenType::var: {
            auto vars = parse_var();
            if (!decs->var) {
                decs->var = vars;
            } else {
                decs->var->vars.insert(decs->var->vars.end(),
                                       vars->vars.begin(), vars->vars.end());
            }
            break;
        }
        default:
            throw ParseException(llvm::formatv("unimplemented {0}", tok.val),
                                 lexer.get_location());
        }
        tok = lexer.peek_token();
    }
    return decs;
};

/**
 * @brief "CONST" (IdentDef "=" INTEGER ";")*
 *
 * @return std::shared_ptr<ASTConst>
 */
std::shared_ptr<ASTConst> Parser::parse_const() {
    debug("Parser::parse_const");
    auto cnst = makeAST<ASTConst>(lexer);

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
        symbols->put(dec.ident->value, {TypeTable::VoidType, Attr::cnst});
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
void Parser::parse_identList(
    std::vector<std::shared_ptr<ASTIdentifier>> &list) {

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
 * @return std::shared_ptr<ASTTypeDec>
 */
std::shared_ptr<ASTTypeDec> Parser::parse_typedec() {
    debug("Parser::parse_typedec");
    auto type = makeAST<ASTTypeDec>(lexer);

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
 * @return std::shared_ptr<ASTVar>
 */
std::shared_ptr<ASTVar> Parser::parse_var() {
    debug("Parser::parse_var");
    auto var = makeAST<ASTVar>(lexer);

    lexer.get_token(); // VAR
    auto tok = lexer.peek_token();
    while (tok.type == TokenType::ident) {
        std::vector<std::shared_ptr<ASTIdentifier>> list;
        parse_identList(list);
        get_token(TokenType::colon);
        auto type = parse_type();
        get_token(TokenType::semicolon);

        std::for_each(begin(list), end(list), [=](auto const &i) {
            VarDec dec;
            dec.first = i;
            dec.second = type;
            // delay type assignment to inspector
            symbols->put(dec.first->value, {TypeTable::VoidType, Attr::null});
            var->vars.push_back(dec);
        });
        tok = lexer.peek_token();
    }
    return var;
}

/**
 * @brief "PROCEDURE" IdentDef [formalParameters] [ ":" type ]
 *         declarations ["BEGIN" statement_seq] "END" ident ";"
 *
 * @return std::shared_ptr<ASTProcedure>
 */
std::shared_ptr<ASTProcedure> Parser::parse_procedure() {
    debug("Parser::parse_procedure");
    auto proc = makeAST<ASTProcedure>(lexer);

    lexer.get_token(); // PROCEDURE
    proc->name = parse_identifier();
    set_attrs(proc->name);

    // Parameters
    auto tok = lexer.peek_token();
    if (tok.type == TokenType::l_paren) {
        parse_parameters(proc->params);
    }

    tok = lexer.peek_token();
    if (tok.type == TokenType::colon) {
        // Do return type
        lexer.get_token();
        proc->return_type = parse_type();
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
    if (tok.val != proc->name->value) {
        throw ParseException(
            llvm::formatv("END name: {0} doesn't match procedure name: {1}",
                          tok.val, proc->name->value),
            lexer.get_location());
    }
    get_token(TokenType::semicolon);
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
        std::vector<std::shared_ptr<ASTIdentifier>> ids;
        auto                                        id = parse_identifier();
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
        std::for_each(begin(ids), end(ids), [&params, type](auto const &id) {
            params.push_back(VarDec{id, type});
        });
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
                             lexer.get_location());
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
        auto ident = lexer.get_token();
        tok = lexer.peek_token();
        debug("Parser::parse_statement next {0}", std::string(tok));
        lexer.push_token(ident);
        if (tok.type == TokenType::l_paren ||
            tok.type == TokenType::semicolon || tok.type == TokenType::end) {
            return parse_call();
        }
        return parse_assignment();
    }
    default:
        throw ParseException(
            llvm::formatv("Unexpected token: {0}", std::string(tok)),
            lexer.get_location());
    }
}

void Parser::parse_statement_block(
    std::vector<std::shared_ptr<ASTStatement>> &stats,
    const std::set<TokenType> &                 end_tokens) {

    auto s = parse_statement();
    stats.push_back(s);
    auto tok = lexer.peek_token();
    while (true) {
        if (tok.type == TokenType::semicolon) {
            get_token(TokenType::semicolon);
        } else if (end_tokens.find(tok.type) != end_tokens.end()) {
            return;
        } else {
            auto s = parse_statement();
            stats.push_back(s);
        }
        tok = lexer.peek_token();
    }
}

/**
 * @brief ident ":=" expr
 *
 * @return std::shared_ptr<ASTAssignment>
 */
std::shared_ptr<ASTAssignment> Parser::parse_assignment() {
    debug("Parser::parse_assignment");
    auto assign = makeAST<ASTAssignment>(lexer);

    assign->ident = parse_designator();
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
    auto ret = makeAST<ASTReturn>(lexer);
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
 * @return std::shared_ptr<ASTExit>
 */
std::shared_ptr<ASTExit> Parser::parse_exit() {
    get_token(TokenType::exit);
    auto ex = makeAST<ASTExit>(lexer);
    return ex;
};

/**
 * @brief  IDENT "(" expr ( "," expr )* ")"
 *
 * @return std::shared_ptr<ASTCall>
 */
std::shared_ptr<ASTCall> Parser::parse_call() {
    auto call = makeAST<ASTCall>(lexer);

    call->name = parse_identifier();
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
            throw ParseException(
                llvm::formatv("Unexpected {0} expecting , or )", tok.val),
                lexer.get_location());
        }
        get_token(TokenType::r_paren);
    }
    return call;
}

inline const std::set<TokenType> if_ends{TokenType::end, TokenType::elsif,
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
    auto stat = makeAST<ASTIf>(lexer);

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
    auto ast = makeAST<ASTFor>(lexer);

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

std::shared_ptr<ASTWhile> Parser::parse_while() {
    auto ast = makeAST<ASTWhile>(lexer);

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

std::shared_ptr<ASTRepeat> Parser::parse_repeat() {
    auto ast = makeAST<ASTRepeat>(lexer);

    // REPEAT
    get_token(TokenType::repeat);

    parse_statement_block(ast->stats, repeat_ends);

    // UNTIL
    get_token(TokenType::until);
    ast->expr = parse_expr();

    return ast;
}

std::shared_ptr<ASTLoop> Parser::parse_loop() {
    auto ast = makeAST<ASTLoop>(lexer);

    // LOOP
    get_token(TokenType::loop);

    parse_statement_block(ast->stats, module_ends);

    // END
    get_token(TokenType::end);
    return ast;
}

std::shared_ptr<ASTBlock> Parser::parse_block() {
    auto ast = makeAST<ASTBlock>(lexer);
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
 * @return std::shared_ptr<ASTExpr>
 */

inline const std::set<TokenType> relationOps = {
    TokenType::equals, TokenType::hash,    TokenType::less,
    TokenType::leq,    TokenType::greater, TokenType::gteq};

std::shared_ptr<ASTExpr> Parser::parse_expr() {
    debug("Parser::parse_expr");
    auto ast = makeAST<ASTExpr>(lexer);

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
    auto expr = makeAST<ASTSimpleExpr>(lexer);

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

inline const std::set<TokenType> termOps = {
    TokenType::asterisk, TokenType::div, TokenType::mod, TokenType::ampersand};

std::shared_ptr<ASTTerm> Parser::parse_term() {
    debug("Parser::parse_term");
    auto term = makeAST<ASTTerm>(lexer);

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
 * @brief factor -> designator
 *                  | procedureCall
 *                  | INTEGER
 *                  | "TRUE" | "FALSE"
 *                  | '('expr ')'
 *
 * @return std::shared_ptr<ASTFactor>
 */
std::shared_ptr<ASTFactor> Parser::parse_factor() {
    debug("Parser::parse_factor");
    auto ast = makeAST<ASTFactor>(lexer);

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
        debug("factor nexttok: {0}", std::string(nexttok));
        if (nexttok.type == TokenType::l_paren) {
            lexer.push_token(tok);
            ast->factor = parse_call();
            return ast;
        }
        lexer.push_token(tok);
        ast->factor = parse_designator();
        return ast;
    }
    default:
        throw ParseException(
            llvm::formatv("Unexpected token: {0}", std::string(tok)),
            lexer.get_location());
    }
    return nullptr; // Not factor
}

/**
 * @brief IDENT selector
 *
 * selector = ( '[' expr ']' )*
 *
 * @return std::shared_ptr<ASTDesignator>
 */

inline const std::set<TokenType> designatorOps = {TokenType::l_bracket,
                                                  TokenType::period};

std::shared_ptr<ASTDesignator> Parser::parse_designator() {
    debug("Parser::parse_designator");
    auto ast = makeAST<ASTDesignator>(lexer);

    ast->ident = parse_qualident();

    auto tok = lexer.peek_token();
    while (designatorOps.find(tok.type) != designatorOps.end()) {
        switch (tok.type) {
        case TokenType::l_bracket: {
            lexer.get_token(); // [
            auto expr = parse_expr();
            get_token(TokenType::r_bracket);
            ast->selectors.emplace_back(expr);
            break;
        }
        case TokenType::period:
            lexer.get_token(); // .
            ast->selectors.push_back(FieldRef{parse_identifier(), -1});
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
 * @return std::shared_ptr<ASTType>
 */
std::shared_ptr<ASTType> Parser::parse_type() {
    auto ast = makeAST<ASTType>(lexer);

    auto tok = lexer.peek_token();
    switch (tok.type) {
    case TokenType::array:
        ast->type = parse_array();
        return ast;
    case TokenType::record:
        ast->type = parse_record();
        return ast;
    default:
        ast->type = parse_qualident();
        return ast;
    }
}

/**
 * @brief "ARRAY" "[" expr "]" "OF" type
 *
 * @return std::shared_ptr<ASTArray>
 */
std::shared_ptr<ASTArray> Parser::parse_array() {
    auto ast = makeAST<ASTArray>(lexer);

    get_token(TokenType::array);
    ast->size = parse_integer();
    get_token(TokenType::of);
    ast->type = parse_type();
    return ast;
}

/**
 * @brief "RECORD" fieldList ( ";" fieldList )* "END"
 *
 * @return std::shared_ptr<ASTRecord>
 */
std::shared_ptr<ASTRecord> Parser::parse_record() {
    debug("Parser::parse_record");
    auto ast = makeAST<ASTRecord>(lexer);

    get_token(TokenType::record);
    auto tok = lexer.peek_token();
    while (tok.type == TokenType::ident) {
        std::vector<std::shared_ptr<ASTIdentifier>> ids;
        parse_identList(ids);

        get_token(TokenType::colon);
        auto type = parse_type();

        std::for_each(begin(ids), end(ids), [ast, type](auto const &i) {
            ast->fields.push_back(VarDec{i, type});
        });
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
 * @brief Qualident = [ident "."] ident.
 *
 * @return std::shared_ptr<ASTIdentifier>
 */
std::shared_ptr<ASTQualident> Parser::parse_qualident() {
    debug("Parser::parse_qualident");
    auto ast = makeAST<ASTQualident>(lexer);

    auto first = get_token(TokenType::ident);
    auto tok = lexer.peek_token();
    if (tok.type == TokenType::period) {

        // Check if identifier is a imported module
        // If a module then make a full qualified identifier,
        // else it is a record access.

        lexer.get_token(); // .
        auto ident = get_token(TokenType::ident);

        auto res = symbols->find(first.val);
        if (res && std::dynamic_pointer_cast<ModuleType>(res->first)) {
            debug("Parser::parse_qualident found module {0}", first.val);
            ast->qual = first.val;
            ast->value = ident.val;
        } else {
            debug("Parser::parse_qualident not found module {0}", first.val);
            // push back tokens
            lexer.push_token(ident);
            lexer.push_token(tok);
            ast->value = first.val;
        }
    } else {
        ast->value = first.val;
    }
    debug("Parser::parse_qualident: {0}", std::string(*ast));
    return ast;
}

/**
 * @brief IDENT
 *
 * @return std::shared_ptr<ASTIdentifier>
 */
std::shared_ptr<ASTIdentifier> Parser::parse_identifier() {
    debug("Parser::parse_identifier");
    auto ast = makeAST<ASTIdentifier>(lexer);

    auto tok = get_token(TokenType::ident);
    ast->value = tok.val;
    return ast;
}

/**
 * @brief INTEGER
 *
 * @return std::shared_ptr<ASTInteger>
 */
std::shared_ptr<ASTInteger> Parser::parse_integer() {
    debug("Parser::parse_integer");
    auto ast = makeAST<ASTInteger>(lexer);

    auto  tok = get_token(TokenType::integer);
    char *end = nullptr;
    ast->value = std::strtol(tok.val.c_str(), &end, 10);
    return ast;
}

/**
 * @brief TRUE | FALSE
 *
 * @return std::shared_ptr<ASTBool>
 */
std::shared_ptr<ASTBool> Parser::parse_boolean() {
    auto ast = makeAST<ASTBool>(lexer);
    auto tok = lexer.get_token();
    ast->value = (tok.type == TokenType::true_k);
    return ast;
}

std::shared_ptr<ASTModule> Parser::parse() {
    return parse_module();
}

void Parser::setup_builtins() {
    debug("Parser::setup_builtins");

    builtins = {
        {"WriteInt",
         std::make_shared<ProcedureType>(
             TypeTable::VoidType,
             ProcedureType::ParamsList{{TypeTable::IntType, Attr::null}})},

        {"WriteBoolean",
         std::make_shared<ProcedureType>(
             TypeTable::VoidType,
             ProcedureType::ParamsList{{TypeTable::BoolType, Attr::null}})},

        {"WriteLn", std::make_shared<ProcedureType>(
                        TypeTable::VoidType, ProcedureType::ParamsList{})}};

    std::for_each(begin(builtins), end(builtins), [this](auto &f) {
        symbols->put(f.first, {f.second, Attr::null});
        types.put(f.first, f.second);
    });
}

}; // namespace ax
