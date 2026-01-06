//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <exception>

#include <utf8.h>

#include "error.hh"
#include "lexerUTF8.hh"

namespace ax {

class EOFException : std::exception {};

Char LexerUTF8::get() {
    Char c = 0;
    if (last_char != 0) {
        std::swap(c, last_char);
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
        throw LexicalException(get_location(), "Not valid UTF-8 text");
    }
    ptr = buf.begin();
};

std::string wcharToString(const wchar_t wchar) {
    std::mbstate_t        state = std::mbstate_t();
    std::array<char, 120> buffer;
    const size_t          length = std::wcrtomb(buffer.data(), wchar, &state);

    if (length == static_cast<size_t>(-1)) {
        throw std::runtime_error("Failed to convert wide character to multibyte string");
    }

    return {buffer.data(), length};
}

} // namespace ax