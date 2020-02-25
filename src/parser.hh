//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include "astmod.hh"
#include "lexer.hh"

namespace ax {
class Parser {
  public:
    Parser(Lexer &l) : lexer(l){};

    std::shared_ptr<ASTModule> parse();

  private:
    std::shared_ptr<ASTModule>      parse_module();
    std::shared_ptr<ASTDeclaration> parse_declaration();
    std::shared_ptr<ASTConst>       parse_const();
    std::shared_ptr<ASTExpr>        parse_expr();
    std::shared_ptr<ASTTerm>        parse_term();
    std::shared_ptr<ASTFactor>      parse_factor();
    std::shared_ptr<ASTInteger>     parse_integer();
    std::shared_ptr<ASTIdentifier>  parse_identifier();

    Token get_token(TokenType t);

    Lexer &lexer;
};

} // namespace ax