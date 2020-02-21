//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "token.hh"

namespace ax {

std::ostream &operator<<(std::ostream &os, const Token &t) {
    switch (t.type) {
    case TokenType::null:
        return os << "null";
    case TokenType::integer:
        return os << "integer";
    case TokenType::semicolon:
        return os << "semicolon";
    case TokenType::eof:
        return os << "EOF";
    default:
        return os << "Unknown token";
    }
}

} // namespace ax
