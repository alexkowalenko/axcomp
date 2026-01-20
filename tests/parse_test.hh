//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <ranges>
#include <string>
#include <vector>

#include "token.hh"

struct LexTests {
    std::string   input;
    ax::TokenType token;
    std::string   val;
    long          val_int;
};

struct ParseTests {
    std::string input;
    std::string output;
    std::string error;
};

// Lexer tester
void do_lex_tests(std::vector<LexTests> &tests);

// Parser tester
void do_parse_tests(std::vector<ParseTests> &tests);

// Inspector tester
void do_inspect_tests(std::vector<ParseTests> &tests);

// Importer tests
void do_def_tests(std::vector<ParseTests> &tests);
void do_inspect_fimport_tests(std::vector<ParseTests> &tests);
void do_defparse_tests(std::vector<ParseTests> &tests);

inline void rtrim(std::string &s) {
    s.erase(std::ranges::find_if(std::views::reverse(s),
                                 [](const int ch) { return !std::isspace(ch); })
                .base(),
            s.end());
}
