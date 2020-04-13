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
    DefParser(Lexer &l, Symbols &s, TypeTable &t, ErrorManager const &e)
        : Parser(l, s, t, e){};

    std::shared_ptr<ASTModule> parse();

  private:
    std::shared_ptr<ASTModule>    parse_module();
    std::shared_ptr<ASTProcedure> parse_procedure();
};

} // namespace ax