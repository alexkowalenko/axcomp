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

TEST(Parser, Identifiers) {
    std::vector<ParseTests> tests = {

        {"MODULE y; CONST x = 1; BEGIN x; END y.",
         "MODULE y;\nCONST\nx = 1;\nBEGIN\nx;\nEND y.", ""},
        {"MODULE y; CONST x = 1; y=2; BEGIN x - y; END y.",
         "MODULE y;\nCONST\nx = 1;\ny = 2;\nBEGIN\nx-y;\nEND y.", ""},

        {"MODULE y; CONST x = 1; y=2; "
         "BEGIN (aa * bb) + ((zero + (dev + jones)) * 4); "
         "END y.",

         "MODULE y;\nCONST\nx = 1;\ny = 2;\nBEGIN\n (aa*bb) + ( (zero+ "
         "(dev+jones) ) *4) ;\nEND y.",
         ""},
    };
    do_parse_tests(tests);
}

TEST(Parser, Var) {
    std::vector<ParseTests> tests = {

        {"MODULE y; VAR x : INTEGER; BEGIN 12; END y.",
         "MODULE y;\nVAR\nx: INTEGER;\nBEGIN\n12;\nEND y.", ""},
        {"MODULE y; VAR x : INTEGER; y: INTEGER; BEGIN 12; END y.",
         "MODULE y;\nVAR\nx: INTEGER;\ny: INTEGER;\nBEGIN\n12;\nEND y.", ""},
        {"MODULE y; "
         "CONST z = 1; "
         "VAR x : INTEGER; y: INTEGER; "
         "BEGIN 12; END y.",
         "MODULE y;\nCONST\nz = 1;\nVAR\nx: INTEGER;\ny: "
         "INTEGER;\nBEGIN\n12;\nEND y.",
         ""},

        // Errors
        {"MODULE y; VAR x : INTEGER BEGIN 12; END y.", "",
         "1: Unexpected token: BEGIN - expecting semicolon"},
        {"MODULE y; VAR : INTEGER;  BEGIN 12; END y.", "",
         "1: Unexpected token: : - expecting BEGIN"},

    };
    do_parse_tests(tests);
}