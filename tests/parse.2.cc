//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "gtest/gtest.h"

#include "parse_test.hh"

TEST(Parser, Const) {
    std::vector<ParseTests> tests = {

        {"MODULE y; CONST x = 1; BEGIN 12; END y.",
         "MODULE y;\nCONST\nx = 1;\nBEGIN\n12;\nEND y.", ""},
        {"MODULE y; CONST x = 1; y=2; BEGIN 12; END y.",
         "MODULE y;\nCONST\nx = 1;\ny = 2;\nBEGIN\n12;\nEND y.", ""},

        // Errors
        {"MODULE y; x = 1; BEGIN 12; END y.", "",
         "1: Unexpected token: x - expecting BEGIN"},

    };
    do_parse_tests(tests);
}