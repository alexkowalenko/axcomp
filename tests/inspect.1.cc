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
         "y;\nBEGIN\nz := 10\nEND x.",
         ""},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, ProcedureDefined) {
    std::vector<ParseTests> tests = {

        // Error
        {R"(MODULE alpha;
            PROCEDURE f;
            END f;

            PROCEDURE f;
            END f;

            BEGIN
                f;
            END alpha.)",
         "", "5,23: PROCEDURE f, identifier is already defined"},

        {R"(MODULE alpha; (* pointers *)
            VAR f: INTEGER;

            PROCEDURE f;
            END f;

            BEGIN
                f;
            END alpha.)",
         "", "4,23: PROCEDURE f, identifier is already defined"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, ReturnType) {
    std::vector<ParseTests> tests = {
        {"MODULE x; PROCEDURE f(): INTEGER; BEGIN RETURN 0 END f; BEGIN "
         "RETURN 333 END x.",
         "MODULE x;\nPROCEDURE f(): INTEGER;\nBEGIN\nRETURN 0\nEND "
         "f;\nBEGIN\nRETURN 333\nEND x.",
         ""},
        {"MODULE x; PROCEDURE f; BEGIN RETURN END f; BEGIN "
         "RETURN 333 END x.",
         "MODULE x;\nPROCEDURE f;\nBEGIN\nRETURN \nEND f;\nBEGIN\nRETURN "
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
         "f;\nBEGIN\nRETURN 3\nEND xxx.",
         ""},

        // Error
        {"MODULE x; PROCEDURE f(): complex; BEGIN RETURN 0 END f; BEGIN "
         "RETURN 333 END x.",
         "", "1,24: Unknown type: complex"},

        {"MODULE x; PROCEDURE f(): INTEGER; BEGIN RETURN END f; BEGIN "
         "RETURN 333 END x.",
         "", "1,46: RETURN type (INTEGER) does not match return type for function f: void"},

        {"MODULE x; PROCEDURE f; BEGIN RETURN 0 END f; BEGIN "
         "RETURN 333 END x.",
         "", "1,35: RETURN type (void) does not match return type for function f: INTEGER"},
        {R"(MODULE xxx;
            PROCEDURE f : BOOLEAN;
            BEGIN
            RETURN 123456
            END f;
            BEGIN
            RETURN 3
            END xxx.)",
         "", "4,18: RETURN type (BOOLEAN) does not match return type for function f: INTEGER"},
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
         "f;\nBEGIN\nf();\nRETURN x\nEND y.",
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
         "0\nEND f;\nBEGIN\nf();\nRETURN f()\nEND y.",
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
         "0\nEND f;\nBEGIN\nRETURN f(1)\nEND xxx.",
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
         "f;\nBEGIN\nf(1)\nEND y.",
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
         "\nEND f;\nBEGIN\nf(1, 2)\nEND y.",
         ""},

        {R"(MODULE alpha;
            PROCEDURE f(x : INTEGER);
            END f;
            PROCEDURE g(x: CHAR);
            END g;
            PROCEDURE h(x: INTEGER; y:CHAR);
            BEGIN
                f(x);
                g(y);
            END h;

            BEGIN
                h(1, 'x');
            END alpha.)",
         "MODULE alpha;\nPROCEDURE f(x : INTEGER);\nEND f;\nPROCEDURE g(x : CHAR);\nEND "
         "g;\nPROCEDURE h(x : INTEGER; y : CHAR);\nBEGIN\nf(x);\ng(y)\nEND h;\nBEGIN\nh(1, "
         "'x')\nEND alpha.",
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

        {R"(MODULE alpha;
            PROCEDURE g(x: CHAR);
            END g;
            PROCEDURE h(x: INTEGER; y:CHAR);
            BEGIN
                g(x);
            END h;

            BEGIN
                h(1, 'x');
            END alpha.)",
         "", "6,18: procedure call g has incorrect type INTEGER for parameter CHAR"},

        {R"(MODULE alpha;
            PROCEDURE f(x : INTEGER);
            END f;
            PROCEDURE h(x: INTEGER; y:CHAR);
            BEGIN
                f(y);
            END h;

            BEGIN
                h(1, 'x');
            END alpha.)",
         "", "6,18: procedure call f has incorrect type CHAR for parameter INTEGER"},

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
         ":= 0;\nRETURN \nEND f;\nBEGIN\nf(y)\nEND y.",
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
         "INTEGER);\nBEGIN\nx := 0;\nRETURN \nEND f;\nBEGIN\nf(1, y)\nEND y.",
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

