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
    {"0dH", TokenType::hexinteger, "0d"},
    {"0cafebabeH", TokenType::hexinteger, "0cafebabe"},

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
    {"..", TokenType::dotdot, ".."},
    {"|", TokenType::bar, "|"},
    {"/", TokenType::slash, "/"},
    {"^", TokenType::caret, "^"},
    {"{", TokenType::l_brace, "{"},
    {"}", TokenType::r_brace, "}"},

    // comments
    {"(* hello *)1", TokenType::integer, "1"},
    {"(* hello *) 1", TokenType::integer, "1"},
    {"(**) 1", TokenType::integer, "1"},
    {"(* hello (* there! *) *)1", TokenType::integer, "1"},
    // error in comment
    {"(* hello (* there! *)1", TokenType::eof, ""},

    // chars
    {"'a'", TokenType::chr, "", 97},
    {"12X", TokenType::hexchr, "12"},

    // strings
    {R"('a')", TokenType::chr, "", 97},
    {R"("a")", TokenType::string, R"("a)"},
    {R"("abc")", TokenType::string, R"("abc)"},
    {R"("Hello there!")", TokenType::string, R"("Hello there!)"},
    {R"("")", TokenType::string, R"(")"},
    {R"('ABC')", TokenType::string, R"('ABC)"},
    {R"('Hello there!')", TokenType::string, R"('Hello there!)"},
    {R"('')", TokenType::string, R"(')"},
    {R"("don't")", TokenType::string, R"("don't)"},
    {R"('Your "problem"')", TokenType::string, R"('Your "problem")"},

    {"1F47EX", TokenType::hexchr, "1F47E"},

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
    {"CASE", TokenType::cse, "CASE"},
    {"POINTER", TokenType::pointer, "POINTER"},
    {"NIL", TokenType::nil, "NIL"},
    {"IN", TokenType::in, "IN"},

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

        // chars
        {"'α'", TokenType::chr, "'α'", 945},
        {"'四''", TokenType::chr, "'四''", 22235},
        {"'👾'", TokenType::chr, "'👾'", 0x1F47E},

        // strings
        {R"("α")", TokenType::string, R"("α)"},
        {R"("χαῖρε")", TokenType::string, R"("χαῖρε)"},
        {R"("Ça va?")", TokenType::string, R"("Ça va?)"},
        {R"("привет")", TokenType::string, R"("привет)"},
        {R"("こんにちは")", TokenType::string, R"("こんにちは)"},
        {R"("👾🍎🇵🇹🍊🍌😀🏖🏄🏻‍♂️🍉")", TokenType::string,
         R"("👾🍎🇵🇹🍊🍌😀🏖🏄🏻‍♂️🍉)"},

        {R"('λόγος')", TokenType::string, R"('λόγος)"},
        {R"('χαῖρε')", TokenType::string, R"('χαῖρε)"},
        {R"('Ça va?')", TokenType::string, R"('Ça va?)"},
    };

    do_lexUTF8_tests(tests);
}

TEST(Lexer, REAL) {

    std::vector<LexTests> tests = {

        // integer
        {"1", TokenType::integer, "1"},
        {"1.", TokenType::integer, "1"},

        // float
        {"12.0", TokenType::real, "12.0"},
        {"1.2", TokenType::real, "1.2"},
        {"1.23", TokenType::real, "1.23"},
        {"0.123", TokenType::real, "0.123"},

        // exponentials
        {"1.0E1", TokenType::real, "1.0E1"},
        {"12.0E+2", TokenType::real, "12.0E+2"},
        {"1.2D-3", TokenType::real, "1.2D-3"},
        {"1.23E+45", TokenType::real, "1.23E+45"},
        {"0.123D-12", TokenType::real, "0.123D-12"},
    };

    do_lexUTF8_tests(tests);
}
