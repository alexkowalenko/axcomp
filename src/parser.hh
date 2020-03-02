//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include "astmod.hh"
#include "lexer.hh"
#include "symbol.hh"
#include "symboltable.hh"

namespace ax {
class Parser {
  public:
    explicit Parser(Lexer &l, SymbolTable<Symbol> &s) : lexer(l), symbols(s){};

    std::shared_ptr<ASTModule> parse();

  private:
    std::shared_ptr<ASTModule>      parse_module();
    std::shared_ptr<ASTDeclaration> parse_declaration();
    std::shared_ptr<ASTConst>       parse_const();
    std::shared_ptr<ASTVar>         parse_var();
    std::shared_ptr<ASTProcedure>   parse_procedure();
    std::shared_ptr<ASTStatement>   parse_statement();
    std::shared_ptr<ASTAssignment>  parse_assignment(Token const &ident);
    std::shared_ptr<ASTReturn>      parse_return();
    std::shared_ptr<ASTCall>        parse_call(Token const &ident);
    std::shared_ptr<ASTExpr>        parse_expr();
    std::shared_ptr<ASTTerm>        parse_term();
    std::shared_ptr<ASTFactor>      parse_factor();
    std::shared_ptr<ASTInteger>     parse_integer();
    std::shared_ptr<ASTIdentifier>  parse_identifier();

    Token get_token(TokenType t);

    Lexer &              lexer;
    SymbolTable<Symbol> &symbols;
};

} // namespace ax