TEST(Inspector, CallVarConst) {
    std::vector<ParseTests> tests = {
        {R"(MODULE alpha; (* SET *)
            VAR y: INTEGER;
            BEGIN
            INC(y);
            END alpha.)",
         "MODULE alpha;\nVAR\ny: INTEGER;\nBEGIN\nINC(y)\nEND alpha.", ""},

        // Errors
        {R"(MODULE alpha; (* SET *)
            CONST x = 1;
            BEGIN
            INC(x);
            END alpha.)",
         "", "4,16: procedure call INC does not have a variable reference for VAR parameter any"},

        {R"(MODULE alpha; (* SET *)
            BEGIN
            INC(1);
            END alpha.)",
         "", "3,16: procedure call INC does not have a variable reference for VAR parameter any"},

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
         "f;\nBEGIN\nRETURN 3\nEND xxx.",
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
         "f;\nBEGIN\nRETURN 3\nEND xxx.",
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

TEST(Inspector, NestedProcs) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha; (* Nested Procedures *)
            VAR x: INTEGER;

            PROCEDURE f(): INTEGER;                
                PROCEDURE g(): INTEGER;
                BEGIN 
                    RETURN 1;
                END g;
            BEGIN
                RETURN g();
            END f;

            BEGIN
                f();
            END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nPROCEDURE f(): INTEGER;\nPROCEDURE g(): "
         "INTEGER;\nBEGIN\nRETURN 1\nEND g;\nBEGIN\nRETURN g()\nEND f;\nBEGIN\nf()\nEND alpha.",
         ""},

        {R"(MODULE alpha; (* Nested Procedures *)
            VAR x: INTEGER;
            PROCEDURE f(): INTEGER;
                CONST x = 2;
                
                PROCEDURE g(): INTEGER;
                CONST x = 1;
                BEGIN 
                    RETURN x;
                END g;
            BEGIN
                g();
                RETURN x;
            END f;

            PROCEDURE g(x: INTEGER): INTEGER;
                BEGIN 
                    RETURN x;
                END g;

            BEGIN
                f();
                g(7);
            END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nPROCEDURE f(): INTEGER;\nCONST\nx = 2;\nPROCEDURE g(): "
         "INTEGER;\nCONST\nx = 1;\nBEGIN\nRETURN x\nEND g;\nBEGIN\ng();\nRETURN x\nEND "
         "f;\nPROCEDURE g(x : INTEGER): INTEGER;\nBEGIN\nRETURN x\nEND g;\nBEGIN\nf();\ng(7)\nEND "
         "alpha.",
         ""},

        // Errors
        {R"(MODULE alpha; (* Nested Procedures *)
            VAR x: INTEGER;

            PROCEDURE f(): INTEGER;
                VAR g: INTEGER;             
                PROCEDURE g(): INTEGER;
                BEGIN 
                    RETURN 1;
                END g;
            BEGIN
                RETURN g();
            END f;

            BEGIN
                f();
            END alpha.)",
         "", "6,27: PROCEDURE g, identifier is already defined"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, ProcReciever) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha; (* Procedures with recievers *)
            TYPE pt = RECORD x, y : INTEGER; END;

            PROCEDURE (a : pt) print();
            BEGIN
            END print;

            PROCEDURE (VAR a : pt) clear();
            BEGIN
                a.x := 0; a.y := 0;
            END clear;

            PROCEDURE (VAR a : pt) set(x, y: INTEGER);
            BEGIN
                a.x := x; a.y := y;
            END set;

            END alpha.)",
         "MODULE alpha;\nTYPE\npt = RECORD\nx: INTEGER;\ny: INTEGER\nEND;\nPROCEDURE (a : pt) "
         "print;\nEND print;\nPROCEDURE (VAR a : pt) clear;\nBEGIN\na.x := 0;\na.y := 0\nEND "
         "clear;\nPROCEDURE (VAR a : pt) set(x : INTEGER; y : INTEGER);\nBEGIN\na.x := x;\na.y := "
         "y\nEND set;\nEND alpha.",
         ""},

        // Errors

        {R"(MODULE alpha; (* Procedures with receivers *)
            PROCEDURE (a : INTEGER) print();
            BEGIN
            END print;

            END alpha.)",
         "",
         "2,26: bound type INTEGER must be a RECORD or POINTER TO RECORD in type-bound PROCEDURE"},

        {R"(MODULE alpha; (* Procedures with receivers *)
            PROCEDURE (a : pt) print();
            BEGIN
            END print;

            END alpha.)",
         "", "2,26: bound type pt not found for type-bound PROCEDURE"},

    };
    do_inspect_tests(tests);
}

