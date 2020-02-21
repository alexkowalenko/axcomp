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
    Lexer(std::istream &stream);

    Token get_token();
    Token peek_token();

    int lineno = 1;

  private:
    void get_comment();

    char  get_char();
    Token scan_digit(char c);

    std::istream &is;
    Token         next_token;
};

} // namespace ax