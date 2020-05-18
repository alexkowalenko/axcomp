//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "gtest/gtest.h"

#include "parse_test.hh"

TEST(Inspector, ReadOnly) {
    std::vector<ParseTests> tests = {
        {R"(MODULE alpha;
                CONST a* = 10;
                TYPE second* = INTEGER;
                VAR x* : ARRAY 5 OF INTEGER;
                VAR pt : RECORD x*, y* : INTEGER END;

                PROCEDURE f* : INTEGER;
                BEGIN
                    RETURN 0
                END f;
                BEGIN
                    RETURN 0 
                END alpha.)",
         "MODULE alpha;\nCONST\na* = 10;\nTYPE\nsecond* = INTEGER;\nVAR\nx*: ARRAY 5 OF "
         "INTEGER;\npt: RECORD\nx*: INTEGER;\ny*: INTEGER\nEND;\nPROCEDURE f*(): "
         "INTEGER;\nBEGIN\nRETURN 0\nEND f.\nBEGIN\nRETURN 0\nEND alpha.",
         ""},

        {R"(MODULE alpha;
                VAR x- : ARRAY 5 OF INTEGER;
                VAR pt : RECORD x-, y- : INTEGER END;

                BEGIN
                    RETURN 0 
                END alpha.)",
         "MODULE alpha;\nVAR\nx-: ARRAY 5 OF INTEGER;\npt: RECORD\nx-: INTEGER;\ny-: "
         "INTEGER\nEND;\nBEGIN\nRETURN 0\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
                CONST a- = 10;
                TYPE second* = INTEGER;
                VAR x* : ARRAY 5 OF INTEGER;
                VAR pt : RECORD x*, y* : INTEGER END;

                PROCEDURE f* : INTEGER;
                BEGIN
                    RETURN 0
                END f;
                BEGIN
                    RETURN 0 
                END alpha.)",
         "", "2,21: CONST a is always read only"},

        {R"(MODULE alpha;
                CONST a* = 10;
                TYPE second- = INTEGER;
                VAR x* : ARRAY 5 OF INTEGER;
                VAR pt : RECORD x*, y* : INTEGER END;

                PROCEDURE f* : INTEGER;
                BEGIN
                    RETURN 0
                END f;
                BEGIN
                    RETURN 0 
                END alpha.)",
         "", "3,20: TYPE second is always read only"},

        {R"(MODULE alpha;
                CONST a = 10;
                TYPE second = INTEGER;
                VAR x : ARRAY 5 OF INTEGER;
                VAR pt : RECORD x, y : INTEGER END;

                PROCEDURE f- : INTEGER;
                BEGIN
                    RETURN 0
                END f;
                BEGIN
                    RETURN 0 
                END alpha.)",
         "", "7,25: PROCEDURE f is always read only"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, IMPORT) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
             IMPORT beta;
             BEGIN
                 RETURN 0;
             END alpha.)",
         "MODULE alpha;\nIMPORT beta;\nBEGIN\nRETURN 0\nEND alpha.", ""},

        {R"(MODULE alpha;
             IMPORT B := beta;
             BEGIN
                 RETURN 0;
             END alpha.)",
         "MODULE alpha;\nIMPORT B := beta;\nBEGIN\nRETURN 0\nEND alpha.", ""},

        // Errors

        {R"(MODULE alpha;
             IMPORT System;
             BEGIN
                 RETURN System.x;
             END alpha.)",
         "", "2,19: MODULE System not found"},

        {R"(MODULE alpha;
             IMPORT S := System;
             BEGIN
                 RETURN System.x;
             END alpha.)",
         "", "2,24: MODULE System not found"},

    };
    do_inspect_fimport_tests(tests);
}

TEST(Inspector, ImportAccess) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
             IMPORT beta;
             BEGIN
                 RETURN beta.a;
             END alpha.)",
         "MODULE alpha;\nIMPORT beta;\nBEGIN\nRETURN beta.a\nEND alpha.", ""},

        {R"(MODULE alpha;
             IMPORT B := beta;
             BEGIN
                 RETURN B.a;
             END alpha.)",
         "MODULE alpha;\nIMPORT B := beta;\nBEGIN\nRETURN beta.a\nEND alpha.", ""},

        {R"(MODULE alpha;
             IMPORT beta;
             BEGIN
                beta.c := 30;
                RETURN beta.c;
             END alpha.)",
         "MODULE alpha;\nIMPORT beta;\nBEGIN\nbeta.c := 30;\nRETURN beta.c\nEND alpha.", ""},

        {R"(MODULE alpha;
             IMPORT beta;
             BEGIN
                beta.d := TRUE;
                RETURN beta.c;
             END alpha.)",
         "MODULE alpha;\nIMPORT beta;\nBEGIN\nbeta.d := TRUE;\nRETURN beta.c\nEND alpha.", ""},

        {R"(MODULE alpha;
             IMPORT beta;
             BEGIN
                beta.f(1);
                RETURN beta.f(100);
             END alpha.)",
         "MODULE alpha;\nIMPORT beta;\nBEGIN\nbeta.f(1);\nRETURN beta.f(100)\nEND alpha.", ""},

        // Errors

        // No object aa in beta
        {R"(MODULE alpha;
             IMPORT beta;
             BEGIN
                 RETURN beta.aa;
             END alpha.)",
         "", "4,28: undefined identifier aa in MODULE beta"},

        // Try to access module without alias
        {R"(MODULE alpha;
             IMPORT B := beta;
             BEGIN
                 RETURN beta.a;
             END alpha.)",
         "", "4,30: undefined identifier beta"},

        // CONST
        {R"(MODULE alpha;
             IMPORT beta;
             BEGIN
                beta.a := 10;
                RETURN beta.a;
             END alpha.)",
         "", "4,25: Can't assign to CONST variable beta.a"},

        // Read only
        {R"(MODULE alpha;
             IMPORT beta;
             BEGIN
                beta.b := 0;
                 RETURN beta.a;
             END alpha.)",
         "", "4,25: Can't assign to read only (-) variable beta.b"},

        // Wrong type
        {R"(MODULE alpha;
             IMPORT beta;
             BEGIN
                beta.d := 0;
                RETURN beta.c;
             END alpha.)",
         "", "4,25: Can't assign expression of type INTEGER to beta.d"},

        // function does not exist
        {R"(MODULE alpha;
             IMPORT beta;
             BEGIN
                beta.g(1, 2);
                RETURN beta.g(100, 200);
             END alpha.)",
         "", "4,23: undefined PROCEDURE beta.g"},

        // Wrong parameters for function
        {R"(MODULE alpha;
             IMPORT beta;
             BEGIN
                beta.f(1, 1);
                RETURN beta.f(100);
             END alpha.)",
         "", "4,23: calling PROCEDURE beta.f, incorrect number of arguments: 2 instead of 1"},

    };
    do_inspect_fimport_tests(tests);
}
