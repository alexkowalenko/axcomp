//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "token.hh"

#include <fmt/core.h>

namespace ax {

std::string to_string(TokenType &t) {

    switch (t) {
    case TokenType::null:
        return "null";
    case TokenType::integer:
        return "integer";
    case TokenType::ident:
        return "indent";
    case TokenType::semicolon:
        return "semicolon";
    case TokenType::period:
        return "period";

    // Keywords
    case TokenType::module:
        return "MODULE";
    case TokenType::begin:
        return "BEGIN";
    case TokenType::end:
        return "END";

    case TokenType::eof:
        return "EOF";
    default:
        return "Unknown token";
    }
}

Token::operator std::string() {
    switch (type) {
    case TokenType::null:
        return "null";
    case TokenType::integer:
        return fmt::format("integer({})", val);
    case TokenType::ident:
        return fmt::format("indent({})", val);
    case TokenType::semicolon:
        return "semicolon";
    case TokenType::period:
        return "period";

    // Keywords
    case TokenType::module:
        return "MODULE";
    case TokenType::begin:
        return "BEGIN";
    case TokenType::end:
        return "END";

    case TokenType::eof:
        return "EOF";
    default:
        return to_string(type);
    }
}

std::ostream &operator<<(std::ostream &os, Token &t) {
    return os << std::string(t);
}

} // namespace ax
