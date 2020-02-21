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
    semicolon,
};

class Token {
  public:
    Token(TokenType t) : type(t){};
    Token(TokenType t, std::string v) : type(t), val(v){};

    TokenType   type;
    std::string val;
};

std::ostream &operator<<(std::ostream &os, const Token &t);

} // namespace ax
