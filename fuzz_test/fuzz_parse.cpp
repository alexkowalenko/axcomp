//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <sstream>

#include "error.hh"
#include "lexer.hh"
#include "parser.hh"
#include "symboltable.hh"
#include "token.hh"

using namespace ax;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    std::string        s{reinterpret_cast<const char *>(data), size};
    std::istringstream is(s);
    ErrorManager       errors;
    Lexer              lex(is, errors);

    auto      symbols = make_Symbols(nullptr);
    TypeTable types;
    types.initialise();
    Parser parser(lex, symbols, types, errors);

    try {
        auto ast = parser.parse();
    } catch (LexicalException const &) {
    } catch (ParseException const &) {
    }

    return 0;
}