//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "gtest/gtest.h"

#include "parse_test.hh"

TEST(Inspector, VarType) {
    std::vector<ParseTests> tests = {
        {"MODULE x; VAR z: INTEGER; BEGIN RETURN END x.",
         "MODULE x;\nVAR\nz: INTEGER;\nBEGIN\nRETURN \nEND x.", ""},

        // Errors
        {"MODULE x; VAR z: complex; BEGIN x := 10 END x.", "", "1,16: Unknown type: complex"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, UnknownExpr) {
    std::vector<ParseTests> tests = {
        {"MODULE y; VAR x : INTEGER; BEGIN RETURN x END y.",
         "MODULE y;\nVAR\nx: INTEGER;\nBEGIN\nRETURN x\nEND y.", ""},

        // Errors
        {"MODULE y; VAR x : INTEGER; BEGIN RETURN z END y.", "", "1,45: undefined identifier z"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, Return) {
    std::vector<ParseTests> tests = {
        {"MODULE x; VAR z: INTEGER; BEGIN z := 10; RETURN z END x.",
         "MODULE x;\nVAR\nz: INTEGER;\nBEGIN\nz := 10;\nRETURN z\nEND x.", ""},

        {"MODULE x; VAR z: INTEGER; PROCEDURE y; BEGIN z := 1 END y; "
         "BEGIN z := 10; END x.",
         "MODULE x;\nVAR\nz: INTEGER;\nPROCEDURE y;\nBEGIN\nz := 1\nEND "
         "y.\nBEGIN\nz := 10\nEND x.",
         ""},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, ReturnType) {
    std::vector<ParseTests> tests = {
        {"MODULE x; PROCEDURE f(): INTEGER; BEGIN RETURN 0 END f; BEGIN "
         "RETURN 333 END x.",
         "MODULE x;\nPROCEDURE f(): INTEGER;\nBEGIN\nRETURN 0\nEND "
         "f.\nBEGIN\nRETURN 333\nEND x.",
         ""},
        {"MODULE x; PROCEDURE f; BEGIN RETURN END f; BEGIN "
         "RETURN 333 END x.",
         "MODULE x;\nPROCEDURE f;\nBEGIN\nRETURN \nEND f.\nBEGIN\nRETURN "
         "333\nEND x.",
         ""},

        {R"(MODULE xxx;
            PROCEDURE f : BOOLEAN;
            BEGIN
            RETURN TRUE
            END f;
            BEGIN
            RETURN 3
            END xxx.)",
         "MODULE xxx;\nPROCEDURE f(): BOOLEAN;\nBEGIN\nRETURN TRUE\nEND "
         "f.\nBEGIN\nRETURN 3\nEND xxx.",
         ""},

        // Error
        {"MODULE x; PROCEDURE f(): complex; BEGIN RETURN 0 END f; BEGIN "
         "RETURN 333 END x.",
         "", "1,24: Unknown type: complex"},

        {"MODULE x; PROCEDURE f(): INTEGER; BEGIN RETURN END f; BEGIN "
         "RETURN 333 END x.",
         "", "1,46: RETURN does not match return type for function f"},

        {"MODULE x; PROCEDURE f; BEGIN RETURN 0 END f; BEGIN "
         "RETURN 333 END x.",
         "", "1,35: RETURN does not match return type for function f"},
        {R"(MODULE xxx;
            PROCEDURE f : BOOLEAN;
            BEGIN
            RETURN 123456
            END f;
            BEGIN
            RETURN 3
            END xxx.)",
         "", "4,18: RETURN does not match return type for function f"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, Call) {
    std::vector<ParseTests> tests = {
        {R"(MODULE y; 
        VAR x : INTEGER; 
        PROCEDURE f; 
        BEGIN 
            RETURN
        END f; 
        BEGIN
            f(); 
            RETURN x
        END y.)",
         "MODULE y;\nVAR\nx: INTEGER;\nPROCEDURE f;\nBEGIN\nRETURN \nEND "
         "f.\nBEGIN\nf();\nRETURN x\nEND y.",
         ""},

        {R"(MODULE y; 
            VAR x : INTEGER;
            PROCEDURE f(): INTEGER;
            BEGIN 
                RETURN 0
            END f; 
            BEGIN 
                f(); 
                RETURN f()
           END y.)",
         "MODULE y;\nVAR\nx: INTEGER;\nPROCEDURE f(): INTEGER;\nBEGIN\nRETURN "
         "0\nEND f.\nBEGIN\nf();\nRETURN f()\nEND y.",
         ""},

        {R"(MODULE xxx;
            PROCEDURE f(x : INTEGER) : INTEGER;
            BEGIN
            RETURN 0
            END f;
            BEGIN
                RETURN f(1)
            END xxx.)",
         "MODULE xxx;\nPROCEDURE f(x : INTEGER): INTEGER;\nBEGIN\nRETURN "
         "0\nEND f.\nBEGIN\nRETURN f(1)\nEND xxx.",
         ""},

        // Errors
        {"MODULE y; VAR x : INTEGER; PROCEDURE f; BEGIN RETURN END f; BEGIN "
         "x(); RETURN x END y.",
         "", "1,68: x is not a PROCEDURE"},
        {"MODULE y; VAR x : INTEGER; PROCEDURE f; BEGIN RETURN END f; BEGIN "
         "g(); RETURN x END y.",
         "", "1,68: undefined PROCEDURE g"},
        {"MODULE y; VAR x : INTEGER; "
         "PROCEDURE f():INTEGER; BEGIN RETURN 0 END f; "
         "BEGIN RETURN g() END y.",
         "", "1,87: undefined PROCEDURE g"},

        {R"(MODULE xxx;
            PROCEDURE f(x : INTEGER) : INTEGER;
            BEGIN
            RETURN 0
            END f;
            BEGIN
                RETURN f()
            END xxx.)",
         "",
         "7,25: calling PROCEDURE f, incorrect number of arguments: 0 instead "
         "of "
         "1"},
        {R"(MODULE xxx;
            PROCEDURE f() : INTEGER;
            BEGIN
            RETURN 0
            END f;
            BEGIN
                RETURN f(1,2,3,4)
            END xxx.)",
         "",
         "7,25: calling PROCEDURE f, incorrect number of arguments: 4 instead "
         "of "
         "0"},

    };
    do_inspect_tests(tests);
}

