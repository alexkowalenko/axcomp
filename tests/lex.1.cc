//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <sstream>

#include "gtest/gtest.h"

#include "error.hh"
#include "lexer.hh"
#include "token.hh"

using namespace ax;

TEST(Lexer, Null) {

    std::istringstream is("");
    Lexer              lex(is);

    auto token = lex.get_token();
    EXPECT_EQ(token.type, TokenType::eof);
}

TEST(Lexer, Exception) {

    std::istringstream is("x");
    Lexer              lex(is);

    try {
        auto token = lex.get_token();
    } catch (LexicalException l) {
        std::cerr << l.error_msg() << std::endl;
        EXPECT_EQ(l.error_msg(), "Error: Unknown character x in line: 1");
    }
}

TEST(Lexer, Whitespace) {

    std::istringstream is(" \n\t");
    Lexer              lex(is);

    auto token = lex.get_token();
    EXPECT_EQ(token.type, TokenType::eof);
}

TEST(Lexer, Digit) {

    std::istringstream is("1\n");
    Lexer              lex(is);

    auto token = lex.get_token();
    EXPECT_EQ(token.type, TokenType::integer);
    EXPECT_EQ(token.val, "1");
}