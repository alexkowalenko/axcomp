//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "gtest/gtest.h"

#include "parse_test.hh"

TEST(Parser, ARRAY) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
                VAR x : ARRAY [5] OF INTEGER;
                BEGIN
                    RETURN 0 
                END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY [5] OF INTEGER;\nBEGIN\nRETURN 0\nEND "
         "alpha.",
         ""},

        {R"(MODULE alpha;
                VAR x : ARRAY [5] OF INTEGER;
                VAR y : ARRAY [5] OF ARRAY [5] OF BOOLEAN;
                BEGIN
                    RETURN 0
                END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY [5] OF INTEGER;\ny: ARRAY [5] OF ARRAY "
         "[5] OF BOOLEAN;\nBEGIN\nRETURN 0\nEND alpha.",
         ""},

        {R"(MODULE alpha;
                VAR x : ARRAY [5] OF INTEGER;
                VAR y : ARRAY [5] OF ARRAY [5] OF INTEGER;

                PROCEDURE sum(a : ARRAY [5] OF BOOLEAN) : INTEGER;
                BEGIN
                    RETURN 0
                END sum;

                PROCEDURE add(a : ARRAY [5] OF INTEGER; a : ARRAY [5] OF INTEGER) : ARRAY [5] OF INTEGER;
                BEGIN
                    RETURN 0
                END add;

                BEGIN
                    RETURN 0
                END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY [5] OF INTEGER;\ny: ARRAY [5] OF ARRAY "
         "[5] OF INTEGER;\nPROCEDURE sum(a : ARRAY [5] OF BOOLEAN): "
         "INTEGER;\nBEGIN\nRETURN 0\nEND sum.\nPROCEDURE add(a : ARRAY [5] OF "
         "INTEGER; a : ARRAY [5] OF INTEGER): ARRAY [5] OF "
         "INTEGER;\nBEGIN\nRETURN 0\nEND add.\nBEGIN\nRETURN 0\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
                VAR x : ARRAY 5] OF INTEGER;
                BEGIN
                    RETURN 0
                END alpha.)",
         "", "2,31: Unexpected token: integer(5) - expecting ["},

        {R"(MODULE alpha;
                VAR x : ARRAY [] OF INTEGER;
                BEGIN
                    RETURN 0
                END alpha.)",
         "", "2,32: Unexpected token: ] - expecting integer"},

        {R"(MODULE alpha;
                VAR x : ARRAY [5 OF INTEGER;
                BEGIN
                    RETURN 0
                END alpha.)",
         "", "2,35: Unexpected token: OF - expecting ]"},

        {R"(MODULE alpha;
                VAR x : ARRAY [5]  INTEGER;
                BEGIN
                    RETURN 0
                END alpha.)",
         "", "2,42: Unexpected token: INTEGER - expecting OF"},

        {R"(MODULE alpha;
                VAR x : ARRAY [5] OF ;
                BEGIN
                    RETURN 0
                END alpha.)",
         "", "2,38: Unexpected token: semicolon - expecting indent"},

    };
    do_parse_tests(tests);
}

TEST(Parser, ArrayIndex) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
            VAR x : ARRAY [5] OF INTEGER;
            BEGIN
                RETURN x[2]
            END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY [5] OF INTEGER;\nBEGIN\nRETURN "
         "x[2]\nEND alpha.",
         ""},

        {R"(MODULE alpha;
            VAR x : ARRAY [5] OF INTEGER;
            VAR y : ARRAY [5] OF ARRAY [5] OF BOOLEAN;
            VAR z : INTEGER;
            BEGIN
                RETURN x[2] + y[z+1][0]
            END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY [5] OF INTEGER;\ny: ARRAY [5] OF ARRAY "
         "[5] OF BOOLEAN;\nz: INTEGER;\nBEGIN\nRETURN x[2]+y[z+1][0]\nEND "
         "alpha.",
         ""},

        // Errors

        {R"(MODULE alpha;
            VAR x : ARRAY [5] OF INTEGER;
            BEGIN
                RETURN x 2]
            END alpha.)",
         "", "4,26: Unexpected token: integer(2) - expecting END"},
        {R"(MODULE alpha;
            VAR x : ARRAY [5] OF INTEGER;
            BEGIN
                RETURN x[]
            END alpha.)",
         "", "4,26: Unexpected token: ]"},
        {R"(MODULE alpha;
            VAR x : ARRAY [5] OF INTEGER;
            BEGIN
                RETURN x[2
            END alpha.)",
         "", "5,15: Unexpected token: END - expecting ]"},
    };

    do_parse_tests(tests);
}

