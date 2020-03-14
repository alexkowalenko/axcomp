//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "gtest/gtest.h"

#include "parse_test.hh"

TEST(Parser, IF) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF TRUE THEN
                x := 1;
                RETURN 1;
            END;
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nIF TRUE THEN\nx := "
         "1;\nRETURN 1;\nEND;\nEND alpha.",
         ""},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF TRUE THEN
                x := 1;
                RETURN 1;
            ELSE
                RETURN 2;
            END;
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nIF TRUE THEN\nx := "
         "1;\nRETURN 1;\nELSE\nRETURN 2;\nEND;\nEND alpha.",
         ""},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF TRUE THEN
                x := 1;
                RETURN 1;
            ELSIF TRUE THEN
                RETURN 3;
            ELSE
                RETURN 2;
            END;
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nIF TRUE THEN\nx := "
         "1;\nRETURN 1;\nELSIF TRUE THEN\nRETURN 3;\nELSE\nRETURN "
         "2;\nEND;\nEND alpha.",
         ""},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF TRUE THEN
                x := 1;
                RETURN 1;
            ELSIF TRUE THEN
                RETURN 3;
            ELSIF TRUE THEN
                RETURN 4;
            ELSIF TRUE THEN
                RETURN 5;
            ELSE
                RETURN 2;
            END;
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nIF TRUE THEN\nx := "
         "1;\nRETURN 1;\nELSIF TRUE THEN\nRETURN 3;\nELSIF TRUE THEN\nRETURN "
         "4;\nELSIF TRUE THEN\nRETURN 5;\nELSE\nRETURN 2;\nEND;\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF TRUE 
                x := 1;
                RETURN 1;
            END;
        END alpha.)",
         "", "5,17: Unexpected token: x - expecting THEN"},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF TRUE THEN
                x := 1;
                RETURN 1;
            ;
        END alpha.)",
         "", "7,13: Unexpected token: semicolon"},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF TRUE THEN
                x := 1;
                RETURN 1;
            ELSIF TRUE 
                RETURN 3;
            ELSE
                RETURN 2;
            END;
        END alpha.)",
         "", "8,22: Unexpected token: RETURN - expecting THEN"},
    };
    do_parse_tests(tests);
}

TEST(Parser, FOR) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            FOR i := 0 TO 10 DO
                x := x + i;
            END;
            RETURN x;
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nFOR i := 0 TO 10 DO\nx := "
         "x+i;\nEND;\nRETURN x;\nEND alpha.",
         ""},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            FOR i := 0 TO 19 BY 2 DO
                x := x + i;
            END;
            RETURN x;
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nFOR i := 0 TO 19 BY 2 DO\nx "
         ":= x+i;\nEND;\nRETURN x;\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            FOR i 0 TO 19 BY 2 DO
                x := x + i;
            END;
            RETURN x;
        END alpha.)",
         "", "4,19: Unexpected token: integer(0) - expecting :="},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            FOR i := 0 19 BY 2 DO
                x := x + i;
            END;
            RETURN x;
        END alpha.)",
         "", "4,25: Unexpected token: integer(19) - expecting TO"},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            FOR i := 0 TO 19 2 DO
                x := x + i;
            END;
            RETURN x;
        END alpha.)",
         "", "4,30: Unexpected token: integer(2) - expecting DO"},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            FOR i := 0 TO 19 BY DO
                x := x + i;
            END;
            RETURN x;
        END alpha.)",
         "", "4,34: Unexpected token: DO"},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            FOR i := 0 TO 19 BY 2
                x := x + i;
            END;
            RETURN x;
        END alpha.)",
         "", "5,17: Unexpected token: x - expecting DO"},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            FOR i := 0 TO 19 BY 2 DO
                x := x + i;
            RETURN x;
        END alpha.)",
         "", "7,17: Unexpected token: alpha - expecting semicolon"},
    };
    do_parse_tests(tests);
}

TEST(Parser, WHILE) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            WHILE x < 10 DO
                x := x + 1;
            END;
            RETURN x;
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nWHILE x < 10 DO\nx := "
         "x+1;\nEND;\nRETURN x;\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            WHILE x < 10
                x := x + 1;
            END;
            RETURN x;
        END alpha.)",
         "", "5,17: Unexpected token: x - expecting DO"},
    };
    do_parse_tests(tests);
}

TEST(Parser, REPEAT) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            REPEAT
                x := x+1;
            UNTIL x > 10;
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nREPEAT\nx := x+1;\nUNTIL x > "
         "10;\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            REPEAT
                x := x+1;
             x > 10;
        END alpha.)",
         "", "6,16: Unexpected token: >"},
    };
    do_parse_tests(tests);
}

TEST(Parser, LOOP) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            LOOP
                x := x + 1;
                EXIT;
            END;
            RETURN x;
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nLOOP\nx := "
         "x+1;\nEXIT;\nEND;\nRETURN x;\nEND alpha.",
         ""},
    };
    do_parse_tests(tests);
}

TEST(Parser, BEGIN) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            BEGIN
                x := x + 1;
                EXIT;
            END;
            RETURN x;
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nBEGIN\nx := "
         "x+1;\nEXIT;\nEND;\nRETURN x;\nEND alpha.",
         ""},
    };
    do_parse_tests(tests);
}
