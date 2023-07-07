//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <sstream>

#include "gtest/gtest.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wshadow"
#include "error.hh"
#include "lexer.hh"
#include "lexerUTF8.hh"
#include "token.hh"
#pragma clang diagnostic pop

using namespace ax;

TEST(Lexer, Null) {

    std::istringstream is("");
    Lexer              lex(is, ErrorManager{});

    auto token = lex.get_token();
    EXPECT_EQ(token.type, TokenType::eof);
}

TEST(Lexer, Exception) {

    std::istringstream is("x");
    ErrorManager       errors;
    Lexer              lex(is, errors);

    try {
        auto token = lex.get_token();
    } catch (LexicalException &l) {
        std::cerr << l.error_msg() << std::endl;
        EXPECT_EQ(l.error_msg(), "1: Unknown character x");
    }
}

TEST(LexerUTF8, Null) {

    std::istringstream is("");
    Lexer              lex(is, ErrorManager{});

    auto token = lex.get_token();
    EXPECT_EQ(token.type, TokenType::eof);
}

TEST(LexerUTF8, Exception) {

    std::istringstream is("x");
    ErrorManager       errors;
    LexerUTF8          lex(is, errors);

    try {
        auto token = lex.get_token();
    } catch (LexicalException &l) {
        std::cerr << l.error_msg() << std::endl;
        EXPECT_EQ(l.error_msg(), "1: Unknown character x");
    }
}

TEST(LexerUTF8, Whitespace) {

    std::istringstream is(" \n\t");
    ErrorManager       errors;
    Lexer              lex(is, errors);

    auto token = lex.get_token();
    EXPECT_EQ(token.type, TokenType::eof);
}

TEST(LexerUTF8, Digit) {

    std::istringstream is("12\n");
    ErrorManager       errors;
    Lexer              lex(is, errors);

    auto token = lex.get_token();
    std::cout << "val = " << token.val << '\n';
    EXPECT_EQ(token.type, TokenType::integer);
    EXPECT_EQ(token.val, "12");
}