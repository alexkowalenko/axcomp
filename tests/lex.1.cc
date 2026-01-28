//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "parse_test.hh"

#include "gtest/gtest.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wshadow"
#include "error.hh"
#include "lexer.hh"
#include "options.hh"
#include "token.hh"
#pragma clang diagnostic pop

#pragma clang diagnostic ignored "-Wmissing-field-initializers"

using namespace ax;

TEST(Location, Basic) {
    EXPECT_EQ(std::string(Location{0, 0}), "[X]");
    EXPECT_EQ(std::string(Location{1, 2}), "[1,2]");
}

TEST(Lexer, Null) {

    LexerUTF8 lex(std::string{}, ErrorManager{});

    auto token = lex.get_token();
    EXPECT_EQ(token.type, TokenType::Eof);

    auto loc = lex.get_location();
    EXPECT_EQ(loc.line_no, 1);
    EXPECT_EQ(loc.char_pos, 0);
}

TEST(Lexer, Exception) {

    const ErrorManager errors;
    LexerUTF8          lex("x", errors);

    try {
        auto token = lex.get_token();
    } catch (LexicalException &l) {
        EXPECT_EQ(l.error_msg(), "1: Unknown character x");
    }
}

TEST(Lexer, Whitespace) {

    const ErrorManager errors;
    LexerUTF8          lex(" \n\t", errors);

    auto token = lex.get_token();
    EXPECT_EQ(token.type, TokenType::Eof);
}

TEST(Lexer, Digit) {

    const ErrorManager errors;
    LexerUTF8          lex("12\n", errors);

    auto token = lex.get_token();
    std::cout << "val = " << token.val << '\n';
    EXPECT_EQ(token.type, TokenType::INTEGER);
    EXPECT_EQ(token.val, "12");

    auto loc = lex.get_location();
    EXPECT_EQ(loc.line_no, 1);
    EXPECT_EQ(loc.char_pos, 2);
}

TEST(Lexer, Identifiers) {
    std::vector<LexTests> tests = {
        // identifiers
        {"a", TokenType::IDENT, "a"},
        {"a1", TokenType::IDENT, "a1"},
        {"a1z", TokenType::IDENT, "a1z"},
        {"IsAlpha", TokenType::IDENT, "IsAlpha"},
        {"is_digit", TokenType::IDENT, "is_digit"},
    };
    do_lex_tests(tests);
}

TEST(Lexer, Numbers) {
    std::vector<LexTests> myTests = {
        {"1", TokenType::INTEGER, "1"},
        {"\n1", TokenType::INTEGER, "1"},
        {"\n12", TokenType::INTEGER, "12"},
        {"\n 1234567890", TokenType::INTEGER, "1234567890"},
        // hex numbers
        {"0dH", TokenType::HEXINTEGER, "0d"},
        {"0cafebabeH", TokenType::HEXINTEGER, "0cafebabe"},
    };
    do_lex_tests(myTests);
}

TEST(Lexer, Symbols) {
    std::vector<LexTests> myTests = {
        {"\n;", TokenType::SEMICOLON, ";"}, {";", TokenType::SEMICOLON, ";"},
        {".", TokenType::PERIOD, "."},      {",", TokenType::COMMA, ","},
        {"+", TokenType::PLUS, "+"},        {"-", TokenType::DASH, "-"},
        {"*", TokenType::ASTÉRIX, "*"},     {"(", TokenType::L_PAREN, "("},
        {")", TokenType::R_PAREN, ")"},     {":", TokenType::COLON, ":"},
        {":=", TokenType::ASSIGN, ":="},    {"=", TokenType::EQUALS, "="},
        {"#", TokenType::HASH, "#"},        {"<", TokenType::LESS, "<"},
        {"<=", TokenType::LEQ, "<="},       {">", TokenType::GREATER, ">"},
        {">=", TokenType::GTEQ, ">="},      {"~", TokenType::TILDE, "~"},
        {"&", TokenType::AMPERSAND, "&"},   {"[", TokenType::L_BRACKET, "["},
        {"]", TokenType::R_BRACKET, "]"},   {"..", TokenType::DOTDOT, ".."},
        {"|", TokenType::BAR, "|"},         {"/", TokenType::SLASH, "/"},
        {"^", TokenType::CARET, "^"},       {"{", TokenType::L_BRACE, "{"},
        {"}", TokenType::R_BRACE, "}"},
    };
    do_lex_tests(myTests);
}

