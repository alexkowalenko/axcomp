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
                RETURN 1
            END
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nIF TRUE THEN\nx := "
         "1;\nRETURN 1\nEND\nEND alpha.",
         ""},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF TRUE THEN
                x := 1;
                RETURN 1
            ELSE
                RETURN 2
            END
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nIF TRUE THEN\nx := "
         "1;\nRETURN 1\nELSE\nRETURN 2\nEND\nEND alpha.",
         ""},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF TRUE THEN
                x := 1;
                RETURN 1
            ELSIF TRUE THEN
                RETURN 3
            ELSE
                RETURN 2
            END
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nIF TRUE THEN\nx := "
         "1;\nRETURN 1\nELSIF TRUE THEN\nRETURN 3\nELSE\nRETURN "
         "2\nEND\nEND alpha.",
         ""},

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
            ELSIF TRUE THEN
                RETURN 5
            ELSE
                RETURN 2
            END
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nIF TRUE THEN\nx := "
         "1;\nRETURN 1\nELSIF TRUE THEN\nRETURN 3\nELSIF TRUE THEN\nRETURN "
         "4\nELSIF TRUE THEN\nRETURN 5\nELSE\nRETURN 2\nEND\nEND alpha.",
         ""},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF TRUE THEN
            ELSE
            END
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nIF TRUE THEN\nELSE\nEND\nEND alpha.", ""},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF TRUE THEN
            ELSIF TRUE THEN
            ELSIF TRUE THEN
            ELSIF TRUE THEN
            ELSE
            END
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nIF TRUE THEN\nELSIF TRUE THEN\nELSIF TRUE "
         "THEN\nELSIF TRUE THEN\nELSE\nEND\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF TRUE 
                x := 1;
                RETURN 1
            END
        END alpha.)",
         "", "5,17: Unexpected token: x - expecting THEN"},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF TRUE THEN
                x := 1;
                RETURN 1
            
        END alpha.)",
         "", "8,19: Unexpected token: EOF - expecting indent"},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF TRUE THEN
                x := 1;
                RETURN 1
            ELSIF TRUE 
                RETURN 3
            ELSE
                RETURN 2
            END
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
                x := x + i
            END;
            RETURN x
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nFOR i := 0 TO 10 DO\nx := "
         "x+i\nEND;\nRETURN x\nEND alpha.",
         ""},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            FOR i := 0 TO 19 BY 2 DO
                x := x + i
            END;
            RETURN x
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nFOR i := 0 TO 19 BY 2 DO\nx "
         ":= x+i\nEND;\nRETURN x\nEND alpha.",
         ""},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            FOR i := 0 TO 19 BY 2 DO
            END;
            RETURN x
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nFOR i := 0 TO 19 BY 2 DO\nEND;\nRETURN x\nEND "
         "alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            FOR i 0 TO 19 BY 2 DO
                x := x + i
            END;
            RETURN x
        END alpha.)",
         "", "4,19: Unexpected token: integer(0) - expecting :="},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            FOR i := 0 19 BY 2 DO
                x := x + i
            END;
            RETURN x
        END alpha.)",
         "", "4,25: Unexpected token: integer(19) - expecting TO"},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            FOR i := 0 TO 19 2 DO
                x := x + i
            END;
            RETURN x
        END alpha.)",
         "", "4,30: Unexpected token: integer(2) - expecting DO"},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            FOR i := 0 TO 19 BY DO
                x := x + i
            END;
            RETURN x
        END alpha.)",
         "", "4,34: Unexpected token: DO"},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            FOR i := 0 TO 19 BY 2
                x := x + i
            END;
            RETURN x
        END alpha.)",
         "", "5,17: Unexpected token: x - expecting DO"},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            FOR i := 0 TO 19 BY 2 DO
                x := x + i
            RETURN x
        END alpha.)",
         "", "7,19: Unexpected token: EOF - expecting indent"},
    };
    do_parse_tests(tests);
}

