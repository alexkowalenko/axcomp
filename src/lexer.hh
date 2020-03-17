//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <iostream>
#include <stack>
#include <string>

#include "location.hh"
#include "token.hh"

namespace ax {

class Lexer {
  public:
    explicit Lexer(std::istream &stream);

    Token get_token();
    void  push_token(Token const &t);
    Token peek_token();

    [[nodiscard]] Location get_location() const { return Location{lineno, charpos}; }

  private:
    void get_comment();

    char  get_char();
    Token scan_digit(char c);
    Token scan_ident(char c);

    char get() {
        charpos++;
        return is.get();
    }

    void set_newline() {
        lineno++;
        charpos = 0;
    }

    int lineno = 1;
    int charpos = 0;

    std::istream &    is;
    std::stack<Token> next_token;
};

} // namespace ax