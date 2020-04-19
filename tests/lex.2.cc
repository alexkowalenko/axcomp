//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//
#include <gtest/gtest.h>

#include "parse_test.hh"

using namespace ax;
std::vector<LexTests> tests = {
    {"\n", TokenType::eof, ""},

    {"1", TokenType::integer, "1"},
    {"\n1", TokenType::integer, "1"},
    {"\n12", TokenType::integer, "12"},
    {"\n 1234567890", TokenType::integer, "1234567890"},
    // hex numbers
    {"0d", TokenType::integer, "0d"},
    {"0cafebabe", TokenType::integer, "0cafebabe"},

    {"\n;", TokenType::semicolon, ";"},
    {";", TokenType::semicolon, ";"},
    {".", TokenType::period, "."},
    {",", TokenType::comma, ","},
    {"+", TokenType::plus, "+"},
    {"-", TokenType::dash, "-"},
    {"*", TokenType::asterisk, "*"},
    {"(", TokenType::l_paren, "("},
    {")", TokenType::r_paren, ")"},
    {":", TokenType::colon, ":"},
    {":=", TokenType::assign, ":="},
    {"=", TokenType::equals, "="},
    {"#", TokenType::hash, "#"},
    {"<", TokenType::less, "<"},
    {"<=", TokenType::leq, "<="},
    {">", TokenType::greater, ">"},
    {">=", TokenType::gteq, ">="},
    {"~", TokenType::tilde, "~"},
    {"&", TokenType::ampersand, "&"},
    {"[", TokenType::l_bracket, "["},
    {"]", TokenType::r_bracket, "]"},

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
    {"CONST", TokenType::cnst, "CONST"},
    {"TYPE", TokenType::type, "TYPE"},
    {"VAR", TokenType::var, "VAR"},
    {"RETURN", TokenType::ret, "RETURN"},
    {"PROCEDURE", TokenType::procedure, "PROCEDURE"},
    {"TRUE", TokenType::true_k, "TRUE"},
    {"FALSE", TokenType::false_k, "FALSE"},
    {"OR", TokenType::or_k, "OR"},
    {"IF", TokenType::if_k, "IF"},
    {"THEN", TokenType::then, "THEN"},
    {"ELSIF", TokenType::elsif, "ELSIF"},
    {"ELSE", TokenType::else_k, "ELSE"},
    {"FOR", TokenType::for_k, "FOR"},
    {"TO", TokenType::to, "TO"},
    {"BY", TokenType::by, "BY"},
    {"DO", TokenType::do_k, "DO"},
    {"WHILE", TokenType::while_k, "WHILE"},
    {"REPEAT", TokenType::repeat, "REPEAT"},
    {"UNTIL", TokenType::until, "UNTIL"},
    {"LOOP", TokenType::loop, "LOOP"},
    {"EXIT", TokenType::exit, "EXIT"},
    {"ARRAY", TokenType::array, "ARRAY"},
    {"RECORD", TokenType::record, "RECORD"},
    {"DEFINITION", TokenType::definition, "DEFINITION"},
    {"IMPORT", TokenType::import, "IMPORT"},

    // identifiers
    {"a", TokenType::ident, "a"},
    {"a1", TokenType::ident, "a1"},
    {"a1z", TokenType::ident, "a1z"},
    {"IsAlpha", TokenType::ident, "IsAlpha"},
    {"is_digit", TokenType::ident, "is_digit"},
};

TEST(Lexer, Lexer) {
    do_lex_tests(tests);
}

TEST(LexerUTF8, Lexer) {
    do_lexUTF8_tests(tests);
}

TEST(LexerUTF8, UTF8) {

    std::vector<LexTests> tests = {

        // comments
        {"(* χαῖρε *)1", TokenType::integer, "1"},
        {"(* Olá *) 1", TokenType::integer, "1"},
        {"(* привет *) 1", TokenType::integer, "1"},
        {"(* こんにちは *) 1", TokenType::integer, "1"},

        // // Identifiers
        // {"a", TokenType::ident, "a"},
        {"liberté", TokenType::ident, "liberté"},
        {"αβγ", TokenType::ident, "αβγ"},
        {"привет", TokenType::ident, "привет"},
        {"こんにちは", TokenType::ident, "こんにちは"},

        {"a👾", TokenType::ident, "a👾"},
        {"a_👾", TokenType::ident, "a_👾"},
        {"🍎", TokenType::ident, "🍎"},
    };

    do_lexUTF8_tests(tests);
}
