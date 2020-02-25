//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <exception>
#include <sstream>

#include "gtest/gtest.h"

#include "parse_test.hh"

TEST(Parser, Comments) {
    std::vector<ParseTests> tests = {

        {"(*hello*) MODULE y; BEGIN 12; END y.",
         "MODULE y;\nBEGIN\n12;\nEND y.", ""},
        {"(*hello*) MODULE (* hello *) y; (* hello *)BEGIN (* hello *)12; (* "
         "hello *)END y.(* hello *)",
         "MODULE y;\nBEGIN\n12;\nEND y.", ""},
        {"(*hello*) MODULE (* hello *) y; (* hello *)BEGIN (* hello 12; 24; *) "
         "36; (*hello *) END y.(* hello *)",
         "MODULE y;\nBEGIN\n36;\nEND y.", ""},
        {"(*hello*) MODULE (* hello *) y; (* hello *)BEGIN (* hello (* 12; 24; "
         "*) *) "
         "36; (*hello *) END y.(* hello *)",
         "MODULE y;\nBEGIN\n36;\nEND y.", ""},

    };
    do_parse_tests(tests);
}

TEST(Parser, Module) {
    std::vector<ParseTests> tests = {

        {"MODULE y; BEGIN 12; END y.", "MODULE y;\nBEGIN\n12;\nEND y.", ""},
        {"MODULE y; BEGIN 12; 24; END y.", "MODULE y;\nBEGIN\n12;\n24;\nEND y.",
         ""},

        // Errors
        {"y; BEGIN 12; END y.", "",
         "1: Unexpected token: y - expecting MODULE"},
        {"MODULE ; BEGIN 12; END y.", "",
         "1: Unexpected token: semicolon - expecting indent"},
        {"MODULE y BEGIN 12; END y.", "",
         "1: Unexpected token: BEGIN - expecting semicolon"},
        {"MODULE y; 12; END y.", "",
         "1: Unexpected token: integer(12) - expecting BEGIN"},
        {"MODULE y; BEGIN ; END y.", "",
         "1: Unexpected token: semicolon - expecting ( or integer"},
        {"MODULE y; BEGIN 12; y.", "",
         "1: Unexpected token: EOF - expecting indent"},
        {"MODULE y; BEGIN 12; END .", "",
         "1: Unexpected token: period - expecting indent"},
        {"MODULE y; BEGIN 12; END y", "",
         "1: Unexpected token: EOF - expecting period"},

        {"MODULE x; BEGIN 12; END y.", "",
         "1: END identifier name: y doesn't match module name: x"},
    };
    do_parse_tests(tests);
}

TEST(Parser, Plus) {
    std::vector<ParseTests> tests = {

        {"MODULE y; BEGIN - 1; END y.", "MODULE y;\nBEGIN\n-1;\nEND y.", ""},
        {"MODULE y; BEGIN + 1; END y.", "MODULE y;\nBEGIN\n+1;\nEND y.", ""},

        {"MODULE y; BEGIN 1 + 1; END y.", "MODULE y;\nBEGIN\n1+1;\nEND y.", ""},
        {"MODULE y; BEGIN 2 - 2; END y.", "MODULE y;\nBEGIN\n2-2;\nEND y.", ""},
        {"MODULE y; BEGIN 1 + 2 - 3; END y.",
         "MODULE y;\nBEGIN\n1+2-3;\nEND y.", ""},
        {"MODULE y; BEGIN 1 + 2 - 3 + 4; END y.",
         "MODULE y;\nBEGIN\n1+2-3+4;\nEND y.", ""},

        {"MODULE y; BEGIN - 1 + 2; END y.", "MODULE y;\nBEGIN\n-1+2;\nEND y.",
         ""},

        // Errors
        {"MODULE y; BEGIN - ; END y.", "",
         "1: Unexpected token: semicolon - expecting ( or integer"},
        {"MODULE y; BEGIN 1 -; END y.", "",
         "1: Unexpected token: semicolon - expecting ( or integer"},
        {"MODULE y; BEGIN - 1 + 2 + ; END y.", "",
         "1: Unexpected token: semicolon - expecting ( or integer"},
    };
    do_parse_tests(tests);
}

TEST(Parser, Mult) {
    std::vector<ParseTests> tests = {

        {"MODULE y; BEGIN 2 * 2; END y.", "MODULE y;\nBEGIN\n2*2;\nEND y.", ""},
        {"MODULE y; BEGIN 4 DIV 2; END y.",
         "MODULE y;\nBEGIN\n4 DIV 2;\nEND y.", ""},
        {"MODULE y; BEGIN 7 MOD 2; END y.",
         "MODULE y;\nBEGIN\n7 MOD 2;\nEND y.", ""},

        {"MODULE y; BEGIN 1 * 2 * 3 * 4; END y.",
         "MODULE y;\nBEGIN\n1*2*3*4;\nEND y.", ""},

        {"MODULE y; BEGIN 3 * 3 + 4 * 4; END y.",
         "MODULE y;\nBEGIN\n3*3+4*4;\nEND y.", ""},

        // Errors
        {"MODULE y; BEGIN * ; END y.", "",
         "1: Unexpected token: * - expecting ( or integer"},
        {"MODULE y; BEGIN 1 MOD; END y.", "",
         "1: Unexpected token: semicolon - expecting ( or integer"},
        {"MODULE y; BEGIN - 1 + 2 DIV ; END y.", "",
         "1: Unexpected token: semicolon - expecting ( or integer"},
    };
    do_parse_tests(tests);
}

TEST(Parser, Parentheses) {
    std::vector<ParseTests> tests = {

        {"MODULE y; BEGIN (2); END y.", "MODULE y;\nBEGIN\n (2) ;\nEND y.", ""},
        {"MODULE y; BEGIN (2 + 1); END y.",
         "MODULE y;\nBEGIN\n (2+1) ;\nEND y.", ""},
        {"MODULE y; BEGIN (2 + (4 * 3)); END y.",
         "MODULE y;\nBEGIN\n (2+ (4*3) ) ;\nEND y.", ""},
        {"MODULE y; BEGIN (2 + (4 * (3 DIV 1))); END y.",
         "MODULE y;\nBEGIN\n (2+ (4* (3 DIV 1) ) ) ;\nEND y.", ""},

        // Errors
        {"MODULE y; BEGIN (2 ; END y.", "",
         "1: Unexpected token: semicolon - expecting )"},
        {"MODULE y; BEGIN (2 + 4) * (3 DIV 1)) ; END y.", "",
         "1: Unexpected token: semicolon - expecting ( or integer"},
    };
    do_parse_tests(tests);
}
