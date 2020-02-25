//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <iostream>
#include <string>

namespace ax {

enum class TokenType {
    null,
    eof,
    integer,
    ident,
    semicolon,
    period,
    plus,
    dash,
    asterisk,
    l_paren,
    r_paren,
    equals,
    colon,

    // Keywords
    module,
    begin,
    end,
    div,
    mod,
    cnst,
    type,
    var,

};

std::string string(TokenType &t);

class Token {
  public:
    Token(TokenType t) : type(t){};
    Token(TokenType t, std::string v) : type(t), val(v){};

    explicit             operator std::string();
    friend std::ostream &operator<<(std::ostream &os, const Token &t);

    TokenType   type;
    std::string val;
};

std::ostream &operator<<(std::ostream &os, const Token &t);

} // namespace ax
