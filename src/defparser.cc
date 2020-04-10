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

inline constexpr bool debug_defparser{false};

template <typename... T> inline void debug(const T &... msg) {
    if constexpr (debug_defparser) {
        std::cerr << std::string(llvm::formatv(msg...)) << std::endl;
    }
}

/**
 * @brief module -> "DEFINITION" IDENT ";"  declarations "END" IDENT "."
 *
 * @return std::shared_ptr<ASTModule>
 */
std::shared_ptr<ASTModule> DefParser::parse_module() {
    auto module = makeAST<ASTModule>(lexer);

    // MODULE ident BEGIN (expr)+ END ident.
    get_token(TokenType::definition);
    auto tok = get_token(TokenType::ident);
    module->name = tok.val;
    symbols->put(module->name, TypeTable::ModuleType);
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
            llvm::formatv(
                "END identifier name: {0} doesn't match module name: {1}",
                tok.val, module->name),
            lexer.get_location());
    }
    get_token(TokenType::period);
    return module;
}

/**
 * @brief "PROCEDURE" IdentDef [formalParameters] [ ":" type ] ";"
 *
 * @return std::shared_ptr<ASTProcedure>
 */
std::shared_ptr<ASTProcedure> DefParser::parse_procedure() {
    debug("DefParser::parse_procedure");
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
    return proc;
}

std::shared_ptr<ASTModule> DefParser::parse() {
    return parse_module();
}

}; // namespace ax
