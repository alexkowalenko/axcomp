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
    explicit DefParser(Lexer &l, std::shared_ptr<SymbolTable<TypePtr>> s,
                       TypeTable &t, ErrorManager const &e)
        : Parser(l, s, t, e){};

    std::shared_ptr<ASTModule> parse();

  private:
    std::shared_ptr<ASTModule>    parse_module();
    std::shared_ptr<ASTProcedure> parse_procedure();
};

extern std::vector<std::pair<std::string, std::shared_ptr<ProcedureType>>>
    builtins;

} // namespace ax