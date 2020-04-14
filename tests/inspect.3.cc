//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "gtest/gtest.h"

#include "parse_test.hh"

void do_inspect_tests(std::vector<ParseTests> &tests);

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
         "MODULE alpha;\nCONST\na* = 10;\nTYPE\nsecond* = INTEGER;\nVAR\nx*: "
         "ARRAY 5 OF INTEGER;\npt: RECORD\n  x*: INTEGER;\n  y*: "
         "INTEGER\n;\nPROCEDURE f*(): INTEGER;\nBEGIN\nRETURN 0\nEND "
         "f.\nBEGIN\nRETURN 0\nEND alpha.",
         ""},

        {R"(MODULE alpha;
                VAR x- : ARRAY 5 OF INTEGER;
                VAR pt : RECORD x-, y- : INTEGER END;

                BEGIN
                    RETURN 0 
                END alpha.)",
         "MODULE alpha;\nVAR\nx-: ARRAY 5 OF INTEGER;\npt: RECORD\n  x-: "
         "INTEGER;\n  y-: INTEGER\n;\nBEGIN\nRETURN 0\nEND alpha.",
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

TEST(Inspector, Import) {
    std::vector<ParseTests> tests = {

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
    do_inspect_tests(tests);
}

TEST(Inspector, Import2) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
             IMPORT beta;
             BEGIN
                 RETURN 0;
             END alpha.)",
         "MODULE alpha;\nIMPORT beta;\nBEGIN\nRETURN 0\nEND alpha.", ""},

        {R"(MODULE alpha;
             IMPORT beta;
             BEGIN
                 RETURN beta.a;
             END alpha.)",
         "MODULE alpha;\nIMPORT beta;\nBEGIN\nRETURN beta.a\nEND alpha.", ""},

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

        // Errors
        {R"(MODULE alpha;
             IMPORT gamma;
             BEGIN
                 RETURN System.x;
             END alpha.)",
         "", "2,19: MODULE gamma not found"},

        // No object aa in beta
        {R"(MODULE alpha;
             IMPORT beta;
             BEGIN
                 RETURN beta.aa;
             END alpha.)",
         "", "4,28: undefined identifier aa in MODULE beta"},

        // CONST
        // {R"(MODULE alpha;
        //      IMPORT beta;
        //      BEGIN
        //         beta.a := 10;
        //         RETURN beta.a;
        //      END alpha.)",
        //  "", "error"},

        // // Read only
        // {R"(MODULE alpha;
        //      IMPORT beta;
        //      BEGIN
        //         beta.b := 0;
        //          RETURN beta.a;
        //      END alpha.)",
        //  "", ""},

        // Wrong type
        {R"(MODULE alpha;
             IMPORT beta;
             BEGIN
                beta.d := 0;
                RETURN beta.c;
             END alpha.)",
         "", "4,25: Can't assign expression of type INTEGER to beta.d"},

    };
    do_inspect_fimport_tests(tests);
}
