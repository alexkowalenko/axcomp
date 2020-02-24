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
    std::shared_ptr<ASTModule>  parse_module();
    std::shared_ptr<ASTExpr>    parse_expr();
    std::shared_ptr<ASTTerm>    parse_term();
    std::shared_ptr<ASTInteger> parse_integer();

    Token get_token(TokenType t);

    Lexer &lexer;
};

} // namespace ax