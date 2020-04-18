//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "lexerUTF8.hh"

#include "error.hh"
#include <exception>

namespace ax {

class EOFException : std::exception {};

Char LexerUTF8::get() {
    Char c = 0;
    try {
        while (ptr == buf.end()) {
            get_line();
        }
        // Char c = utf8::next(ptr, buf.end());
        c = *ptr;
        ptr++;
    } catch (EOFException &) {
        c = -1;
    }
    return c;
}
Char LexerUTF8::peek() {
    return *ptr;
}

void LexerUTF8::get_line() {
    if (!getline(is, buf)) {
        throw EOFException{};
    }
    buf.push_back('\n');
    ptr = buf.begin();
};

} // namespace ax