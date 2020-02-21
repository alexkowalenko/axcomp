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

    ASTModule *parse();

  private:
    ASTModule * parse_module();
    ASTExpr *   parse_expr();
    ASTInteger *parse_integer();

    Lexer &lexer;
};

} // namespace ax