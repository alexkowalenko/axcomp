//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "gtest/gtest.h"

#include "parse_test.hh"

TEST(Parser, Comments) {
    std::vector<ParseTests> tests = {

        {"(*hello*) MODULE y; BEGIN RETURN 12 END y.", "MODULE y;\nBEGIN\nRETURN 12\nEND y.", ""},

        {"(*hello*) MODULE (* hello *) y; (* hello *)BEGIN (* hello *) RETURN "
         "12 (* "
         "hello *)END y.(* hello *)",
         "MODULE y;\nBEGIN\nRETURN 12\nEND y.", ""},

        {"(*hello*) MODULE (* hello *) y; (* hello *)BEGIN RETURN  (* hello "
         "12; 24; *) "
         "36 (*hello *) END y.(* hello *)",
         "MODULE y;\nBEGIN\nRETURN 36\nEND y.", ""},

        {"(*hello*) MODULE (* hello *) y; (* hello *)BEGIN RETURN  (* hello (* "
         "12; 24; "
         "*) *) "
         "36 (*hello *) END y.(* hello *)",
         "MODULE y;\nBEGIN\nRETURN 36\nEND y.", ""},

    };
    do_parse_tests(tests);
}

TEST(Parser, Module) {
    std::vector<ParseTests> tests = {

        {"MODULE y; BEGIN RETURN 12 END y.", "MODULE y;\nBEGIN\nRETURN 12\nEND y.", ""},
        {"MODULE y; BEGIN RETURN 12; RETURN 24 END y.",
         "MODULE y;\nBEGIN\nRETURN 12;\nRETURN 24\nEND y.", ""},
        {"MODULE y; BEGIN RETURN END y.", "MODULE y;\nBEGIN\nRETURN \nEND y.", ""},
        {"MODULE y; BEGIN END y.", "MODULE y;\nEND y.", ""},
        {"MODULE y; END y.", "MODULE y;\nEND y.", ""},

        // Errors
        {"y; BEGIN RETURN 12 END y.", "", "1,1: Unexpected token: y - expecting MODULE"},
        {"MODULE ; BEGIN RETURN 12 END y.", "",
         "1,8: Unexpected token: semicolon - expecting indent"},
        {"MODULE y BEGIN RETURN 12 END y.", "",
         "1,14: Unexpected token: BEGIN - expecting semicolon"},
        {"MODULE y; RETURN 12 END y.", "", "1,16: Unexpected token: RETURN - expecting END"},

        {"MODULE y; BEGIN RETURN 12 y.", "", "1,29: Unexpected token: EOF - expecting indent"},
        {"MODULE y; BEGIN RETURN 12 END .", "",
         "1,31: Unexpected token: period - expecting indent"},
        {"MODULE y; BEGIN RETURN 12 END y", "", "1,32: Unexpected token: EOF - expecting period"},

        {"MODULE x; BEGIN RETURN 12 END y.", "",
         "1,31: END identifier name: y doesn't match module name: x"},
    };
    do_parse_tests(tests);
}

TEST(Parser, Logic) {
    std::vector<ParseTests> tests = {

        {"MODULE y; BEGIN RETURN 1 = 1 END y.", "MODULE y;\nBEGIN\nRETURN 1 = 1\nEND y.", ""},
        {"MODULE y; BEGIN RETURN 1 # 1 END y.", "MODULE y;\nBEGIN\nRETURN 1 # 1\nEND y.", ""},
        {"MODULE y; BEGIN RETURN (1 < 1) = (1 > 1) END y.",
         "MODULE y;\nBEGIN\nRETURN  (1 < 1)  =  (1 > 1) \nEND y.", ""},
        {"MODULE y; BEGIN RETURN (1 >= 1) # (1 <= 1) END y.",
         "MODULE y;\nBEGIN\nRETURN  (1 >= 1)  #  (1 <= 1) \nEND y.", ""},

        {"MODULE y; BEGIN RETURN 1 OR 1 END y.", "MODULE y;\nBEGIN\nRETURN 1 OR 1\nEND y.", ""},
        {"MODULE y; BEGIN RETURN 1 & 1 END y.", "MODULE y;\nBEGIN\nRETURN 1 & 1\nEND y.", ""},
        {"MODULE y; BEGIN RETURN ~ TRUE END y.", "MODULE y;\nBEGIN\nRETURN ~ TRUE\nEND y.", ""},
        {"MODULE y; BEGIN RETURN ~ ~ TRUE END y.", "MODULE y;\nBEGIN\nRETURN ~ ~ TRUE\nEND y.",
         ""},
        {"MODULE y; BEGIN RETURN ~ TRUE & ~ FALSE END y.",
         "MODULE y;\nBEGIN\nRETURN ~ TRUE & ~ FALSE\nEND y.", ""},

    };
    do_parse_tests(tests);
}