TEST(Inspector, CallType) {
    std::vector<ParseTests> tests = {
        {R"(MODULE y; 
        PROCEDURE f(x: INTEGER); 
        BEGIN 
            RETURN
        END f; 
        BEGIN
            f(1) 
        END y.)",
         "MODULE y;\nPROCEDURE f(x : INTEGER);\nBEGIN\nRETURN \nEND "
         "f.\nBEGIN\nf(1)\nEND y.",
         ""},

        {R"(MODULE y; 
        PROCEDURE f(x, y: INTEGER); 
        BEGIN 
            RETURN
        END f; 
        BEGIN
            f(1,2)
        END y.)",
         "MODULE y;\nPROCEDURE f(x : INTEGER; y : INTEGER);\nBEGIN\nRETURN "
         "\nEND f.\nBEGIN\nf(1, 2)\nEND y.",
         ""},

        // Errors
        {R"(MODULE y; 
        PROCEDURE f(x: INTEGER); 
        BEGIN 
            RETURN
        END f; 
        BEGIN
            f(TRUE)
        END y.)",
         "",
         "7,14: procedure call f has incorrect type BOOLEAN for parameter "
         "INTEGER"},

        {R"(MODULE y; 
        PROCEDURE f(x, y: INTEGER); 
        BEGIN 
            RETURN
        END f; 
        BEGIN
            f(TRUE,2)
        END y.)",
         "",
         "7,14: procedure call f has incorrect type BOOLEAN for parameter "
         "INTEGER"},

        {R"(MODULE y; 
        PROCEDURE f(x, y: INTEGER); 
        BEGIN 
            RETURN
        END f; 
        BEGIN
            f(1,TRUE)
        END y.)",
         "",
         "7,14: procedure call f has incorrect type BOOLEAN for parameter "
         "INTEGER"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, CallVar) {
    std::vector<ParseTests> tests = {
        {R"(MODULE y; 
        VAR y : INTEGER;
        PROCEDURE f(VAR x: INTEGER); 
        BEGIN
            x := 0;
            RETURN
        END f; 
        BEGIN
            f(y) 
        END y.)",
         "MODULE y;\nVAR\ny: INTEGER;\nPROCEDURE f(VAR x : "
         "INTEGER);\nBEGIN\nx "
         ":= 0;\nRETURN \nEND f.\nBEGIN\nf(y)\nEND y.",
         ""},

        {R"(MODULE y; 
        VAR y : INTEGER;
        PROCEDURE f(x : INTEGER; VAR y: INTEGER); 
        BEGIN
            x := 0;
            RETURN
        END f; 
        BEGIN
            f(1, y) 
        END y.)",
         "MODULE y;\nVAR\ny: INTEGER;\nPROCEDURE f(x : INTEGER; VAR y : "
         "INTEGER);\nBEGIN\nx := 0;\nRETURN \nEND f.\nBEGIN\nf(1, y)\nEND y.",
         ""},

        // Errors

        {R"(MODULE y; 
        VAR y : INTEGER;
        PROCEDURE f(VAR x: INTEGER); 
        BEGIN
            x := 0;
            RETURN
        END f; 
        BEGIN
            f(1) 
        END y.)",
         "",
         "9,14: procedure call f does not have a variable reference for VAR "
         "parameter INTEGER"},

        {R"(MODULE y; 
        VAR y : INTEGER;
        PROCEDURE f(VAR x: INTEGER); 
        BEGIN
            x := 0;
            RETURN
        END f; 
        BEGIN
            f(y + 1) 
        END y.)",
         "",
         "9,14: procedure call f does not have a variable reference for VAR "
         "parameter INTEGER"},

    };
    do_inspect_tests(tests);
}

