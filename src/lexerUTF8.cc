//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "lexerUTF8.hh"

#include <exception>

#include "utf8.h"

#include "error.hh"

namespace ax {

class EOFException : std::exception {};

Char LexerUTF8::get() {
    Char c = 0;
    if (last_char != 0) {
        c = last_char;
        last_char = 0;
        return c;
    }
    try {
        while (ptr == buf.end()) {
            get_line();
        }
        c = utf8::next(ptr, buf.end());
    } catch (EOFException &) {
        c = -1;
        return c;
    }
    charpos++;
    return c;
}
Char LexerUTF8::peek() {
    Char c = (Char)utf8::peek_next(ptr, buf.end());
    return c;
}

void LexerUTF8::get_line() {
    if (!getline(is, buf)) {
        throw EOFException{};
    }
    buf.push_back('\n');
    // check UTF-8 correctness
    if (!utf8::is_valid(buf.begin(), buf.end())) {
        throw LexicalException("Not valid UTF-8 text", get_location());
    }
    ptr = buf.begin();
};

} // namespace ax