//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "gtest/gtest.h"

#include "parse_test.hh"

TEST(Parser, Globals) {
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
                CONST a** = 10;
                BEGIN
                    RETURN 0 
                END alpha.)",
         "", "2,25: Unexpected token: * - expecting ="},

        {R"(MODULE alpha;
                CONST a+ = 10;
                PROCEDURE f* : INTEGER;
                BEGIN
                    RETURN 0
                END f;
                BEGIN
                    RETURN 0 
                END alpha.)",
         "", "2,24: Unexpected token: + - expecting ="},

    };
    do_parse_tests(tests);
}

TEST(Parser, IMPORT) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
            IMPORT System;
            BEGIN
                RETURN 0
            END alpha.)",
         "MODULE alpha;\nIMPORT System;\nBEGIN\nRETURN 0\nEND alpha.", ""},

        {R"(MODULE alpha;
            IMPORT B := A;
            BEGIN
                RETURN 0
            END alpha.)",
         "MODULE alpha;\nIMPORT B := A;\nBEGIN\nRETURN 0\nEND alpha.", ""},

        {R"(MODULE alpha;
            IMPORT System, B := A;
            BEGIN
                RETURN 0
            END alpha.)",
         "MODULE alpha;\nIMPORT System,\nB := A;\nBEGIN\nRETURN 0\nEND alpha.",
         ""},

        {R"(MODULE alpha;
            IMPORT System, B := A, C := D; 
            BEGIN
                RETURN 0
            END alpha.)",
         "MODULE alpha;\nIMPORT System,\nB := A,\nC := D;\nBEGIN\nRETURN "
         "0\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
            IMPORT ;
            BEGIN
                RETURN 0
            END alpha.)",
         "", "2,20: Unexpected token: semicolon - expecting indent"},

        {R"(MODULE alpha;
            IMPORT B := ;
            BEGIN
                RETURN 0
            END alpha.)",
         "", "2,25: Unexpected token: semicolon - expecting indent"},

    };
    do_parse_tests(tests);
}

TEST(Parser, Qualident) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
            IMPORT System;
            BEGIN
                System.x := System.y + 1;
                RETURN System.error;
            END alpha.)",
         "MODULE alpha;\nIMPORT System;\nBEGIN\nSystem.x := "
         "System.y+1;\nRETURN System.error\nEND alpha.",
         ""},

        {R"(MODULE alpha;
            IMPORT System;
            TYPE sys = System.Type;
            VAR x : System.error;
            PROCEDURE f* (y : System.Jones): System.Jones;
                BEGIN
                    RETURN y
                END f;
            BEGIN
                System.x := System.y + 1;
                RETURN System.error;
            END alpha.)",
         "MODULE alpha;\nIMPORT System;\nTYPE\nsys = System.Type;\nVAR\nx: "
         "System.error;\nPROCEDURE f*(y : System.Jones): "
         "System.Jones;\nBEGIN\nRETURN y\nEND f.\nBEGIN\nSystem.x := "
         "System.y+1;\nRETURN System.error\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
            IMPORT System;
            BEGIN
                RETURN System.
            END alpha.)",
         "", "5,15: Unexpected token: END - expecting indent"},

        {R"(MODULE alpha;
            IMPORT System;
            TYPE sys = System.;
            BEGIN
                RETURN 0;
            END alpha.)",
         "", "3,31: Unexpected token: semicolon - expecting indent"},
    };
    do_parse_tests(tests);
}

TEST(Parser, QualidentFunctionCall) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
            IMPORT Out;
            BEGIN
                Out.WriteInt(1);
                RETURN 0;
            END alpha.)",
         "MODULE alpha;\nIMPORT Out;\nBEGIN\nOut.WriteInt(1);\nRETURN 0\nEND "
         "alpha.",
         ""},

        {R"(MODULE alpha;
             IMPORT Math;
             BEGIN
                 RETURN Math.Abs(1) + Math.Abs(2);
             END alpha.)",
         "MODULE alpha;\nIMPORT Math;\nBEGIN\nRETURN "
         "Math.Abs(1)+Math.Abs(2)\nEND alpha.",
         ""},

        // Errors
    };
    do_parse_tests(tests);
}