TEST(Inspector, FunctionParams) {
    std::vector<ParseTests> tests = {

        {R"(MODULE xxx;
            VAR z : INTEGER;
            PROCEDURE f(x : INTEGER) : INTEGER;
            VAR zz : INTEGER;
            BEGIN
            RETURN zz
            END f;
            BEGIN
            RETURN 3
            END xxx.)",
         "MODULE xxx;\nVAR\nz: INTEGER;\nPROCEDURE f(x : INTEGER): "
         "INTEGER;\nVAR\nzz: INTEGER;\nBEGIN\nRETURN zz\nEND "
         "f.\nBEGIN\nRETURN 3\nEND xxx.",
         ""},

        {R"(MODULE xxx;
            VAR z : INTEGER;
            PROCEDURE f(x : INTEGER; y: INTEGER) : INTEGER;
            VAR zz : INTEGER;
            BEGIN
            RETURN zz
            END f;
            BEGIN
            RETURN 3
            END xxx.)",
         "MODULE xxx;\nVAR\nz: INTEGER;\nPROCEDURE f(x : INTEGER; y : "
         "INTEGER): INTEGER;\nVAR\nzz: INTEGER;\nBEGIN\nRETURN zz\nEND "
         "f.\nBEGIN\nRETURN 3\nEND xxx.",
         ""},

        // Errors
        {R"(MODULE xxx;
            VAR z : INTEGER;
            PROCEDURE f(x : UNDEF) : INTEGER;
            VAR zz : INTEGER;
            BEGIN
            RETURN zz
            END f;
            BEGIN
            RETURN 3
            END xxx.)",
         "", "3,27: Unknown type: UNDEF"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, Assignment) {
    std::vector<ParseTests> tests = {

        {R"(MODULE xxx;
            VAR z : INTEGER;
            BEGIN
            z := 33;
            RETURN z
            END xxx.)",
         "MODULE xxx;\nVAR\nz: INTEGER;\nBEGIN\nz := 33;\nRETURN z\nEND xxx.", ""},

        {R"(MODULE xxx;
            VAR z : BOOLEAN;
            BEGIN
            z := TRUE;
            RETURN z
            END xxx.)",
         "MODULE xxx;\nVAR\nz: BOOLEAN;\nBEGIN\nz := TRUE;\nRETURN z\nEND "
         "xxx.",
         ""},

        // Errors
        {R"(MODULE xxx;
            VAR z : BOOLEAN;
            BEGIN
            z := 4;
            RETURN z
            END xxx.)",
         "", "4,16: Can't assign expression of type INTEGER to z"},
        {R"(MODULE xxx;
            VAR z : INTEGER;
            BEGIN
            z := TRUE;
            RETURN z
            END xxx.)",
         "", "4,16: Can't assign expression of type BOOLEAN to z"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, ExprCompare) {
    std::vector<ParseTests> tests = {

        {R"(MODULE xxx;
            BEGIN
            RETURN TRUE = FALSE
            END xxx.)",
         "MODULE xxx;\nBEGIN\nRETURN TRUE = FALSE\nEND xxx.", ""},

        // Errors
        {R"(MODULE xxx;
            BEGIN
            RETURN TRUE = 1
            END xxx.)",
         "", "3,23: operator = doesn't takes types BOOLEAN and INTEGER"},
        {R"(MODULE xxx;
            BEGIN
            RETURN 0 # FALSE
            END xxx.)",
         "", "3,20: operator # doesn't takes types INTEGER and BOOLEAN"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, SimpleExpr) {
    std::vector<ParseTests> tests = {

        {R"(MODULE xxx;
            BEGIN
            RETURN TRUE OR FALSE
            END xxx.)",
         "MODULE xxx;\nBEGIN\nRETURN TRUE OR FALSE\nEND xxx.", ""},

        // Errors
        {R"(MODULE xxx;
            BEGIN
            RETURN TRUE + 1
            END xxx.)",
         "", "3,23: operator + doesn't takes types BOOLEAN and INTEGER"},
        {R"(MODULE xxx;
            BEGIN
            RETURN 0 OR FALSE
            END xxx.)",
         "", "3,20: operator OR doesn't takes types INTEGER and BOOLEAN"},
        {R"(MODULE xxx;
            BEGIN
            RETURN 0 OR 0
            END xxx.)",
         "", "3,20: operator OR doesn't takes types INTEGER and INTEGER"},
        {R"(MODULE xxx;
            BEGIN
            RETURN FALSE + FALSE
            END xxx.)",
         "", "3,24: operator + doesn't takes types BOOLEAN and BOOLEAN"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, Term) {
    std::vector<ParseTests> tests = {

        {R"(MODULE xxx;
            BEGIN
            RETURN TRUE & FALSE
            END xxx.)",
         "MODULE xxx;\nBEGIN\nRETURN TRUE & FALSE\nEND xxx.", ""},

        // Errors
        {R"(MODULE xxx;
            BEGIN
            RETURN TRUE * 1
            END xxx.)",
         "", "3,23: operator * doesn't takes types BOOLEAN and INTEGER"},
        {R"(MODULE xxx;
            BEGIN
            RETURN 0 & FALSE
            END xxx.)",
         "", "3,20: operator & doesn't takes types INTEGER and BOOLEAN"},
        {R"(MODULE xxx;
            BEGIN
            RETURN TRUE DIV FALSE
            END xxx.)",
         "", "3,23: operator DIV doesn't takes types BOOLEAN and BOOLEAN"},
        {R"(MODULE xxx;
            BEGIN
            RETURN 0 & 23
            END xxx.)",
         "", "3,20: operator & doesn't takes types INTEGER and INTEGER"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, Factor) {
    std::vector<ParseTests> tests = {

        {R"(MODULE xxx;
            BEGIN
            RETURN ~ TRUE
            END xxx.)",
         "MODULE xxx;\nBEGIN\nRETURN ~ TRUE\nEND xxx.", ""},

        // Errors
        {R"(MODULE xxx;
            BEGIN
            RETURN ~ 1
            END xxx.)",
         "", "3,20: type in ~ expression must be BOOLEAN"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, TypeDef) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
                TYPE time = INTEGER;
                     spin = BOOLEAN;
                VAR seconds : time;
                    orientation : spin;
                BEGIN
                    RETURN seconds
                END alpha.)",
         "MODULE alpha;\nTYPE\ntime = INTEGER;\nspin = BOOLEAN;\nVAR\nseconds: "
         "time;\norientation: spin;\nBEGIN\nRETURN seconds\nEND alpha.",
         ""},

        // Errors

        {R"(MODULE alpha;
                TYPE time = INTEGER;
                     spin = BOOLEAN;
                VAR seconds : complex;
                    orientation : spin;
                BEGIN
                    RETURN seconds
                END alpha.)",
         "", "4,29: Unknown type: complex"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, TypeAssign) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
                TYPE time = INTEGER;
                     spin = BOOLEAN;
                VAR seconds : time;
                    orientation : spin;
                BEGIN
                    seconds := 0;
                    orientation := FALSE;
                    RETURN seconds
                END alpha.)",
         "MODULE alpha;\nTYPE\ntime = INTEGER;\nspin = BOOLEAN;\nVAR\nseconds: "
         "time;\norientation: spin;\nBEGIN\nseconds := 0;\norientation := "
         "FALSE;\nRETURN seconds\nEND alpha.",
         ""},

        // Errors

        {R"(MODULE alpha;
                TYPE time = INTEGER;
                VAR seconds : time;
                BEGIN
                    seconds := TRUE;
                    RETURN seconds
                END alpha.)",
         "", "5,30: Can't assign expression of type BOOLEAN to seconds"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, TypeExpr) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
                TYPE time = INTEGER;
                     spin = BOOLEAN;
                VAR seconds : time;
                    orientation : spin;
                BEGIN
                    seconds := 60;
                    seconds := seconds + 1;
                    orientation := FALSE;
                    RETURN seconds
                END alpha.)",
         "MODULE alpha;\nTYPE\ntime = INTEGER;\nspin = BOOLEAN;\nVAR\nseconds: "
         "time;\norientation: spin;\nBEGIN\nseconds := 60;\nseconds := "
         "seconds+1;\norientation := FALSE;\nRETURN seconds\nEND alpha.",
         ""},

        // Errors
    };
    do_inspect_tests(tests);
}