TEST(Parser, WHILE) {
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

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            WHILE x < 10 DO
            END;
            RETURN x
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nWHILE x < 10 DO\nEND;\nRETURN x\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            WHILE x < 10
                x := x + 1
            END;
            RETURN x
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
                x := x+1
            UNTIL x > 10
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nREPEAT\nx := x+1\nUNTIL x > "
         "10\nEND alpha.",
         ""},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            REPEAT
            UNTIL x > 10
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nREPEAT\nUNTIL x > 10\nEND alpha.", ""},

        // Errors
        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            REPEAT
                x := x+1
             x > 10
        END alpha.)",
         "", "6,16: Unexpected token: > - expecting :="},
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
                EXIT
            END;
            RETURN x
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nLOOP\nx := "
         "x+1;\nEXIT\nEND;\nRETURN x\nEND alpha.",
         ""},

        {R"(MODULE alpha;
        BEGIN
            LOOP
            END;
            RETURN x
        END alpha.)",
         "MODULE alpha;\nBEGIN\nLOOP\nEND;\nRETURN x\nEND alpha.", ""},
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
                EXIT
            END;
            RETURN x
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nBEGIN\nx := "
         "x+1;\nEXIT\nEND;\nRETURN x\nEND alpha.",
         ""},

        {R"(MODULE alpha;
        BEGIN
            BEGIN
            END;
            RETURN x
        END alpha.)",
         "MODULE alpha;\nBEGIN\nBEGIN\nEND;\nRETURN x\nEND alpha.", ""},
    };
    do_parse_tests(tests);
}

