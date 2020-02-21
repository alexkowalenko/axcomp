//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <fmt/core.h>

#include "error.hh"
#include "lexer.hh"

namespace ax {

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
};

char Lexer::get_char() {
    char c = 0;
    while (is) {
        c = is.get();
        fmt::print("Char: {} next: {}\n", c, is.peek());
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
    while (isdigit(c)) {
        is.get();
        digit += c;
        c = is.peek();
    }
    return Token(TokenType::integer, digit);
}

Token Lexer::get_token() {
    auto c = get_char();
    switch (c) {
    case -1:
        return Token(TokenType::eof);
    case ';':
        return Token(TokenType::semicolon);
    default:
        if (isdigit(c)) {
            return scan_digit(c);
        }
        throw LexicalException(std::string("Unknown character ") + c, lineno);
    }
}

} // namespace ax