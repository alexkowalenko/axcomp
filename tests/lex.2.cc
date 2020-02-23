//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <sstream>
#include <vector>

#include "gtest/gtest.h"

#include "error.hh"
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

    {"1", TokenType::integer, "1"},
    {"\n1", TokenType::integer, "1"},
    {"\n12", TokenType::integer, "12"},
    {"\n 1234567890", TokenType::integer, "1234567890"},

    {"\n;", TokenType::semicolon, ";"},
    {";", TokenType::semicolon, ";"},
    {".", TokenType::period, "."},
    {"+", TokenType::plus, "+"},
    {"-", TokenType::dash, "-"},
    {"*", TokenType::asterisk, "*"},
    {"(", TokenType::l_paren, "("},
    {")", TokenType::r_paren, ")"},

    // comments
    {"(* hello *)1", TokenType::integer, "1"},
    {"(* hello *) 1", TokenType::integer, "1"},
    {"(**) 1", TokenType::integer, "1"},
    {"(* hello (* there! *) *)1", TokenType::integer, "1"},
    // error in comment
    {"(* hello (* there! *)1", TokenType::eof, ""},

    // keyword
    {"MODULE", TokenType::module, "MODULE"},
    {"BEGIN", TokenType::begin, "BEGIN"},
    {"END", TokenType::end, "END"},
    {"DIV", TokenType::div, "DIV"},
    {"MOD", TokenType::mod, "MOD"},

    // identifiers
    {"a", TokenType::ident, "a"},
    {"a1", TokenType::ident, "a1"},
    {"a1z", TokenType::ident, "a1z"},
    {"IsAlpha", TokenType::ident, "IsAlpha"},
    {"is_digit", TokenType::ident, "is_digit"},
};

TEST(Lexer, Lexer1) {

    for (auto t : tests) {

        std::istringstream is(t.input);
        Lexer              lex(is);

        std::cout << "Scan " << t.input;
        try {
            auto token = lex.get_token();
            std::cout << " get " << token.val << std::endl;
            EXPECT_EQ(token.type, t.token);
            EXPECT_EQ(token.val, t.val);
        } catch (LexicalException &l) {
            std::cerr << "Exception: " << l.error_msg() << std::endl;
            FAIL();
        } catch (...) {
            std::cerr << "Unknown Exception" << std::endl;
            FAIL();
        }
    }
}