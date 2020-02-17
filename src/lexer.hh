//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <iostream>
#include <string>

#include "token.hh"

namespace ax {

class Lexer {
  public:
    Lexer(std::istream &stream) : is(stream){};

    Token get_token();

  private:
    char  get_char();
    Token scan_digit(char c);

    std::istream &is;
    int           lineno = 1;
};

} // namespace ax