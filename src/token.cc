//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "token.hh"

#include <format>
#include <map>

namespace ax {

static const std::map<TokenType, std::string> mapping{
    {TokenType::null, "null"},

    {TokenType::ident, "indent"},
    {TokenType::semicolon, "semicolon"},
    {TokenType::period, "period"},
    {TokenType::comma, ","},

    {TokenType::integer, "integer"},
    {TokenType::hexinteger, "hexinteger"},
    {TokenType::chr, "chr"},
    {TokenType::hexchr, "hexchr"},
    {TokenType::string, "string"},

    {TokenType::plus, "+"},
    {TokenType::dash, "-"},
    {TokenType::asterisk, "*"},

    {TokenType::l_paren, "("},
    {TokenType::r_paren, ")"},
    {TokenType::colon, ":"},
    {TokenType::equals, "="},
    {TokenType::assign, ":="},
    {TokenType::hash, "#"},
    {TokenType::less, "<"},
    {TokenType::leq, "<="},
    {TokenType::greater, ">"},
    {TokenType::gteq, ">="},
    {TokenType::tilde, "~"},
    {TokenType::ampersand, "&"},
    {TokenType::l_bracket, "["},
    {TokenType::r_bracket, "]"},
    {TokenType::dotdot, ".."},
    {TokenType::bar, "|"},
    {TokenType::slash, "/"},
    {TokenType::caret, "^"},
    {TokenType::l_brace, "{"},
    {TokenType::r_brace, "}"},

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
    {TokenType::or_k, "OR"},
    {TokenType::if_k, "IF"},
    {TokenType::then, "THEN"},
    {TokenType::elsif, "ELSIF"},
    {TokenType::else_k, "ELSE"},
    {TokenType::for_k, "FOR"},
    {TokenType::to, "TO"},
    {TokenType::by, "BY"},
    {TokenType::do_k, "DO"},
    {TokenType::while_k, "WHILE"},
    {TokenType::repeat, "REPEAT"},
    {TokenType::until, "UNTIL"},
    {TokenType::loop, "LOOP"},
    {TokenType::exit, "EXIT"},
    {TokenType::array, "ARRAY"},
    {TokenType::of, "OF"},
    {TokenType::record, "RECORD"},
    {TokenType::definition, "DEFINITION"},
    {TokenType::import, "IMPORT"},
    {TokenType::cse, "CASE"},
    {TokenType::pointer, "POINTER"},
    {TokenType::nil, "NIL"},
    {TokenType::in, "IN"},

    {TokenType::eof, "EOF"},
};

std::string string(TokenType t) {
    if (auto res = mapping.find(t); res != mapping.end()) {
        return res->second;
    }
    return "Unknown token";
}

Token::operator std::string() const {
    switch (type) {
    case TokenType::integer:
        return std::format("integer({0})", val);
    case TokenType::hexinteger:
        return std::format("hexinteger({0})", val);
    case TokenType::chr:
        return std::format("'{0}'", val_int);
    case TokenType::hexchr:
        return std::format("hexchar({0})", val);
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
