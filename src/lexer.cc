//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <unordered_map>

#include <fmt/core.h>

#include "error.hh"
#include "lexer.hh"

namespace ax {

static Token nullToken = Token(TokenType::null);

static std::unordered_map<std::string, Token> keyword_map = {
    {"MODULE", Token(TokenType::module, "MODULE")},
    {"BEGIN", Token(TokenType::begin, "BEGIN")},
    {"END", Token(TokenType::end, "END")},
    {"DIV", Token(TokenType::div, "DIV")},
    {"MOD", Token(TokenType::mod, "MOD")},
    {"CONST", Token(TokenType::cnst, "CONST")},
    {"TYPE", Token(TokenType::type, "TYPE")},
    {"VAR", Token(TokenType::var, "VAR")},
    {"RETURN", Token(TokenType::ret, "RETURN")},
    {"PROCEDURE", Token(TokenType::procedure, "PROCEDURE")},
};

static std::unordered_map<char, Token> token_map = {
    {-1, Token(TokenType::eof)},
    {';', Token(TokenType::semicolon, ";")},
    {'.', Token(TokenType::period, ".")},
    {',', Token(TokenType::comma, ",")},
    {'+', Token(TokenType::plus, "+")},
    {'-', Token(TokenType::dash, "-")},
    {'*', Token(TokenType::asterisk, "*")},
    {'(', Token(TokenType::l_paren, "(")},
    {')', Token(TokenType::r_paren, ")")},
    {'=', Token(TokenType::equals, "=")},
};

Lexer::Lexer(std::istream &stream) : is(stream), next_token(nullToken){};

void Lexer::get_comment() {
    is.get(); // get asterisk
    do {
        char c = is.get();
        if (c == '*' && is.peek() == ')') {
            is.get();
            return;
        }
        if (c == '(' && is.peek() == '*') {
            // suport nested comments, call
            // recursively
            get_comment();
        }
    } while (is);
}

char Lexer::get_char() {
    char c = 0;
    while (is) {
        c = is.get();
        // fmt::print("Char: {} next: {}\n", c,
        // is.peek());
        if (c == '\n') {
            lineno++;
            continue;
        }
        if (c == '(' && is.peek() == '*') {
            get_comment();
            continue;
        }
        if (std::isspace(c)) {
            continue;
        }
        return c;
    }
    return -1;
}

Token Lexer::scan_digit(char c) {
    std::string digit(1, c);
    c = is.peek();
    while (std::isdigit(c)) {
        is.get();
        digit += c;
        c = is.peek();
    }
    return Token(TokenType::integer, digit);
}

Token Lexer::scan_ident(char c) {
    std::string ident(1, c);
    c = is.peek();
    while (std::isalnum(c) || c == '_') {
        is.get();
        ident += c;
        c = is.peek();
    }
    // Look for keywords
    if (auto res = keyword_map.find(ident); res != keyword_map.end()) {
        return res->second;
    }
    return Token(TokenType::ident, ident);
}

Token Lexer::get_token() {
    // Check if there is already a token
    if (next_token.type != TokenType::null) {
        Token s = next_token;
        next_token = nullToken;
        return s;
    }

    // Get next token
    auto c = get_char();

    // Check single digit character tokens
    if (auto res = token_map.find(c); res != token_map.end()) {
        return res->second;
    }

    // Check multiple character tokens
    if (c == ':') {
        if (is.peek() == '=') {
            get_char();
            return Token(TokenType::assign, ":=");
        }
        return Token(TokenType::colon, ":");
    }
    if (std::isdigit(c)) {
        return scan_digit(c);
    }
    if (std::isalpha(c)) {
        return scan_ident(c);
    }
    throw LexicalException(std::string("Unknown character ") + c, lineno);
}

Token Lexer::peek_token() {
    if (next_token.type == TokenType::null) {
        next_token = get_token();
    }
    return next_token;
}

} // namespace ax