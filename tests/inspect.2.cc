//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "gtest/gtest.h"

#include "parse_test.hh"

TEST(Inspector, IF) {
    std::vector<ParseTests> tests = {
        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF x > 12 THEN
                x := 1;
                RETURN 1
            END
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nIF x > 12 THEN\nx := "
         "1;\nRETURN 1\nEND\nEND alpha.",
         ""},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF x = 12 THEN
                x := 1;
                RETURN 1
            ELSIF x < 3 THEN
                RETURN 3
            ELSE
                RETURN 2
            END
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nIF x = 12 THEN\nx := "
         "1;\nRETURN 1\nELSIF x < 3 THEN\nRETURN 3\nELSE\nRETURN "
         "2\nEND\nEND alpha.",
         ""},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF x = 12 THEN
                x := 1;
                RETURN 1
            ELSIF (x < 12) OR (x > 12) THEN
                RETURN 3
            ELSIF x > 12 THEN
                RETURN 4
            ELSIF x + x * x > 20 THEN
                RETURN 5
            ELSE
                RETURN 2
            END
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nIF x = 12 THEN\nx := "
         "1;\nRETURN 1\nELSIF  (x < 12)  OR  (x > 12)  THEN\nRETURN 3\nELSIF "
         "x > 12 THEN\nRETURN 4\nELSIF x+x*x > 20 THEN\nRETURN "
         "5\nELSE\nRETURN 2\nEND\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF x THEN
                x := 1;
                RETURN 1
            END
        END alpha.)",
         "", "4,14: IF expression must be type BOOLEAN"},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF TRUE THEN
                x := 1;
                RETURN 1
            ELSIF 34345 + 38489 THEN
                RETURN 3
            ELSE
                RETURN 2
            END
        END alpha.)",
         "", "4,14: ELSIF expression must be type BOOLEAN"},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF TRUE THEN
                x := 1;
                RETURN 1
            ELSIF TRUE THEN
                RETURN 3
            ELSIF TRUE THEN
                RETURN 4
            ELSIF 3747 * (x - 3) THEN
                RETURN 5
            ELSE
                RETURN 2
            END
        END alpha.)",
         "", "4,14: ELSIF expression must be type BOOLEAN"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, FOR) {
    std::vector<ParseTests> tests = {
        {R"(MODULE e06;
            VAR i: INTEGER;
            BEGIN 
            FOR i := 0 TO 9 DO
                RETURN i
            END;
            RETURN
            END e06.)",
         "MODULE e06;\nVAR\ni: INTEGER;\nBEGIN\nFOR i := 0 TO 9 DO\nRETURN i\nEND;\nRETURN \nEND "
         "e06.",
         ""},

        // Errors
        {R"(MODULE e06;
            VAR i: INTEGER;
            BEGIN 
            FOR i := 0 TO TRUE DO 
                RETURN i
            END;
            RETURN
            END e06.)",
         "", "4,15: FOR end expression must be numeric type"},

        {R"(MODULE e06;
            VAR i: INTEGER;
            BEGIN 
            FOR i := FALSE TO TRUE DO 
                RETURN i
            END;
            RETURN
            END e06.)",
         "", "4,15: FOR start expression must be numeric type"},

        {R"(MODULE e06;
            BEGIN 
            FOR i := 0 TO 9 DO
                RETURN i
            END;
            RETURN
            END e06.)",
         "", "3,15: FOR index variable i not defined"},

        {R"(MODULE For2;
            TYPE
                T = INTEGER;
            BEGIN
                FOR T := 1 TO 8 DO
                END;
            END For2.)",
         "", "5,19: FOR index variable T not defined"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, WHILE) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            WHILE x < 10 DO
                x := x + 1
            END;
            RETURN x
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nWHILE x < 10 DO\nx := "
         "x+1\nEND;\nRETURN x\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            WHILE 120 DO
                x := x + 1
            END;
            RETURN x
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
                x := x+1
            UNTIL x > 10;
            RETURN
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nREPEAT\nx := x+1\nUNTIL x > "
         "10;\nRETURN \nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            REPEAT
                x := x+1
            UNTIL  10
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
                EXIT
            END;
            RETURN x
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nLOOP\nx := "
         "x+1;\nEXIT\nEND;\nRETURN x\nEND alpha.",
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
                EXIT
            END;
            RETURN x
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nBEGIN\nx := "
         "x+1;\nEXIT\nEND;\nRETURN x\nEND alpha.",
         ""},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, CASE) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
            VAR x : INTEGER;
            BEGIN
                CASE x OF
                    1 : x := 1;
                |   2 : x := 2;
                |   3, 4, 5: x := 5;
                ELSE
                    x := 0;
                END;
                RETURN 0;
            END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nCASE x OF\n1 : x := 1;\n| 2 : x := 2;\n| 3, 4, "
         "5 : x := 5;\nELSE\nx := 0\nEND;\nRETURN 0\nEND alpha.",
         ""},

        {R"(MODULE alpha;
            VAR x : INTEGER;
                c : CHAR;
            BEGIN
                CASE c OF
                    'a' : x := 1;
                |   'A' : x := 2;
                |   'B', 'b', 'C': x := 5;
                ELSE
                    x := 0;
                END;
                RETURN 0;
            END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nc: CHAR;\nBEGIN\nCASE c OF\n'a' : x := 1;\n| 'A' : x "
         ":= 2;\n| 'B', 'b', 'C' : x := 5;\nELSE\nx := 0\nEND;\nRETURN 0\nEND alpha.",
         ""},

        {R"(MODULE beta;
            VAR x :INTEGER;

            BEGIN
                CASE x OF
                    1..2 : x := 1;
                |   5, 6..8, 9 :  x := 5;
                |   10..11, 12..15, 16 : x := 10;
                ELSE
                    x := 20;
                END
                RETURN 0
            END beta.)",
         "MODULE beta;\nVAR\nx: INTEGER;\nBEGIN\nCASE x OF\n1..2 : x := 1;\n| 5, 6..8, 9 : x := "
         "5;\n| 10..11, 12..15, 16 : x := 10;\nELSE\nx := 20\nEND;\nRETURN 0\nEND beta.",
         ""},

        {R"(MODULE alpha;
            VAR c : CHAR;
            BEGIN
                CASE c OF
                    'A' : c := 'a';
                |   'B', 'C' : c := 'b';
                |   'D'..'F' : c := 'd';
                ELSE
                    c := 'g';
                END
                RETURN 0;
            END alpha.)",
         "MODULE alpha;\nVAR\nc: CHAR;\nBEGIN\nCASE c OF\n'A' : c := 'a';\n| 'B', 'C' : c := "
         "'b';\n| 'D'..'F' : c := 'd';\nELSE\nc := 'g'\nEND;\nRETURN 0\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
            VAR x : INTEGER;
            BEGIN
                CASE x OF
                    1 : x := 1;
                |   's' : x := 2;
                |   3, 4, 5: x := 5;
                ELSE
                    x := 0;
                END;
                RETURN 0;
            END alpha.)",
         "",
         "6,23: CASE expression mismatch type CHAR does not match CASE expression type INTEGER"},

        {R"(MODULE alpha;
            VAR x : INTEGER;
                c : CHAR;
            BEGIN
                CASE c OF
                    1 : x := 1;
                |   'A' : x := 2;
                |   'B', 'b', 'C': x := 5;
                ELSE
                    x := 0;
                END;
                RETURN 0;
            END alpha.)",
         "",
         "6,21: CASE expression mismatch type INTEGER does not match CASE expression type CHAR"},

        {R"(MODULE beta;
            VAR x :INTEGER;
            BEGIN
                CASE x OF
                    'a'..2 : x := 1;
                END
                RETURN 0
            END beta.)",
         "",
         "5,25: CASE expression range mismatch first type CHAR does not match CASE expression "
         "type INTEGER"},

        {R"(MODULE beta;
            VAR x :INTEGER;
            BEGIN
                CASE x OF
                    1..'z' : x := 1;
                END
                RETURN 0
            END beta.)",
         "",
         "5,23: CASE expression range mismatch last type CHAR does not match CASE expression type "
         "INTEGER"},

        {R"(MODULE alpha;
            VAR c : CHAR;
            BEGIN
                CASE c OF
                |   1..'F' : c := 'd';
                END
                RETURN 0;
            END alpha.)",
         "",
         "5,23: CASE expression range mismatch first type INTEGER does not match CASE expression "
         "type CHAR"},

        {R"(MODULE alpha;
            VAR c : CHAR;
            BEGIN
                CASE c OF
                |   'D'..99 : c := 'd';
                END
                RETURN 0;
            END alpha.)",
         "",
         "5,25: CASE expression range mismatch last type INTEGER does not match CASE expression "
         "type CHAR"},

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
                    x := x + 1
                END;
                WriteInt(x); WriteLn();
                RETURN x
            END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nx := 0;\nWHILE x < 10 DO\nx "
         ":= x+1\nEND;\nWriteInt(x);\nWriteLn();\nRETURN x\nEND alpha.",
         ""},
    };
    do_inspect_tests(tests);
}
