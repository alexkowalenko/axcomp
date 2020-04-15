//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include "parser.hh"

namespace ax {

class DefParser : public Parser {
  public:
    DefParser(Lexer &l, Symbols &s, TypeTable &t, ErrorManager const &e) : Parser(l, s, t, e){};

    ASTModulePtr parse();

  private:
    ASTModulePtr    parse_module();
    ASTProcedurePtr parse_procedure();
};

} // namespace ax