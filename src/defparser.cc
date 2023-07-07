//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "defparser.hh"

#include <optional>

#include <llvm/Support/FormatVariadic.h>

#include "error.hh"
#include "typetable.hh"

namespace ax {

/**
 * @brief module -> "DEFINITION" IDENT ";"  declarations "END" IDENT "."
 *
 * @return ASTModulePtr
 */
ASTModule DefParser::parse_module() {
    auto module = makeAST<ASTModule_>(lexer);

    // MODULE ident BEGIN (expr)+ END ident.
    get_token(TokenType::definition);
    auto tok = get_token(TokenType::ident);
    module->name = tok.val;
    get_token(TokenType::semicolon);
    module->decs = parse_declaration();

    // Procedures
    tok = lexer.peek_token();
    while (tok.type == TokenType::procedure) {
        module->procedures.push_back(parse_procedure());
        tok = lexer.peek_token();
    }

    // END
    get_token(TokenType::end);
    tok = get_token(TokenType::ident);
    if (tok.val != module->name) {
        throw ParseException(
            llvm::formatv("END identifier name: {0} doesn't match module name: {1}", tok.val,
                          module->name),
            lexer.get_location());
    }
    get_token(TokenType::period);
    return module;
}

/**
 * @brief "PROCEDURE" IdentDef [formalParameters] [ ":" type ] ";"
 *
 * @return ASTProcedurePtr
 */
ASTProcedure DefParser::parse_procedure() {
    auto proc = makeAST<ASTProcedure_>(lexer);

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
    return proc;
}

ASTModule DefParser::parse() {
    return parse_module();
}

}; // namespace ax
