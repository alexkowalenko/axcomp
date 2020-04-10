//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <sstream>
#include <vector>

#include <llvm/Support/FormatVariadic.h>

#include <gtest/gtest.h>

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

TEST(Lexer, Lexer1) {

    for (auto const &t : tests) {

        std::istringstream is(t.input);
        ErrorManager       errors;
        Lexer              lex(is, errors);

        try {
            auto token = lex.get_token();
            std::cout << std::string(llvm::formatv("Scan {0} get {1}", t.input,
                                                   token.val))
                      << std::endl;
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