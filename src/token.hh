//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <iostream>
#include <string>

namespace ax {

enum class TokenType { integer, semicolon, eof };

class Token {
  public:
    Token(TokenType t) : type(t){};

    TokenType    type;
    std::wstring val;
};

std::ostream &operator<<(std::ostream &os, const Token &t);

} // namespace ax