TEST(Parser, ArrayIndexAssign) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
            VAR x : ARRAY [5] OF INTEGER;
            BEGIN
                x[2] := 2;
                RETURN x[2]
            END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY [5] OF INTEGER;\nBEGIN\nx[2] := "
         "2;\nRETURN x[2]\nEND alpha.",
         ""},

        {R"(MODULE alpha;
            VAR x : ARRAY [5] OF INTEGER;
            VAR y : ARRAY [5] OF ARRAY [5] OF BOOLEAN;
            BEGIN
                 x[1 + 1] := 2;
                 y[1][2] := TRUE;
                RETURN x[2] + y[z+1][0]
            END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY [5] OF INTEGER;\ny: ARRAY [5] OF ARRAY "
         "[5] OF BOOLEAN;\nBEGIN\nx[1+1] := 2;\ny[1][2] := TRUE;\nRETURN "
         "x[2]+y[z+1][0]\nEND alpha.",
         ""},

        // Errors

        {R"(MODULE alpha;
            VAR x : ARRAY [5] OF INTEGER;
            BEGIN
                 x 2] := 2;
                RETURN 0
            END alpha.)",
         "", "4,20: Unexpected token: integer(2) - expecting :="},
        {R"(MODULE alpha;
            VAR x : ARRAY [5] OF INTEGER;
            BEGIN
                x[] := 2;
                RETURN x[2]
            END alpha.)",
         "", "4,19: Unexpected token: ]"},
        {R"(MODULE alpha;
            VAR x : ARRAY [5] OF INTEGER;
            BEGIN
                x[2 := 2;
                RETURN x[2]
            END alpha.)",
         "", "4,22: Unexpected token: := - expecting ]"},
    };

    do_parse_tests(tests);
}

TEST(Parser, RECORD) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
                VAR pt : RECORD
                        x : INTEGER;
                        y : INTEGER
                    END;
                BEGIN
                    RETURN 0 
                END alpha.)",
         "MODULE alpha;\nVAR\npt: RECORD\n  x: INTEGER;\n  y: "
         "INTEGER\n;\nBEGIN\nRETURN 0\nEND alpha.",
         ""},

        {R"(MODULE alpha;
                VAR pt : RECORD
                        x, y, z : INTEGER;
                    END;
                BEGIN
                    RETURN 0 
                END alpha.)",
         "MODULE alpha;\nVAR\npt: RECORD\n  x: INTEGER;\n  y: INTEGER;\n  z: "
         "INTEGER\n;\nBEGIN\nRETURN 0\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
                VAR pt : RECORD
                        x : INTEGER;
                        y : INTEGER
                    ;
                BEGIN
                    RETURN 0 
                END alpha.)",
         "", "6,21: Unexpected token: BEGIN - expecting END"},

    };
    do_parse_tests(tests);
}

TEST(Parser, RecordFields) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
                VAR pt : RECORD
                        x : INTEGER;
                        y : INTEGER
                    END;
                BEGIN
                    RETURN pt.x + pt.y
                END alpha.)",
         "MODULE alpha;\nVAR\npt: RECORD\n  x: INTEGER;\n  y: "
         "INTEGER\n;\nBEGIN\nRETURN pt.x+pt.y\nEND alpha.",
         ""},

        {R"(MODULE alpha;
                VAR pt : RECORD
                        x, y, z : INTEGER;
                    END;
                BEGIN
                    pt.x := 1;
                    pt.y := 2;
                    pt.z := 3;
                    RETURN pt.x * pt.y * pt.z
                END alpha.)",
         "MODULE alpha;\nVAR\npt: RECORD\n  x: INTEGER;\n  y: INTEGER;\n  z: "
         "INTEGER\n;\nBEGIN\npt.x := 1;\npt.y := 2;\npt.z := 3;\nRETURN "
         "pt.x*pt.y*pt.z\nEND alpha.",
         ""},

        {R"(MODULE alpha;
                VAR a : RECORD
                        x : INTEGER;
                        y : ARRAY [3] OF INTEGER;
                    END;
                BEGIN
                    a.x := 1;
                    a.y[1] := 2;
                    RETURN a.y[2]
                END alpha.)",
         "MODULE alpha;\nVAR\na: RECORD\n  x: INTEGER;\n  y: ARRAY [3] OF "
         "INTEGER\n;\nBEGIN\na.x := 1;\na.y[1] := 2;\nRETURN a.y[2]\nEND "
         "alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
                VAR pt : RECORD
                        x : INTEGER;
                        y : INTEGER
                    END;
                BEGIN
                    RETURN pt. + pt.y
                END alpha.)",
         "", "7,32: Unexpected token: + - expecting indent"},

    };
    do_parse_tests(tests);
}
