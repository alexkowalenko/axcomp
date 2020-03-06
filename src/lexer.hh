//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "token.hh"

namespace ax {

class Lexer {
  public:
    explicit Lexer(std::istream &stream);

    Token get_token();
    void  push_token(Token const &t);
    Token peek_token();

    int get_lineno() { return lineno; };

  private:
    void get_comment();

    char  get_char();
    Token scan_digit(char c);
    Token scan_ident(char c);

    int                lineno = 1;
    std::istream &     is;
    std::vector<Token> next_token;
};

} // namespace ax