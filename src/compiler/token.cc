//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "token.hh"

#include <format>
#include <map>

namespace ax {

namespace {

const std::map<TokenType, std::string> mapping{
    {TokenType::None, "null"},

    {TokenType::IDENT, "indent"},
    {TokenType::SEMICOLON, "semicolon"},
    {TokenType::PERIOD, "period"},
    {TokenType::COMMA, ","},

    {TokenType::INTEGER, "integer"},
    {TokenType::HEXINTEGER, "hexinteger"},
    {TokenType::CHR, "chr"},
    {TokenType::HEXCHR, "hexchr"},
    {TokenType::STRING, "string"},

    {TokenType::PLUS, "+"},
    {TokenType::DASH, "-"},
    {TokenType::ASTÉRIX, "*"},

    {TokenType::L_PAREN, "("},
    {TokenType::R_PAREN, ")"},
    {TokenType::COLON, ":"},
    {TokenType::EQUALS, "="},
    {TokenType::ASSIGN, ":="},
    {TokenType::HASH, "#"},
    {TokenType::LESS, "<"},
    {TokenType::LEQ, "<="},
    {TokenType::GREATER, ">"},
    {TokenType::GTEQ, ">="},
    {TokenType::TILDE, "~"},
    {TokenType::AMPERSAND, "&"},
    {TokenType::L_BRACKET, "["},
    {TokenType::R_BRACKET, "]"},
    {TokenType::DOTDOT, ".."},
    {TokenType::BAR, "|"},
    {TokenType::SLASH, "/"},
    {TokenType::CARET, "^"},
    {TokenType::L_BRACE, "{"},
    {TokenType::R_BRACE, "}"},

    // Keywords
    {TokenType::MODULE, "MODULE"},
    {TokenType::BEGIN, "BEGIN"},
    {TokenType::END, "END"},
    {TokenType::DIV, "DIV"},
    {TokenType::MOD, "MOD"},
    {TokenType::CONST, "CONST"},
    {TokenType::TYPE, "TYPE"},
    {TokenType::VAR, "VAR"},
    {TokenType::RETURN, "RETURN"},
    {TokenType::PROCEDURE, "PROCEDURE"},
    {TokenType::TRUE, "TRUE"},
    {TokenType::FALSE, "FALSE"},
    {TokenType::OR, "OR"},
    {TokenType::IF, "IF"},
    {TokenType::THEN, "THEN"},
    {TokenType::ELSIF, "ELSIF"},
    {TokenType::ELSE, "ELSE"},
    {TokenType::FOR, "FOR"},
    {TokenType::TO, "TO"},
    {TokenType::BY, "BY"},
    {TokenType::DO, "DO"},
    {TokenType::WHILE, "WHILE"},
    {TokenType::REPEAT, "REPEAT"},
    {TokenType::UNTIL, "UNTIL"},
    {TokenType::LOOP, "LOOP"},
    {TokenType::EXIT, "EXIT"},
    {TokenType::ARRAY, "ARRAY"},
    {TokenType::OF, "OF"},
    {TokenType::RECORD, "RECORD"},
    {TokenType::DEFINITION, "DEFINITION"},
    {TokenType::IMPORT, "IMPORT"},
    {TokenType::CASE, "CASE"},
    {TokenType::POINTER, "POINTER"},
    {TokenType::NIL, "NIL"},
    {TokenType::IN, "IN"},

    {TokenType::Eof, "EOF"},
};

}

std::string string(const TokenType t) {
    if (mapping.contains(t)) {
        return mapping.at(t);
    }
    return "Unknown token";
}

Token::operator std::string() const {
    switch (type) {
    case TokenType::INTEGER:
        return std::format("integer({0})", val);
    case TokenType::HEXINTEGER:
        return std::format("hexinteger({0})", val);
    case TokenType::CHR:
        return std::format("'{0}'", val_int);
    case TokenType::HEXCHR:
        return std::format("hexchar({0})", val);
    case TokenType::IDENT:
        return val;
    default:
        return string(type);
    }
}

std::ostream &operator<<(std::ostream &os, Token &t) {
    return os << std::string(t);
}

} // namespace ax
