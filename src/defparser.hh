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
    DefParser(Lexer &l, SymbolFrameTable &s, TypeTable &t, ErrorManager &e) : Parser(l, s, t, e){};

    ASTModule parse();

  private:
    ASTModule    parse_module();
    ASTProcedure parse_procedure();
};

} // namespace ax