TEST(Inspector, ProcRecieverCall) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha; (* Procedures with recievers *)
            TYPE pt = RECORD x, y : INTEGER; END;
            VAR x : pt;

            PROCEDURE (a : pt) print();
            BEGIN
            END print;

            PROCEDURE (VAR a : pt) clear();
            BEGIN
                a.x := 0; a.y := 0;
            END clear;

            PROCEDURE (VAR a : pt) set(x, y: INTEGER);
            BEGIN
                a.x := x; a.y := y;
            END set;

            BEGIN
                x.clear;
                x.set(1, 2);
                x.print;
            END alpha.)",
         "MODULE alpha;\nTYPE\npt = RECORD\nx: INTEGER;\ny: INTEGER\nEND;\nVAR\nx: pt;\nPROCEDURE "
         "(a : pt) print;\nEND print;\nPROCEDURE (VAR a : pt) clear;\nBEGIN\na.x := 0;\na.y := "
         "0\nEND clear;\nPROCEDURE (VAR a : pt) set(x : INTEGER; y : INTEGER);\nBEGIN\na.x := "
         "x;\na.y := y\nEND set;\nBEGIN\nx.clear();\nx.set(1, 2);\nx.print()\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha; (* Procedures with recievers *)
            TYPE pt = RECORD x, y : INTEGER; END;
            VAR x : INTEGER;

            PROCEDURE (a : pt) print();
            BEGIN
            END print;

            BEGIN
                x.print;
            END alpha.)",
         "", "10,24: base type: INTEGER does not match bound procedure print, type: {x,y}"},

        {R"(MODULE alpha; (* Procedures with recievers *)
            TYPE pt = RECORD x, y : INTEGER; END;
            VAR x : INTEGER;

            PROCEDURE (a : pt) print();
            BEGIN
            END print;

            BEGIN
                print(x);
            END alpha.)",
         "", "10,22: calling PROCEDURE print, incorrect number of arguments: 1 instead of 0"},

        {R"(MODULE alpha; (* Procedures with recievers *)
            TYPE pt = RECORD x, y : INTEGER; END;
            VAR x : pt;

            PROCEDURE (a : pt) print();
            BEGIN
            END print;

            BEGIN
                x.jones;
            END alpha.)",
         "", "10,24: jones is not a PROCEDURE"},

        {R"(MODULE alpha; (* Procedures with recievers *)
            TYPE pt = RECORD x, y : INTEGER; END;
            VAR x : pt;

            PROCEDURE (a : pt) print();
            BEGIN
            END print;

            BEGIN
                x.x := 1;
                x.x();
            END alpha.)",
         "", "11,20: x is not a PROCEDURE"},

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

        {R"(MODULE xxx;
            BEGIN
            RETURN 1 < 2
            END xxx.)",
         "MODULE xxx;\nBEGIN\nRETURN 1 < 2\nEND xxx.", ""},

        {R"(MODULE xxx;
            BEGIN
            RETURN 1.0 < 2.0
            END xxx.)",
         "MODULE xxx;\nBEGIN\nRETURN 1.0 < 2.0\nEND xxx.", ""},

        // CHAR

        {R"(MODULE xxx;
            BEGIN
            RETURN 'a' = 'b'
            END xxx.)",
         "MODULE xxx;\nBEGIN\nRETURN 'a' = 'b'\nEND xxx.", ""},

        {R"(MODULE xxx;
            BEGIN
            RETURN 'a' >= 'b'
            END xxx.)",
         "MODULE xxx;\nBEGIN\nRETURN 'a' >= 'b'\nEND xxx.", ""},

        // Mixed types

        {R"(MODULE xxx;
            BEGIN
            RETURN 1.0 < 2
            END xxx.)",
         "MODULE xxx;\nBEGIN\nRETURN 1.0 < 2\nEND xxx.", ""},

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

        {R"(MODULE alpha;
                TYPE time = INTEGER;
                PROCEDURE T(): time;
                BEGIN
                    RETURN 0;
                END T;
                BEGIN
                    RETURN 0;
                END alpha.)",
         "MODULE alpha;\nTYPE\ntime = INTEGER;\nPROCEDURE T(): time;\nBEGIN\nRETURN 0\nEND "
         "T;\nBEGIN\nRETURN 0\nEND alpha.",
         ""},

        {R"(MODULE C1;
            PROCEDURE Sub(a, b: LONGINT): LONGINT;
            BEGIN
                RETURN a-b;
            END Sub;
            PROCEDURE f*(a, b: LONGINT): LONGINT;
            BEGIN
                RETURN Sub(a, b);
            END f;
            END C1.)",
         "MODULE C1;\nPROCEDURE Sub(a : LONGINT; b : LONGINT): LONGINT;\nBEGIN\nRETURN a-b\nEND "
         "Sub;\nPROCEDURE f*(a : LONGINT; b : LONGINT): LONGINT;\nBEGIN\nRETURN Sub(a, b)\nEND "
         "f;\nEND C1.",
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

        {R"(MODULE alpha;
                TYPE CHAR = CHAR;
            END alpha.)",
         "", "2,20: TYPE CHAR already defined"},

        {R"(MODULE alpha;
                TYPE time = INTEGER;
                PROCEDURE T(): time;
                BEGIN
                    RETURN 'x';
                END T;
                BEGIN
                    RETURN 0;
                END alpha.)",
         "", "5,26: RETURN type (INTEGER) does not match return type for function T: CHAR"},

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

        {R"(MODULE alpha;
            TYPE I = INTEGER;
            VAR x : ARRAY 3 OF I;
            BEGIN
                x[0] := 1;
                RETURN x;
            END alpha.)",
         "MODULE alpha;\nTYPE\nI = INTEGER;\nVAR\nx: ARRAY 3 OF I;\nBEGIN\nx[0] := 1;\nRETURN "
         "x\nEND alpha.",
         ""},

        {R"(MODULE alpha;
                TYPE I = INTEGER;
                VAR pt : RECORD
                        x,y : I
                    END;
                BEGIN
                    RETURN pt.x + pt.y
                END alpha.)",
         "MODULE alpha;\nTYPE\nI = INTEGER;\nVAR\npt: RECORD\nx: I;\ny: I\nEND;\nBEGIN\nRETURN "
         "pt.x+pt.y\nEND alpha.",
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

