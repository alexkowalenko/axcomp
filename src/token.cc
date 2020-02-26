//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "token.hh"

#include <fmt/core.h>

namespace ax {

std::string string(TokenType &t) {

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
    case TokenType::comma:
        return ",";

    case TokenType::plus:
        return "+";
    case TokenType::dash:
        return "-";
    case TokenType::asterisk:
        return "*";

    case TokenType::l_paren:
        return "(";
    case TokenType::r_paren:
        return ")";
    case TokenType::colon:
        return ":";
    case TokenType::equals:
        return "=";
    case TokenType::assign:
        return ":=";

    // Keywords
    case TokenType::module:
        return "MODULE";
    case TokenType::begin:
        return "BEGIN";
    case TokenType::end:
        return "END";
    case TokenType::div:
        return "DIV";
    case TokenType::mod:
        return "MOD";
    case TokenType::cnst:
        return "CONST";
    case TokenType::type:
        return "TYPE";
    case TokenType::var:
        return "VAR";
    case TokenType::ret:
        return "RETURN";
    case TokenType::procedure:
        return "PROCEDURE";

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
        return val;

    default:
        return string(type);
    }
}

std::ostream &operator<<(std::ostream &os, Token &t) {
    return os << std::string(t);
}

} // namespace ax
