//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "gtest/gtest.h"

#include "parse_test.hh"

TEST(Parser, Const) {
    std::vector<ParseTests> tests = {

        {"MODULE y; CONST x = 1; BEGIN RETURN 12; END y.",
         "MODULE y;\nCONST\nx = 1;\nBEGIN\nRETURN 12;\nEND y.", ""},
        {"MODULE y; CONST x = 1; y=2; BEGIN RETURN 12; END y.",
         "MODULE y;\nCONST\nx = 1;\ny = 2;\nBEGIN\nRETURN 12;\nEND y.", ""},

        // Errors
        {"MODULE y; x = 1; BEGIN RETURN 12; END y.", "",
         "1: Unexpected token: x - expecting BEGIN"},

    };
    do_parse_tests(tests);
}

TEST(Parser, Identifiers) {
    std::vector<ParseTests> tests = {

        {"MODULE y; CONST x = 1; BEGIN RETURN x; END y.",
         "MODULE y;\nCONST\nx = 1;\nBEGIN\nRETURN x;\nEND y.", ""},
        {"MODULE y; CONST x = 1; y=2; BEGIN RETURN x - y; END y.",
         "MODULE y;\nCONST\nx = 1;\ny = 2;\nBEGIN\nRETURN x-y;\nEND y.", ""},

        {"MODULE y; CONST x = 1; y=2; "
         "BEGIN RETURN (aa * bb) + ((zero + (dev + jones)) * 4); "
         "END y.",

         "MODULE y;\nCONST\nx = 1;\ny = 2;\nBEGIN\nRETURN  (aa*bb) + ( (zero+ "
         "(dev+jones) ) *4) ;\nEND y.",
         ""},
    };
    do_parse_tests(tests);
}

TEST(Parser, Var) {
    std::vector<ParseTests> tests = {

        {"MODULE y; VAR x : INTEGER; BEGIN RETURN 12; END y.",
         "MODULE y;\nVAR\nx: INTEGER;\nBEGIN\nRETURN 12;\nEND y.", ""},
        {"MODULE y; VAR x : INTEGER; y: INTEGER; BEGIN RETURN 12; END y.",
         "MODULE y;\nVAR\nx: INTEGER;\ny: INTEGER;\nBEGIN\nRETURN 12;\nEND y.",
         ""},
        {"MODULE y; "
         "CONST z = 1+10; "
         "VAR x : INTEGER; y: INTEGER; "
         "BEGIN RETURN 12; END y.",
         "MODULE y;\nCONST\nz = 1+10;\nVAR\nx: INTEGER;\ny: "
         "INTEGER;\nBEGIN\nRETURN 12;\nEND y.",
         ""},

        // Errors
        {"MODULE y; VAR x : INTEGER BEGIN RETURN 12; END y.", "",
         "1: Unexpected token: BEGIN - expecting semicolon"},
        {"MODULE y; VAR : INTEGER;  BEGIN RETURN 12; END y.", "",
         "1: Unexpected token: : - expecting BEGIN"},

    };
    do_parse_tests(tests);
}

TEST(Parser, Assignment) {
    std::vector<ParseTests> tests = {

        {"MODULE y; VAR x : INTEGER; BEGIN x := 12; END y.",
         "MODULE y;\nVAR\nx: INTEGER;\nBEGIN\nx := 12;\nEND y.", ""},
        {"MODULE y; VAR x : INTEGER; y: INTEGER; BEGIN "
         "x := 3; y := x + 5; END y.",
         "MODULE y;\nVAR\nx: INTEGER;\ny: INTEGER;\nBEGIN\nx := 3;\ny := "
         "x+5;\nEND y.",
         ""},
        {"MODULE y; "
         "CONST z = 1+10; "
         "VAR x : INTEGER; y: INTEGER; "
         "BEGIN x := z * (2 + z); END y.",
         "MODULE y;\nCONST\nz = 1+10;\nVAR\nx: INTEGER;\ny: "
         "INTEGER;\nBEGIN\nx := z* (2+z) ;\nEND y.",
         ""},

        // Errors
        {"MODULE y; VAR x : INTEGER; BEGIN := 2; END y.", "",
         "1: Unexpected token: :="},
        {"MODULE y; VAR x : INTEGER;  BEGIN x 12; END y.", "",
         "1: Unexpected token: integer(12)"},

    };
    do_parse_tests(tests);
}