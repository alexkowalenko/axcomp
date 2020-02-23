//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <fmt/core.h>

#include "error.hh"
#include "lexer.hh"

namespace ax {

static Token nullToken = Token(TokenType::null);

Lexer::Lexer(std::istream &stream) : is(stream), next_token(nullToken){};

void Lexer::get_comment() {
    char c = is.get(); // get asterisk
    do {
        c = is.get();
        if (c == '*' && is.peek() == ')') {
            c = is.get();
            return;
        } else if (c == '(' && is.peek() == '*') {
            // suport nested comments, call recursively
            get_comment();
        }
    } while (is);
}

char Lexer::get_char() {
    char c = 0;
    while (is) {
        c = is.get();
        // fmt::print("Char: {} next: {}\n", c, is.peek());
        if (c == '\n') {
            lineno++;
            continue;
        } else if (c == '(' && is.peek() == '*') {
            get_comment();
            continue;
        }
        if (isspace(c)) {
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
    while (std::isalnum(c)) {
        is.get();
        ident += c;
        c = is.peek();
    }
    if (ident == "MODULE") {
        return Token(TokenType::module, ident);
    }
    if (ident == "BEGIN") {
        return Token(TokenType::begin, ident);
    }
    if (ident == "END") {
        return Token(TokenType::end, ident);
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
    switch (c) {
    case -1:
        return Token(TokenType::eof);
    case ';':
        return Token(TokenType::semicolon, ";");
    case '.':
        return Token(TokenType::period, ".");
    default:
        if (std::isdigit(c)) {
            return scan_digit(c);
        }
        if (std::isalpha(c)) {
            return scan_ident(c);
        }
        throw LexicalException(std::string("Unknown character ") + c, lineno);
    }
}

Token Lexer::peek_token() {
    if (next_token.type == TokenType::null) {
        next_token = get_token();
    }
    return next_token;
}

} // namespace ax