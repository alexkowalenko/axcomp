//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "lexer.hh"
#include "error.hh"

namespace ax {

char Lexer::get_char() {
    char c = 0;
    while (is) {
        c = is.get();
        if (c == '\n') {
            lineno++;
            continue;
        } else if (isspace(c)) {
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