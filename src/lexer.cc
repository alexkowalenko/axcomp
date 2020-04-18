//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "lexer.hh"

namespace ax {

const std::unordered_map<std::string, Token> keyword_map = {
    {"MODULE", Token(TokenType::module, "MODULE")},
    {"BEGIN", Token(TokenType::begin, "BEGIN")},
    {"END", Token(TokenType::end, "END")},
    {"DIV", Token(TokenType::div, "DIV")},
    {"MOD", Token(TokenType::mod, "MOD")},
    {"CONST", Token(TokenType::cnst, "CONST")},
    {"TYPE", Token(TokenType::type, "TYPE")},
    {"VAR", Token(TokenType::var, "VAR")},
    {"RETURN", Token(TokenType::ret, "RETURN")},
    {"PROCEDURE", Token(TokenType::procedure, "PROCEDURE")},
    {"TRUE", Token(TokenType::true_k, "TRUE")},
    {"FALSE", Token(TokenType::false_k, "FALSE")},
    {"OR", Token(TokenType::or_k, "OR")},
    {"IF", Token(TokenType::if_k, "IF")},
    {"THEN", Token(TokenType::then, "THEN")},
    {"ELSIF", Token(TokenType::elsif, "ELSIF")},
    {"ELSE", Token(TokenType::else_k, "ELSE")},
    {"FOR", Token(TokenType::for_k, "FOR")},
    {"TO", Token(TokenType::to, "TO")},
    {"BY", Token(TokenType::by, "BY")},
    {"DO", Token(TokenType::do_k, "DO")},
    {"WHILE", Token(TokenType::while_k, "WHILE")},
    {"REPEAT", Token(TokenType::repeat, "REPEAT")},
    {"UNTIL", Token(TokenType::until, "UNTIL")},
    {"LOOP", Token(TokenType::loop, "LOOP")},
    {"EXIT", Token(TokenType::exit, "EXIT")},
    {"ARRAY", Token(TokenType::array, "ARRAY")},
    {"OF", Token(TokenType::of, "OF")},
    {"RECORD", Token(TokenType::record, "RECORD")},
    {"DEFINITION", Token(TokenType::definition, "DEFINITION")},
    {"IMPORT", Token(TokenType::import, "IMPORT")},
};

const std::unordered_map<char, Token> token_map = {
    {-1, Token(TokenType::eof)},
    {';', Token(TokenType::semicolon, ";")},
    {'.', Token(TokenType::period, ".")},
    {',', Token(TokenType::comma, ",")},
    {'+', Token(TokenType::plus, "+")},
    {'-', Token(TokenType::dash, "-")},
    {'*', Token(TokenType::asterisk, "*")},
    {'(', Token(TokenType::l_paren, "(")},
    {')', Token(TokenType::r_paren, ")")},
    {'=', Token(TokenType::equals, "=")},
    {'#', Token(TokenType::hash, "#")},
    {'~', Token(TokenType::tilde, "~")},
    {'&', Token(TokenType::ampersand, "&")},
    {'[', Token(TokenType::l_bracket, "[")},
    {']', Token(TokenType::r_bracket, "]")},
};

} // namespace ax