TEST(Lexer, Comments) {
    std::vector<LexTests> myTests = {
        // comments
        {"(* hello *)1", TokenType::INTEGER, "1"},
        {"(* hello *) 1", TokenType::INTEGER, "1"},
        {"(**) 1", TokenType::INTEGER, "1"},
        {"(* hello (* there! *) *)1", TokenType::INTEGER, "1"},
        // error in comment
        {"(* hello (* there! *)1", TokenType::Eof, "[1,22]: Unterminated comment"},
    };
    do_lex_tests(myTests);
}

TEST(Lexer, Chars) {
    std::vector<LexTests> myTests = {
        // chars
        {"'a'", TokenType::CHR, "", 97},
        {"12X", TokenType::HEXCHR, "12"},
        {"1F47EX", TokenType::HEXCHR, "1F47E"},
    };
    do_lex_tests(myTests);
}

TEST(Lexer, Strings) {
    std::vector<LexTests> myTests = {
        // strings
        {R"('a')", TokenType::CHR, "", 97},
        {R"("a")", TokenType::STRING, R"("a)"},
        {R"("abc")", TokenType::STRING, R"("abc)"},
        {R"("Hello there!")", TokenType::STRING, R"("Hello there!)"},
        {R"("")", TokenType::STRING, R"(")"},
        {R"('ABC')", TokenType::STRING, R"('ABC)"},
        {R"('Hello there!')", TokenType::STRING, R"('Hello there!)"},
        {R"('')", TokenType::STRING, R"(')"},
        {R"("don't")", TokenType::STRING, R"("don't)"},
        {R"('Your "problem"')", TokenType::STRING, R"('Your "problem")"},
    };
    do_lex_tests(myTests);
}

TEST(Lexer, Keywords) {
    std::vector<LexTests> myTests = {
        // keywords
        {"MODULE", TokenType::MODULE, "MODULE"},
        {"module", TokenType::MODULE, "MODULE"},
        {"Module", TokenType::IDENT, "Module"},
        {"BEGIN", TokenType::BEGIN, "BEGIN"},
        {"begin", TokenType::BEGIN, "BEGIN"},
        {"Begin", TokenType::IDENT, "Begin"},
        {"END", TokenType::END, "END"},
        {"DIV", TokenType::DIV, "DIV"},
        {"MOD", TokenType::MOD, "MOD"},
        {"CONST", TokenType::CONST, "CONST"},
        {"TYPE", TokenType::TYPE, "TYPE"},
        {"VAR", TokenType::VAR, "VAR"},
        {"RETURN", TokenType::RETURN, "RETURN"},
        {"PROCEDURE", TokenType::PROCEDURE, "PROCEDURE"},
        {"TRUE", TokenType::TRUE, "TRUE"},
        {"FALSE", TokenType::FALSE, "FALSE"},
        {"OR", TokenType::OR, "OR"},
        {"IF", TokenType::IF, "IF"},
        {"THEN", TokenType::THEN, "THEN"},
        {"ELSIF", TokenType::ELSIF, "ELSIF"},
        {"ELSE", TokenType::ELSE, "ELSE"},
        {"FOR", TokenType::FOR, "FOR"},
        {"TO", TokenType::TO, "TO"},
        {"BY", TokenType::BY, "BY"},
        {"DO", TokenType::DO, "DO"},
        {"WHILE", TokenType::WHILE, "WHILE"},
        {"REPEAT", TokenType::REPEAT, "REPEAT"},
        {"UNTIL", TokenType::UNTIL, "UNTIL"},
        {"LOOP", TokenType::LOOP, "LOOP"},
        {"EXIT", TokenType::EXIT, "EXIT"},
        {"ARRAY", TokenType::ARRAY, "ARRAY"},
        {"RECORD", TokenType::RECORD, "RECORD"},
        {"DEFINITION", TokenType::DEFINITION, "DEFINITION"},
        {"IMPORT", TokenType::IMPORT, "IMPORT"},
        {"CASE", TokenType::CASE, "CASE"},
        {"POINTER", TokenType::POINTER, "POINTER"},
        {"NIL", TokenType::NIL, "NIL"},
        {"IN", TokenType::IN, "IN"},
    };
    do_lex_tests(myTests);
}

