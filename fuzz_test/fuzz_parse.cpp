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
    std::string        s{(const char *)data, size};
    std::istringstream is(s);
    Lexer              lex(is);

    auto   symbols = std::make_shared<SymbolTable<TypePtr>>(nullptr);
    Parser parser(lex, symbols);

    try {
        auto ast = parser.parse();
    } catch (LexicalException) {
    } catch (ParseException) {
    }

    return 0;
}