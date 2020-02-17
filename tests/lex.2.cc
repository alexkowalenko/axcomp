//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <sstream>
#include <vector>

#include "gtest/gtest.h"

#include "lexer.hh"
#include "token.hh"

using namespace ax;

struct LexTests {
    std::string input;
    TokenType   token;
    std::string val;
};

std::vector<LexTests> tests = {
    {"\n", TokenType::eof, ""},

    {"\n1", TokenType::integer, "1"},
    {"\n12", TokenType::integer, "12"},
    {"\n 1234567890", TokenType::integer, "1234567890"},

    {"\n;", TokenType::semicolon, ""},
    {";", TokenType::semicolon, ""},
};

TEST(Lexer, Lexer1) {

    for (auto t : tests) {

        std::istringstream is(t.input);
        Lexer              lex(is);

        EXPECT_NO_THROW({
            auto token = lex.get_token();
            EXPECT_EQ(token.type, t.token);
            EXPECT_EQ(token.val, t.val);
        });
    }
}