//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <sstream>

#include "error.hh"
#include "lexer.hh"
#include "token.hh"

using namespace ax;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    std::string        s{reinterpret_cast<const char *>(data), size};
    std::istringstream is(s);
    Lexer              lex(is, ErrorManager{});

    try {
        Token token = lex.get_token();
        while (token.type != TokenType::eof) {
            token = lex.get_token();
        }
    } catch (LexicalException const &) {
    }

    return 0;
}