TEST(Inspector, Const) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
                CONST time = 60 * 60;
                BEGIN
                    RETURN time
                END alpha.)",
         "MODULE alpha;\nCONST\ntime = 60*60;\nBEGIN\nRETURN time\nEND alpha.", ""},

        {R"(MODULE alpha;
                CONST seconds = 60;
                      minutes = 60 * seconds;
                BEGIN
                    RETURN minutes
                END alpha.)",
         "MODULE alpha;\nCONST\nseconds = 60;\nminutes = "
         "60*seconds;\nBEGIN\nRETURN minutes\nEND alpha.",
         ""},

        {R"(MODULE alpha; (* REAL *)
                CONST pi = 3.1415927;
                BEGIN
                    RETURN 0
                END alpha.)",
         "MODULE alpha;\nCONST\npi = 3.1415927;\nBEGIN\nRETURN 0\nEND alpha.", ""},

        // Errors

        {R"(MODULE alpha;
                CONST time = 60 * 60;
                
                PROCEDURE f (x : INTEGER): INTEGER; 
                CONST y = x;
                BEGIN 
                    RETURN y
                END f;

                BEGIN
                    RETURN time
                END alpha.)",
         "", "5,21: CONST y is not a constant expression"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, ConstAssign) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
                CONST time = 60 * 60;
                BEGIN
                    RETURN time
                END alpha.)",
         "MODULE alpha;\nCONST\ntime = 60*60;\nBEGIN\nRETURN time\nEND alpha.", ""},

        // Errors
        {R"(MODULE alpha;
                CONST time = 60 * 60;
                BEGIN
                    time := 40;
                    RETURN time
                END alpha.)",
         "", "4,27: Can't assign to CONST variable time"},
    };
    do_inspect_tests(tests);
}