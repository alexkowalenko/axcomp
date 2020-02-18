//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include "ast.hh"
#include "lexer.hh"

namespace ax {
class Parser {
  public:
    Parser(Lexer &l) : lexer(l){};

    ASTBase parse();

  private:
    Lexer &lexer;
};

} // namespace ax