TEST(Parser, CASE) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
            BEGIN
                CASE i OF
                    1 : Out.String('One');
                |   2 : Out.String('Two');
                |   3 : Out.String('Three');
                ELSE
                    Out.String("More")
                END
                RETURN 0;
            END alpha.)",
         "MODULE alpha;\nBEGIN\nCASE i OF\n1 : Out.String('One');\n| 2 : Out.String('Two');\n| 3 "
         ": Out.String('Three');\nELSE\nOut.String(\"More\")\nEND;\nRETURN 0\nEND alpha.",
         ""},

        {R"(MODULE alpha;
            BEGIN
                CASE i OF
                    1 : Out.String('One');
                |   2 : Out.String('Two');
                |   3 : Out.String('Three');
                END
                RETURN 0;
            END alpha.)",
         "MODULE alpha;\nBEGIN\nCASE i OF\n1 : Out.String('One');\n| 2 : Out.String('Two');\n| 3 "
         ": Out.String('Three');\nEND;\nRETURN 0\nEND alpha.",
         ""},

        {R"(MODULE alpha;
            BEGIN
                CASE i OF
                |   1 : Out.String('One');
                |   2 : Out.String('Two');
                END
                RETURN 0;
            END alpha.)",
         "MODULE alpha;\nBEGIN\nCASE i OF\n1 : Out.String('One');\n| 2 : "
         "Out.String('Two');\nEND;\nRETURN 0\nEND alpha.",
         ""},

        {R"(MODULE alpha; (* CASE *)
            IMPORT Out;
            BEGIN
                FOR i := 1 TO 4 DO
                    CASE i OF
                        1 : Out.String('One');
                    |   2 : Out.String('Two');
                    |   3, 4, 5: Out.String('More');
                    END
                    Out.Ln;
                END
                RETURN 0;
            END alpha.)",
         "MODULE alpha;\nIMPORT Out;\nBEGIN\nFOR i := 1 TO 4 DO\nCASE i OF\n1 : "
         "Out.String('One');\n| 2 : Out.String('Two');\n| 3, 4, 5 : "
         "Out.String('More');\nEND;\nOut.Ln()\nEND;\nRETURN 0\nEND alpha.",
         ""},

        {R"(MODULE beta;
            IMPORT Out;
            VAR x :INTEGER;

            BEGIN
                CASE x OF
                    1 .. 2 : Out.String("A"); Out.Ln;
                |   5, 6 .. 8, 9 : Out.String("B,C"); Out.Ln;
                |   10 .. 11, 12 .. 15, 16 : Out.String("D-F"); Out.Ln;
                ELSE
                    Out.String('D-Z'); Out.Ln;
                END
                RETURN 0
            END beta.)",
         "MODULE beta;\nIMPORT Out;\nVAR\nx: INTEGER;\nBEGIN\nCASE x OF\n1..2 : "
         "Out.String(\"A\");\nOut.Ln();\n| 5, 6..8, 9 : Out.String(\"B,C\");\nOut.Ln();\n| "
         "10..11, 12..15, 16 : "
         "Out.String(\"D-F\");\nOut.Ln();\nELSE\nOut.String('D-Z');\nOut.Ln()\nEND;\nRETURN "
         "0\nEND beta.",
         ""},

        {R"(MODULE alpha; (* CASE *)
            IMPORT Out;
            VAR c : CHAR;

            BEGIN
                CASE c OF
                    'A' : Out.String("A"); Out.Ln;
                |   'B', 'C' : Out.String("B,C"); Out.Ln;
                |   'D' .. 'F' : Out.String("D-F"); Out.Ln;
                ELSE
                    Out.String('D-Z'); Out.Ln;
                END
                RETURN 0;
            END alpha.)",
         "MODULE alpha;\nIMPORT Out;\nVAR\nc: CHAR;\nBEGIN\nCASE c OF\n'A' : "
         "Out.String(\"A\");\nOut.Ln();\n| 'B', 'C' : Out.String(\"B,C\");\nOut.Ln();\n| 'D'..'F' "
         ": Out.String(\"D-F\");\nOut.Ln();\nELSE\nOut.String('D-Z');\nOut.Ln()\nEND;\nRETURN "
         "0\nEND alpha.",
         ""},

        {R"(MODULE alpha; (* CASE *)
            IMPORT Out;
            VAR c : CHAR;

            BEGIN
                CASE c OF
                    'A' :
                |   'B', 'C' : 
                |   'D' .. 'F' : 
                ELSE
                END
                RETURN 0;
            END alpha.)",
         "MODULE alpha;\nIMPORT Out;\nVAR\nc: CHAR;\nBEGIN\nCASE c OF\n'A' : | 'B', 'C' : | "
         "'D'..'F' : END;\nRETURN 0\nEND alpha.",
         ""},

        // Errors

        {R"(MODULE alpha;
            BEGIN
                CASE i
                |   1 : Out.String('One');
                |   2 : Out.String('Two');
                END
                RETURN 0;
            END alpha.)",
         "", "4,17: Unexpected token: | - expecting OF"},

        {R"(MODULE alpha;
            BEGIN
                CASE i OF
                |   1 : Out.String('One');
                   2 : Out.String('Two');
                END
                RETURN 0;
            END alpha.)",
         "", "5,20: Unexpected token: integer(2)"},

        {R"(MODULE alpha;
            BEGIN
                CASE i OF
                |   1 : Out.String('One');
                |   2 : Out.String('Two');
                
                RETURN 0;
            END alpha.)",
         "", "9,0: Unexpected token: EOF - expecting indent"},

        {R"(MODULE alpha; (* CASE *)
            IMPORT Out;
            BEGIN
                CASE i OF
                    1 : Out.String('One');
                |   2 : Out.String('Two');
                |   3, 4, : Out.String('More');
                END
                Out.Ln;
                RETURN 0;
            END alpha.)",
         "", "7,27: Unexpected token: :"},

        {R"(MODULE alpha; (* CASE *)
            IMPORT Out;
            BEGIN
                CASE i OF
                    1 : Out.String('One');
                |   2 : Out.String('Two');
                |   3 4 : Out.String('More');
                END
                Out.Ln;
                RETURN 0;
            END alpha.)",
         "", "7,23: Unexpected token: integer(4) - expecting :"},

        {R"(MODULE alpha; (* CASE *)
            IMPORT Out;
            BEGIN
                CASE i OF
                    1 .. : Out.String('One');
                END
                Out.Ln;
                RETURN 0;
            END alpha.)",
         "", "5,26: Unexpected token: :"},

        {R"(MODULE alpha; (* CASE *)
            IMPORT Out;
            BEGIN
                CASE i OF
                    .. 3 : Out.String('One');
                END
                Out.Ln;
                RETURN 0;
            END alpha.)",
         "", "5,22: Unexpected token: .."},

    };
    do_parse_tests(tests);
}
