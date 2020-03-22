//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "gtest/gtest.h"

#include "parse_test.hh"

void do_inspect_tests(std::vector<ParseTests> &tests);

TEST(Inspector, IF) {
    std::vector<ParseTests> tests = {
        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF x > 12 THEN
                x := 1;
                RETURN 1;
            END;
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nIF x > 12 THEN\nx := "
         "1;\nRETURN 1;\nEND;\nEND alpha.",
         ""},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF x = 12 THEN
                x := 1;
                RETURN 1;
            ELSIF x < 3 THEN
                RETURN 3;
            ELSE
                RETURN 2;
            END;
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nIF x = 12 THEN\nx := "
         "1;\nRETURN 1;\nELSIF x < 3 THEN\nRETURN 3;\nELSE\nRETURN "
         "2;\nEND;\nEND alpha.",
         ""},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF x = 12 THEN
                x := 1;
                RETURN 1;
            ELSIF (x < 12) OR (x > 12) THEN
                RETURN 3;
            ELSIF x > 12 THEN
                RETURN 4;
            ELSIF x + x * x > 20 THEN
                RETURN 5;
            ELSE
                RETURN 2;
            END;
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nIF x = 12 THEN\nx := "
         "1;\nRETURN 1;\nELSIF  (x < 12)  OR  (x > 12)  THEN\nRETURN 3;\nELSIF "
         "x > 12 THEN\nRETURN 4;\nELSIF x+x*x > 20 THEN\nRETURN "
         "5;\nELSE\nRETURN 2;\nEND;\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF x THEN
                x := 1;
                RETURN 1;
            END;
        END alpha.)",
         "", "4,14: IF expression must be type BOOLEAN"},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF TRUE THEN
                x := 1;
                RETURN 1;
            ELSIF 34345 + 38489 THEN
                RETURN 3;
            ELSE
                RETURN 2;
            END;
        END alpha.)",
         "", "4,14: ELSIF expression must be type BOOLEAN"},

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
            ELSIF 3747 * (x - 3) THEN
                RETURN 5;
            ELSE
                RETURN 2;
            END;
        END alpha.)",
         "", "4,14: ELSIF expression must be type BOOLEAN"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, For) {
    std::vector<ParseTests> tests = {
        {R"(MODULE e06;
            BEGIN 
            FOR i := 0 TO 9 DO 
            END;
            RETURN; 
            END e06.)",
         "MODULE e06;\nBEGIN\nFOR i := 0 TO 9 DO\nEND;\nRETURN ;\nEND e06.",
         ""},

        // Errors
        {R"(MODULE e06;
            BEGIN 
            FOR i := 0 TO TRUE DO 
            END;
            RETURN; 
            END e06.)",
         "", "3,15: FOR end expression must be numeric type"},
        {R"(MODULE e06;
            BEGIN 
            FOR i := FALSE TO TRUE DO 
            END;
            RETURN; 
            END e06.)",
         "", "3,15: FOR start expression must be numeric type"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, WHILE) {
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
            WHILE 120 DO
                x := x + 1;
            END;
            RETURN x;
        END alpha.)",
         "", "4,17: WHILE expression must be type BOOLEAN"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, REPEAT) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            REPEAT
                x := x+1;
            UNTIL x > 10;
            RETURN;
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nREPEAT\nx := x+1;\nUNTIL x > "
         "10;\nRETURN ;\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            REPEAT
                x := x+1;
            UNTIL  10;
        END alpha.)",
         "", "4,18: REPEAT expression must be type BOOLEAN"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, LOOP) {
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
    do_inspect_tests(tests);
}

TEST(Inspector, BEGIN) {
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
    do_inspect_tests(tests);
}

TEST(Inspector, Builtins) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
            VAR x : INTEGER;
            BEGIN
                x := 0;
                WHILE x < 10 DO
                    x := x + 1;
                END;
                WriteInt(x); WriteLn();
                RETURN x; 
            END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nx := 0;\nWHILE x < 10 DO\nx "
         ":= x+1;\nEND;\nWriteInt(x);\nWriteLn();\nRETURN x;\nEND alpha.",
         ""},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, Arrays) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
                VAR x : ARRAY [5] OF BOOLEAN;
                VAR y : ARRAY [5] OF ARRAY [5] OF INTEGER;

                PROCEDURE sum(a : ARRAY [3] OF BOOLEAN) : INTEGER;
                BEGIN
                    RETURN 0;
                END sum;

                BEGIN
                    RETURN 0; 
                END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY [5] OF BOOLEAN;\ny: ARRAY [5] OF ARRAY "
         "[5] OF INTEGER;\nPROCEDURE sum(a : ARRAY [3] OF BOOLEAN): "
         "INTEGER;\nBEGIN\nRETURN 0;\nEND sum.\nBEGIN\nRETURN 0;\nEND alpha."},

        // Errors

        {R"(MODULE alpha;
            VAR x : ARRAY [5] OF complex;
            BEGIN
                RETURN 0; 
            END alpha.)",
         "", "2,32: Unknown type: complex"},
        {R"(MODULE alpha;
            VAR x : ARRAY [TRUE] OF complex;
            BEGIN
                RETURN 0; 
            END alpha.)",
         "", "2,31: Unexpected token: TRUE - expecting integer"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, ArraysIndex) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
                VAR x : ARRAY [5] OF BOOLEAN;
                VAR y : ARRAY [5] OF ARRAY [5] OF INTEGER;

                BEGIN
                    RETURN x[1]; 
                END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY [5] OF BOOLEAN;\ny: ARRAY [5] OF ARRAY "
         "[5] OF INTEGER;\nPROCEDURE sum(a : ARRAY [3] OF BOOLEAN): "
         "INTEGER;\nBEGIN\nRETURN 0;\nEND sum.\nBEGIN\nRETURN 0;\nEND alpha."},

        {R"(MODULE alpha;
                VAR x : ARRAY [5] OF BOOLEAN;
                VAR y : ARRAY [5] OF ARRAY [5] OF INTEGER;

                BEGIN
                    RETURN x[1] + y[2 + 3][x[2]]; 
                END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY [5] OF BOOLEAN;\ny: ARRAY [5] OF ARRAY "
         "[5] OF INTEGER;\nPROCEDURE sum(a : ARRAY [3] OF BOOLEAN): "
         "INTEGER;\nBEGIN\nRETURN 0;\nEND sum.\nBEGIN\nRETURN 0;\nEND alpha."},

        // Errors
    };
    // do_inspect_tests(tests);
}
