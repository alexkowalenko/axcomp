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

        {"(*hello*) MODULE y; BEGIN RETURN 12; END y.",
         "MODULE y;\nBEGIN\nRETURN 12;\nEND y.", ""},
        {"(*hello*) MODULE (* hello *) y; (* hello *)BEGIN (* hello *) RETURN "
         "12; (* "
         "hello *)END y.(* hello *)",
         "MODULE y;\nBEGIN\nRETURN 12;\nEND y.", ""},
        {"(*hello*) MODULE (* hello *) y; (* hello *)BEGIN RETURN  (* hello "
         "12; 24; *) "
         "36; (*hello *) END y.(* hello *)",
         "MODULE y;\nBEGIN\nRETURN 36;\nEND y.", ""},
        {"(*hello*) MODULE (* hello *) y; (* hello *)BEGIN RETURN  (* hello (* "
         "12; 24; "
         "*) *) "
         "36; (*hello *) END y.(* hello *)",
         "MODULE y;\nBEGIN\nRETURN 36;\nEND y.", ""},

    };
    do_parse_tests(tests);
}

TEST(Parser, Module) {
    std::vector<ParseTests> tests = {

        {"MODULE y; BEGIN RETURN 12; END y.",
         "MODULE y;\nBEGIN\nRETURN 12;\nEND y.", ""},
        {"MODULE y; BEGIN RETURN 12; RETURN 24; END y.",
         "MODULE y;\nBEGIN\nRETURN 12;\nRETURN 24;\nEND y.", ""},

        // Errors
        {"y; BEGIN RETURN 12; END y.", "",
         "1: Unexpected token: y - expecting MODULE"},
        {"MODULE ; BEGIN RETURN 12; END y.", "",
         "1: Unexpected token: semicolon - expecting indent"},
        {"MODULE y BEGIN RETURN 12; END y.", "",
         "1: Unexpected token: BEGIN - expecting semicolon"},
        {"MODULE y; RETURN 12; END y.", "",
         "1: Unexpected token: RETURN - expecting BEGIN"},
        {"MODULE y; BEGIN RETURN ; END y.", "",
         "1: Unexpected token: semicolon"},
        {"MODULE y; BEGIN RETURN 12; y.", "", "1: Unexpected token: period"},
        {"MODULE y; BEGIN RETURN 12; END .", "",
         "1: Unexpected token: period - expecting indent"},
        {"MODULE y; BEGIN RETURN 12; END y", "",
         "1: Unexpected token: EOF - expecting period"},

        {"MODULE x; BEGIN RETURN 12; END y.", "",
         "1: END identifier name: y doesn't match module name: x"},
    };
    do_parse_tests(tests);
}

TEST(Parser, Plus) {
    std::vector<ParseTests> tests = {

        {"MODULE y; BEGIN RETURN - 1; END y.",
         "MODULE y;\nBEGIN\nRETURN -1;\nEND y.", ""},
        {"MODULE y; BEGIN RETURN + 1; END y.",
         "MODULE y;\nBEGIN\nRETURN +1;\nEND y.", ""},

        {"MODULE y; BEGIN RETURN 1 + 1; END y.",
         "MODULE y;\nBEGIN\nRETURN 1+1;\nEND y.", ""},
        {"MODULE y; BEGIN RETURN 2 - 2; END y.",
         "MODULE y;\nBEGIN\nRETURN 2-2;\nEND y.", ""},
        {"MODULE y; BEGIN RETURN 1 + 2 - 3; END y.",
         "MODULE y;\nBEGIN\nRETURN 1+2-3;\nEND y.", ""},
        {"MODULE y; BEGIN RETURN 1 + 2 - 3 + 4; END y.",
         "MODULE y;\nBEGIN\nRETURN 1+2-3+4;\nEND y.", ""},

        {"MODULE y; BEGIN RETURN - 1 + 2; END y.",
         "MODULE y;\nBEGIN\nRETURN -1+2;\nEND y.", ""},

        // Errors
        {"MODULE y; BEGIN RETURN - ; END y.", "",
         "1: Unexpected token: semicolon"},
        {"MODULE y; BEGIN RETURN 1 -; END y.", "",
         "1: Unexpected token: semicolon"},
        {"MODULE y; BEGIN RETURN - 1 + 2 + ; END y.", "",
         "1: Unexpected token: semicolon"},
    };
    do_parse_tests(tests);
}

TEST(Parser, Mult) {
    std::vector<ParseTests> tests = {

        {"MODULE y; BEGIN RETURN 2 * 2; END y.",
         "MODULE y;\nBEGIN\nRETURN 2*2;\nEND y.", ""},
        {"MODULE y; BEGIN RETURN 4 DIV 2; END y.",
         "MODULE y;\nBEGIN\nRETURN 4 DIV 2;\nEND y.", ""},
        {"MODULE y; BEGIN RETURN 7 MOD 2; END y.",
         "MODULE y;\nBEGIN\nRETURN 7 MOD 2;\nEND y.", ""},

        {"MODULE y; BEGIN RETURN 1 * 2 * 3 * 4; END y.",
         "MODULE y;\nBEGIN\nRETURN 1*2*3*4;\nEND y.", ""},

        {"MODULE y; BEGIN RETURN 3 * 3 + 4 * 4; END y.",
         "MODULE y;\nBEGIN\nRETURN 3*3+4*4;\nEND y.", ""},

        // Errors
        {"MODULE y; BEGIN RETURN * ; END y.", "", "1: Unexpected token: *"},
        {"MODULE y; BEGIN RETURN 1 MOD; END y.", "",
         "1: Unexpected token: semicolon"},
        {"MODULE y; BEGIN RETURN - 1 + 2 DIV ; END y.", "",
         "1: Unexpected token: semicolon"},
    };
    do_parse_tests(tests);
}

TEST(Parser, Parentheses) {
    std::vector<ParseTests> tests = {

        {"MODULE y; BEGIN RETURN (2); END y.",
         "MODULE y;\nBEGIN\nRETURN  (2) ;\nEND y.", ""},
        {"MODULE y; BEGIN RETURN (2 + 1); END y.",
         "MODULE y;\nBEGIN\nRETURN  (2+1) ;\nEND y.", ""},
        {"MODULE y; BEGIN RETURN (2 + (4 * 3)); END y.",
         "MODULE y;\nBEGIN\nRETURN  (2+ (4*3) ) ;\nEND y.", ""},
        {"MODULE y; BEGIN RETURN (2 + (4 * (3 DIV 1))); END y.",
         "MODULE y;\nBEGIN\nRETURN  (2+ (4* (3 DIV 1) ) ) ;\nEND y.", ""},

        // Errors
        {"MODULE y; BEGIN RETURN (2 ; END y.", "",
         "1: Unexpected token: semicolon - expecting )"},
        {"MODULE y; BEGIN RETURN (2 + 4) * (3 DIV 1)) ; END y.", "",
         "1: Unexpected token: ) - expecting semicolon"},
    };
    do_parse_tests(tests);
}