TEST(Inspector, TypeCompatible) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha; (* compatibity *)
            TYPE I = INTEGER;
            VAR x : SHORTINT;
                xx : I;
                y : LONGINT;
                z : HUGEINT;
                f : LONGREAL;
                
            BEGIN
                x := 1;
                x := xx + 1;
                y := x + 2;
                z := y - 4;
                f := 1.0 + z;
                RETURN x;
            END alpha.)",
         "MODULE alpha;\nTYPE\nI = INTEGER;\nVAR\nx: SHORTINT;\nxx: I;\ny: LONGINT;\nz: "
         "HUGEINT;\nf: LONGREAL;\nBEGIN\nx := 1;\nx := xx+1;\ny := x+2;\nz := y-4;\nf := "
         "1.0+z;\nRETURN x\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
            VAR x : SHORTINT;
                f : LONGREAL;
                
            BEGIN
                x := f;
                RETURN x;
            END alpha.)",
         "", "6,20: Can't assign expression of type REAL to x"},

        {R"(MODULE alpha; (* compatibity *)
            TYPE I = INTEGER;
            VAR x : I;
                f : LONGREAL;
            BEGIN
                f := x;
                RETURN x;
            END alpha.)",
         "", "6,20: Can't assign expression of type INTEGER to f"},

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

TEST(Inspector, RealExpr) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
                CONST pi = 3.14159269;
                VAR x, y : REAL;
                BEGIN
                    x := 2.3 * 2.3 * pi;
                    y := (x + 1) / 2;
                    RETURN 0
                END alpha.)",
         "MODULE alpha;\nCONST\npi = 3.14159269;\nVAR\nx: REAL;\ny: REAL;\nBEGIN\nx := "
         "2.3*2.3*pi;\ny :=  (x+1)  / 2;\nRETURN 0\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
                CONST pi = 3.14159269;
                VAR x, y : INTEGER;
                BEGIN
                    x := 2.3 * 2.3 * pi;
                    RETURN 0
                END alpha.)",
         "", "5,24: Can't assign expression of type REAL to x"},

        {R"(MODULE alpha;
                CONST pi = 3.14159269;
                VAR x : INTEGER;
                BEGIN
                    x := 1 * pi;
                    RETURN 0
                END alpha.)",
         "", "5,24: Can't assign expression of type REAL to x"},

        {R"(MODULE alpha;
                VAR x : INTEGER;
                BEGIN
                    x := 3 + 2.5;
                    RETURN 0
                END alpha.)",
         "", "4,24: Can't assign expression of type REAL to x"},

    };
    do_inspect_tests(tests);
}