TEST(Parser, Plus) {
    std::vector<ParseTests> tests = {

        {"MODULE y; BEGIN RETURN - 1 END y.", "MODULE y;\nBEGIN\nRETURN -1\nEND y.", ""},
        {"MODULE y; BEGIN RETURN + 1 END y.", "MODULE y;\nBEGIN\nRETURN +1\nEND y.", ""},

        {"MODULE y; BEGIN RETURN 1 + 1 END y.", "MODULE y;\nBEGIN\nRETURN 1+1\nEND y.", ""},
        {"MODULE y; BEGIN RETURN 2 - 2 END y.", "MODULE y;\nBEGIN\nRETURN 2-2\nEND y.", ""},
        {"MODULE y; BEGIN RETURN 1 + 2 - 3 END y.", "MODULE y;\nBEGIN\nRETURN 1+2-3\nEND y.", ""},
        {"MODULE y; BEGIN RETURN 1 + 2 - 3 + 4 END y.", "MODULE y;\nBEGIN\nRETURN 1+2-3+4\nEND y.",
         ""},

        {"MODULE y; BEGIN RETURN - 1 + 2 END y.", "MODULE y;\nBEGIN\nRETURN -1+2\nEND y.", ""},

        // Errors
        {"MODULE y; BEGIN RETURN -  END y.", "", "1,29: Unexpected token: END"},
        {"MODULE y; BEGIN RETURN 1 - END y.", "", "1,30: Unexpected token: END"},
        {"MODULE y; BEGIN RETURN - 1 + 2 +  END y.", "", "1,37: Unexpected token: END"},
    };
    do_parse_tests(tests);
}

TEST(Parser, Mult) {
    std::vector<ParseTests> tests = {

        {"MODULE y; BEGIN RETURN 2 * 2 END y.", "MODULE y;\nBEGIN\nRETURN 2*2\nEND y.", ""},
        {"MODULE y; BEGIN RETURN 4 DIV 2 END y.", "MODULE y;\nBEGIN\nRETURN 4 DIV 2\nEND y.", ""},
        {"MODULE y; BEGIN RETURN 7 MOD 2 END y.", "MODULE y;\nBEGIN\nRETURN 7 MOD 2\nEND y.", ""},

        {"MODULE y; BEGIN RETURN 1 * 2 * 3 * 4 END y.", "MODULE y;\nBEGIN\nRETURN 1*2*3*4\nEND y.",
         ""},

        {"MODULE y; BEGIN RETURN 3 * 3 + 4 * 4 END y.", "MODULE y;\nBEGIN\nRETURN 3*3+4*4\nEND y.",
         ""},

        // Errors
        {"MODULE y; BEGIN RETURN *  END y.", "", "1,24: Unexpected token: *"},
        {"MODULE y; BEGIN RETURN 1 MOD END y.", "", "1,32: Unexpected token: END"},
        {"MODULE y; BEGIN RETURN - 1 + 2 DIV  END y.", "", "1,39: Unexpected token: END"},
    };
    do_parse_tests(tests);
}

TEST(Parser, Parentheses) {
    std::vector<ParseTests> tests = {

        {"MODULE y; BEGIN RETURN (2) END y.", "MODULE y;\nBEGIN\nRETURN  (2) \nEND y.", ""},
        {"MODULE y; BEGIN RETURN (2 + 1) END y.", "MODULE y;\nBEGIN\nRETURN  (2+1) \nEND y.", ""},
        {"MODULE y; BEGIN RETURN (2 + (4 * 3)) END y.",
         "MODULE y;\nBEGIN\nRETURN  (2+ (4*3) ) \nEND y.", ""},
        {"MODULE y; BEGIN RETURN (2 + (4 * (3 DIV 1))) END y.",
         "MODULE y;\nBEGIN\nRETURN  (2+ (4* (3 DIV 1) ) ) \nEND y.", ""},

        // Errors
        {"MODULE y; BEGIN RETURN (2 ; END y.", "",
         "1,27: Unexpected token: semicolon - expecting )"},
        {"MODULE y; BEGIN RETURN (2 + 4) * (3 DIV 1)) ; END y.", "", "1,43: Unexpected token: )"},
    };
    do_parse_tests(tests);
}

TEST(Parser, HexDigits) {
    std::vector<ParseTests> tests = {
        {R"(MODULE x;
            BEGIN
                RETURN 0dH
            END x.)",
         "MODULE x;\nBEGIN\nRETURN 0dH\nEND x.", ""},

        {R"(MODULE x;
            BEGIN
                RETURN 0cafeH + 0babeH * 0deadH;
            END x.)",
         "MODULE x;\nBEGIN\nRETURN 0cafeH+0babeH*0deadH\nEND x.", ""},
    };
    do_parse_tests(tests);
}

TEST(Parser, REAL) {
    std::vector<ParseTests> tests = {
        {R"(MODULE alpha; (* REAL *)
            CONST c = 1.2;
                d = 1.2E+2;
                f = 2.3D+2;
                h = 0.23D-8;
            BEGIN
                RETURN 0;
            END alpha.)",
         "MODULE alpha;\nCONST\nc = 1.2;\nd = 1.2E+2;\nf = 2.3D+2;\nh = 0.23D-8;\nBEGIN\nRETURN "
         "0\nEND alpha.",
         ""},

        // Errors

        {R"(MODULE alpha; (* REAL *)
            CONST c = .E;
            BEGIN
                RETURN 0;
            END alpha.)",
         "", "2,23: Unexpected token: period"},
    };
    do_parse_tests(tests);
}
