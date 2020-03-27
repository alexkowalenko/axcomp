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
