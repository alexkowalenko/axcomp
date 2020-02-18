//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <sstream>

#include "gtest/gtest.h"

#include "error.hh"
#include "lexer.hh"
#include "parser.hh"

using namespace ax;

TEST(Parser, Null) {

    std::istringstream is("");
    Lexer              lex(is);
    Parser             parser(lex);

    auto ast = parser.parse();
}
