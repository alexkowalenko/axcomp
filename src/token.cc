//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "token.hh"

#include <unordered_map>

#include <fmt/core.h>

namespace ax {

static std::unordered_map<TokenType, std::string> mapping{
    {TokenType::null, "null"},
    {TokenType::integer, "integer"},
    {TokenType::ident, "indent"},
    {TokenType::semicolon, "semicolon"},
    {TokenType::period, "period"},
    {TokenType::comma, ","},

    {TokenType::plus, "+"},
    {TokenType::dash, "-"},
    {TokenType::asterisk, "*"},

    {TokenType::l_paren, "("},
    {TokenType::r_paren, ")"},
    {TokenType::colon, ":"},
    {TokenType::equals, "="},
    {TokenType::assign, ":="},

    // Keywords
    {TokenType::module, "MODULE"},
    {TokenType::begin, "BEGIN"},
    {TokenType::end, "END"},
    {TokenType::div, "DIV"},
    {TokenType::mod, "MOD"},
    {TokenType::cnst, "CONST"},
    {TokenType::type, "TYPE"},
    {TokenType::var, "VAR"},
    {TokenType::ret, "RETURN"},
    {TokenType::procedure, "PROCEDURE"},
    {TokenType::true_k, "TRUE"},
    {TokenType::false_k, "FALSE"},

    {TokenType::eof, "EOF"},
};

std::string string(TokenType t) {
    if (auto res = mapping.find(t); res != mapping.end()) {
        return res->second;
    }
    return "Unknown token";
}

Token::operator std::string() {
    switch (type) {
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