TEST(Lexer, Directives) {
    const ErrorManager errors;
    Options            options;

    LexerUTF8 lex("<* MAIN + *>MODULE M; END M.", errors, &options);
    auto      token = lex.get_token();
    EXPECT_EQ(token.type, TokenType::MODULE);
    EXPECT_TRUE(options.output_main);

    Options options_disabled;
    LexerUTF8 lex_disabled("<* main - *>MODULE M; END M.", errors, &options_disabled);
    lex_disabled.get_token();
    EXPECT_FALSE(options_disabled.output_main);
}

TEST(Lexer, UTF8) {

    std::vector<LexTests> tests1 = {
        // comments
        {"(* χαῖρε *)1", TokenType::INTEGER, "1"},
        {"(* Olá *) 1", TokenType::INTEGER, "1"},
        {"(* привет *) 1", TokenType::INTEGER, "1"},
        {"(* こんにちは *) 1", TokenType::INTEGER, "1"},

        // // Identifiers
        {"a", TokenType::IDENT, "a"},
        {"liberté", TokenType::IDENT, "liberté"},
        {"αβγ", TokenType::IDENT, "αβγ"},
        {"привет", TokenType::IDENT, "привет"},
        {"こんにちは", TokenType::IDENT, "こんにちは"},

        {"a👾", TokenType::IDENT, "a👾"},
        {"a_👾", TokenType::IDENT, "a_👾"},
        {"🍎", TokenType::IDENT, "🍎"},

        // chars
        {"'α'", TokenType::CHR, "'α'", 945},
        {"'四''", TokenType::CHR, "'四''", 22235},
        {"'👾'", TokenType::CHR, "'👾'", 0x1F47E},

        // strings
        {R"("α")", TokenType::STRING, R"("α)"},
        {R"("χαῖρε")", TokenType::STRING, R"("χαῖρε)"},
        {R"("Ça va?")", TokenType::STRING, R"("Ça va?)"},
        {R"("привет")", TokenType::STRING, R"("привет)"},
        {R"("こんにちは")", TokenType::STRING, R"("こんにちは)"},
        {R"("👾🍎🇵🇹🍊🍌😀🏖🏄🏻‍♂️🍉")", TokenType::STRING,
         R"("👾🍎🇵🇹🍊🍌😀🏖🏄🏻‍♂️🍉)"},

        {R"('λόγος')", TokenType::STRING, R"('λόγος)"},
        {R"('χαῖρε')", TokenType::STRING, R"('χαῖρε)"},
        {R"('Ça va?')", TokenType::STRING, R"('Ça va?)"},
    };

    do_lex_tests(tests1);
}

TEST(Lexer, Real) {
    std::vector<LexTests> tests1 = {
        // integer
        {"1", TokenType::INTEGER, "1"},
        {"1.", TokenType::INTEGER, "1"}, // . has to be followed by a digit to be a real

        // float
        {"12.0", TokenType::REAL, "12.0"},
        {"1.2", TokenType::REAL, "1.2"},
        {"1.23", TokenType::REAL, "1.23"},
        {"0.123", TokenType::REAL, "0.123"},

        // exponential
        {"1.0E1", TokenType::REAL, "1.0E1"},
        {"12.0E+2", TokenType::REAL, "12.0E+2"},
        {"1.2D-3", TokenType::REAL, "1.2D-3"},
        {"1.23E+45", TokenType::REAL, "1.23E+45"},
        {"0.123D-12", TokenType::REAL, "0.123D-12"},
    };

    do_lex_tests(tests1);
}
