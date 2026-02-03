//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "gtest/gtest.h"

#include "parse_test.hh"

#pragma clang diagnostic ignored "-Wmissing-field-initializers"

TEST(Inspector, VarType) {
    std::vector<ParseTests> tests = {
        {"MODULE x; VAR z: INTEGER; BEGIN RETURN END x.",
         "MODULE x;\nVAR\nz: INTEGER;\nBEGIN\nRETURN \nEND x.", ""},

        // Errors
        {"MODULE x; VAR z: complex; BEGIN x := 10 END x.", "", "[1,16]: Unknown type: complex"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, Enumeration) {
    std::vector<ParseTests> tests = {
        {R"(MODULE enum1;
            TYPE color = (red, green);
            VAR c: color;
            BEGIN
                c := red
            END enum1.)",
         "MODULE enum1;\nTYPE\ncolor = (red, green);\nVAR\nc: color;\nBEGIN\nc := red\nEND "
         "enum1.",
         ""},
        // Error
        {R"(MODULE enumdup;
            TYPE color = (red, green, red);
            BEGIN
            END enumdup.)",
         "", "[2,26]: Enumeration identifier red already defined"},
        {R"(MODULE enum1;
            TYPE color = (red, green);
            VAR c: color;
            BEGIN
                c := blue
            END enum1.)",
         "", "[6,15]: undefined identifier blue"},
        {R"(MODULE enum1;
            TYPE color = (red, green);
                 quality = (up, down, charm, strange, top, bottom);
            VAR c: color;
            BEGIN
                c := up
            END enum1.)",
         "", "[6,20]: Can't assign expression of type quality to c"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, EnumerationCastOrd) {
    std::vector<ParseTests> tests = {
        {R"(MODULE enumcast;
            TYPE color = (red, green, blue);
            VAR i: INTEGER; c: color;
            BEGIN
                i := ORD(green);
                c := CAST(color, 2)
            END enumcast.)",
         "MODULE enumcast;\nTYPE\ncolor = (red, green, blue);\nVAR\ni: INTEGER;\nc: "
         "color;\nBEGIN\ni "
         ":= ORD(green);\nc := CAST(color, 2)\nEND enumcast.",
         ""},

        // Errors
        {R"(MODULE enumcast1;
            TYPE color = (red, green, blue);
            VAR i: INTEGER;
            BEGIN
                i := ORD(1)
            END enumcast1.)",
         "", "[5,25]: procedure call ORD has incorrect type INTEGER for parameter CHAR"},
        {R"(MODULE enumcast2;
            TYPE color = (red, green, blue);
            VAR c: color;
            BEGIN
                c := CAST(INTEGER, 2)
            END enumcast2.)",
         "", "[5,26]: CAST expects enumeration type, got INTEGER"},
        {R"(MODULE enumcast3;
            TYPE color = (red, green, blue);
            VAR c: color;
            BEGIN
                c := CAST(color, TRUE)
            END enumcast3.)",
         "", "[5,26]: procedure call CAST has incorrect type BOOLEAN for parameter INTEGER"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, EnumerationMaxMinIncDec) {
    std::vector<ParseTests> tests = {
        {R"(MODULE enumbuiltins;
            TYPE color = (red, green, blue);
            VAR c: color;
            BEGIN
                c := MIN(color);
                c := MAX(color);
                c := INC(c);
                c := DEC(c)
            END enumbuiltins.)",
         "MODULE enumbuiltins;\nTYPE\ncolor = (red, green, blue);\nVAR\nc: color;\nBEGIN\nc "
         ":= MIN(color);\nc := MAX(color);\nc := INC(c);\nc := DEC(c)\nEND enumbuiltins.",
         ""},

        // Errors
        {"MODULE enummaxerr;\nTYPE color = (red, green);\nVAR c: color;\nBEGIN\nc := "
         "MAX(color, 1)\nEND enummaxerr.",
         "", "[5,9]: calling PROCEDURE MAX, incorrect number of arguments: 2 instead of 1"},
        {"MODULE enumincerr;\nTYPE color = (red, green);\nBEGIN\nINC(red)\nEND enumincerr.", "",
         "[4,4]: procedure call INC does not have a variable reference for VAR parameter any"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, UnknownExpr) {
    std::vector<ParseTests> tests = {
        {"MODULE y; VAR x : INTEGER; BEGIN RETURN x END y.",
         "MODULE y;\nVAR\nx: INTEGER;\nBEGIN\nRETURN x\nEND y.", ""},

        // Errors
        {"MODULE y; VAR x : INTEGER; BEGIN RETURN z END y.", "", "[1,45]: undefined identifier z"},
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
         "", "[5,23]: PROCEDURE f, identifier is already defined"},

        {R"(MODULE alpha; (* pointers *)
            VAR f: INTEGER;

            PROCEDURE f;
            END f;

            BEGIN
                f;
            END alpha.)",
         "", "[4,23]: PROCEDURE f, identifier is already defined"},
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
         "", "[1,24]: Unknown type: complex"},

        {"MODULE x; PROCEDURE f(): INTEGER; BEGIN RETURN END f; BEGIN "
         "RETURN 333 END x.",
         "", "[1,46]: RETURN type (INTEGER) does not match return type for function f: void"},

        {"MODULE x; PROCEDURE f; BEGIN RETURN 0 END f; BEGIN "
         "RETURN 333 END x.",
         "", "[1,35]: RETURN type (void) does not match return type for function f: INTEGER"},
        {R"(MODULE xxx;
            PROCEDURE f : BOOLEAN;
            BEGIN
            RETURN 123456
            END f;
            BEGIN
            RETURN 3
            END xxx.)",
         "", "[4,18]: RETURN type (BOOLEAN) does not match return type for function f: INTEGER"},
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
         "", "[1,68]: x is not a PROCEDURE"},
        {"MODULE y; VAR x : INTEGER; PROCEDURE f; BEGIN RETURN END f; BEGIN "
         "g(); RETURN x END y.",
         "", "[1,68]: undefined PROCEDURE g"},
        {"MODULE y; VAR x : INTEGER; "
         "PROCEDURE f():INTEGER; BEGIN RETURN 0 END f; "
         "BEGIN RETURN g() END y.",
         "", "[1,87]: undefined PROCEDURE g"},

        {R"(MODULE xxx;
            PROCEDURE f(x : INTEGER) : INTEGER;
            BEGIN
            RETURN 0
            END f;
            BEGIN
                RETURN f()
            END xxx.)",
         "",
         "[7,25]: calling PROCEDURE f, incorrect number of arguments: 0 instead "
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
         "[7,25]: calling PROCEDURE f, incorrect number of arguments: 4 instead "
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
         "[7,14]: procedure call f has incorrect type BOOLEAN for parameter "
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
         "[7,14]: procedure call f has incorrect type BOOLEAN for parameter "
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
         "[7,14]: procedure call f has incorrect type BOOLEAN for parameter "
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
         "", "[6,18]: procedure call g has incorrect type INTEGER for parameter CHAR"},

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
         "", "[6,18]: procedure call f has incorrect type CHAR for parameter INTEGER"},

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
         "[9,14]: procedure call f does not have a variable reference for VAR "
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
         "[9,14]: procedure call f does not have a variable reference for VAR "
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
         "",
         "[4,16]: procedure call INC does not have a variable reference for VAR parameter any"},

        {R"(MODULE alpha; (* SET *)
            BEGIN
            INC(1);
            END alpha.)",
         "",
         "[3,16]: procedure call INC does not have a variable reference for VAR parameter any"},

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
         "", "[3,27]: Unknown type: UNDEF"},
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
         "", "[6,27]: PROCEDURE g, identifier is already defined"},
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
         "[2,26]: bound type INTEGER must be a RECORD or POINTER TO RECORD in type-bound "
         "PROCEDURE"},

        {R"(MODULE alpha; (* Procedures with receivers *)
            PROCEDURE (a : pt) print();
            BEGIN
            END print;

            END alpha.)",
         "", "[2,26]: bound type pt not found for type-bound PROCEDURE"},

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
         "", "[10,24]: base type: INTEGER does not match bound procedure print, type: {x,y}"},

        {R"(MODULE alpha; (* Procedures with recievers *)
            TYPE pt = RECORD x, y : INTEGER; END;
            VAR x : INTEGER;

            PROCEDURE (a : pt) print();
            BEGIN
            END print;

            BEGIN
                print(x);
            END alpha.)",
         "", "[10,22]: calling PROCEDURE print, incorrect number of arguments: 1 instead of 0"},

        {R"(MODULE alpha; (* Procedures with recievers *)
            TYPE pt = RECORD x, y : INTEGER; END;
            VAR x : pt;

            PROCEDURE (a : pt) print();
            BEGIN
            END print;

            BEGIN
                x.jones;
            END alpha.)",
         "", "[10,24]: jones is not a PROCEDURE"},

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
         "", "[11,20]: x is not a PROCEDURE"},

    };
    do_inspect_tests(tests);
}

TEST(Inspector, ProcDerivedReciever) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha; (* Procedures with recievers *)
                TYPE    pt = RECORD x, y : INTEGER; END;
                        pt3 = RECORD(pt) z: INTEGER; END;

                VAR i: pt;
                    j: pt3;

                PROCEDURE (a : pt) print();
                BEGIN
                END print;

                BEGIN
                    i.print;
                    j.z := 5;
                    j.print;
                END alpha.)",
         "MODULE alpha;\nTYPE\npt = RECORD\nx: INTEGER;\ny: INTEGER\nEND;\npt3 = RECORD (pt)\nz: "
         "INTEGER\nEND;\nVAR\ni: pt;\nj: pt3;\nPROCEDURE (a : pt) print;\nEND "
         "print;\nBEGIN\ni.print();\nj.z := 5;\nj.print()\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha; (* Procedures with recievers *)
                TYPE    pt = RECORD x, y : INTEGER; END;
                        pts = RECORD x: INTEGER; END;

                VAR i: pt;
                    k: pts;

                PROCEDURE (a : pt) print();
                BEGIN
                END print;

                BEGIN
                    i.print; 
                    k.print; 
                END alpha.)",
         "", "[14,28]: base type: {x} does not match bound procedure print, type: {x,y}"},

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
         "", "[4,16]: Can't assign expression of type INTEGER to z"},
        {R"(MODULE xxx;
            VAR z : INTEGER;
            BEGIN
            z := TRUE;
            RETURN z
            END xxx.)",
         "", "[4,16]: Can't assign expression of type BOOLEAN to z"},
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
         "", "[3,23]: operator = doesn't takes types BOOLEAN and INTEGER"},
        {R"(MODULE xxx;
            BEGIN
            RETURN 0 # FALSE
            END xxx.)",
         "", "[3,20]: operator # doesn't takes types INTEGER and BOOLEAN"},
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
         "", "[3,23]: operator + doesn't takes types BOOLEAN and INTEGER"},
        {R"(MODULE xxx;
            BEGIN
            RETURN 0 OR FALSE
            END xxx.)",
         "", "[3,20]: operator OR doesn't takes types INTEGER and BOOLEAN"},
        {R"(MODULE xxx;
            BEGIN
            RETURN 0 OR 0
            END xxx.)",
         "", "[3,20]: operator OR doesn't takes types INTEGER and INTEGER"},
        {R"(MODULE xxx;
            BEGIN
            RETURN FALSE + FALSE
            END xxx.)",
         "", "[3,24]: operator + doesn't takes types BOOLEAN and BOOLEAN"},
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
         "", "[3,23]: operator * doesn't takes types BOOLEAN and INTEGER"},
        {R"(MODULE xxx;
            BEGIN
            RETURN 0 & FALSE
            END xxx.)",
         "", "[3,20]: operator & doesn't takes types INTEGER and BOOLEAN"},
        {R"(MODULE xxx;
            BEGIN
            RETURN TRUE DIV FALSE
            END xxx.)",
         "", "[3,23]: operator DIV doesn't takes types BOOLEAN and BOOLEAN"},
        {R"(MODULE xxx;
            BEGIN
            RETURN 0 & 23
            END xxx.)",
         "", "[3,20]: operator & doesn't takes types INTEGER and INTEGER"},
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
         "", "[3,20]: type in ~ expression must be BOOLEAN"},
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
         "", "[4,29]: Unknown type: complex"},

        {R"(MODULE alpha;
                TYPE CHAR = CHAR;
            END alpha.)",
         "", "[2,20]: TYPE CHAR already defined"},

        {R"(MODULE alpha;
                TYPE time = INTEGER;
                PROCEDURE T(): time;
                BEGIN
                    RETURN 'x';
                END T;
                BEGIN
                    RETURN 0;
                END alpha.)",
         "", "[5,26]: RETURN type (INTEGER) does not match return type for function T: CHAR"},

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
         "", "[5,30]: Can't assign expression of type BOOLEAN to seconds"},
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
         "", "[6,20]: Can't assign expression of type REAL to x"},

        {R"(MODULE alpha; (* compatibity *)
            TYPE I = INTEGER;
            VAR x : I;
                f : LONGREAL;
            BEGIN
                f := x;
                RETURN x;
            END alpha.)",
         "", "[6,20]: Can't assign expression of type INTEGER to f"},

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
         "", "[5,25]: CONST y is not a constant expression"},
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
         "", "[4,27]: Can't assign to CONST variable time"},
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
         "", "[5,24]: Can't assign expression of type REAL to x"},

        {R"(MODULE alpha;
                CONST pi = 3.14159269;
                VAR x : INTEGER;
                BEGIN
                    x := 1 * pi;
                    RETURN 0
                END alpha.)",
         "", "[5,24]: Can't assign expression of type REAL to x"},

        {R"(MODULE alpha;
                VAR x : INTEGER;
                BEGIN
                    x := 3 + 2.5;
                    RETURN 0
                END alpha.)",
         "", "[4,24]: Can't assign expression of type REAL to x"},

    };
    do_inspect_tests(tests);
}

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
         "", "[4,14]: IF expression must be type BOOLEAN"},

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
         "", "[4,14]: ELSIF expression must be type BOOLEAN"},

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
         "", "[4,14]: ELSIF expression must be type BOOLEAN"},
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
         "", "[4,15]: FOR end expression must be numeric type"},

        {R"(MODULE e06;
            VAR i: INTEGER;
            BEGIN 
            FOR i := FALSE TO TRUE DO 
                RETURN i
            END;
            RETURN
            END e06.)",
         "", "[4,15]: FOR start expression must be numeric type"},

        {R"(MODULE e06;
            BEGIN 
            FOR i := 0 TO 9 DO
                RETURN i
            END;
            RETURN
            END e06.)",
         "", "[3,15]: FOR index variable i not defined"},

        {R"(MODULE For2;
            TYPE
                T = INTEGER;
            BEGIN
                FOR T := 1 TO 8 DO
                END;
            END For2.)",
         "", "[5,19]: FOR index variable T not defined"},
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
         "", "[4,17]: WHILE expression must be type BOOLEAN"},
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
         "", "[4,18]: REPEAT expression must be type BOOLEAN"},
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
         ""}};
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
         "[6,23]: CASE expression mismatch type CHAR does not match CASE expression type INTEGER"},

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
         "[6,21]: CASE expression mismatch type INTEGER does not match CASE expression type CHAR"},

        {R"(MODULE beta;
            VAR x :INTEGER;
            BEGIN
                CASE x OF
                    'a'..2 : x := 1;
                END
                RETURN 0
            END beta.)",
         "",
         "[5,25]: CASE expression range mismatch first type CHAR does not match CASE expression "
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
         "[5,23]: CASE expression range mismatch last type CHAR does not match CASE expression "
         "type "
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
         "[5,23]: CASE expression range mismatch first type INTEGER does not match CASE "
         "expression "
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
         "[5,25]: CASE expression range mismatch last type INTEGER does not match CASE expression "
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
         "INTEGER;\nBEGIN\nRETURN 0\nEND f;\nBEGIN\nRETURN 0\nEND alpha.",
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
         "", "[2,21]: CONST a is always read only"},

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
         "", "[7,27]: PROCEDURE f is always read only"},
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
         "", "[2,19]: MODULE System not found"},

        {R"(MODULE alpha;
             IMPORT S := System;
             BEGIN
                 RETURN System.x;
             END alpha.)",
         "", "[2,24]: MODULE System not found"},

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
         "", "[4,28]: undefined identifier aa in MODULE beta"},

        // Try to access module without alias
        {R"(MODULE alpha;
             IMPORT B := beta;
             BEGIN
                 RETURN beta.a;
             END alpha.)",
         "", "[4,30]: undefined identifier beta"},

        // CONST
        {R"(MODULE alpha;
             IMPORT beta;
             BEGIN
                beta.a := 10;
                RETURN beta.a;
             END alpha.)",
         "", "[4,25]: Can't assign to CONST variable beta.a"},

        // Read only
        {R"(MODULE alpha;
             IMPORT beta;
             BEGIN
                beta.b := 0;
                 RETURN beta.a;
             END alpha.)",
         "", "[4,25]: Can't assign to read only (-) variable beta.b"},

        // Wrong type
        {R"(MODULE alpha;
             IMPORT beta;
             BEGIN
                beta.d := 0;
                RETURN beta.c;
             END alpha.)",
         "", "[4,25]: Can't assign expression of type INTEGER to beta.d"},

        // function does not exist
        {R"(MODULE alpha;
             IMPORT beta;
             BEGIN
                beta.g(1, 2);
                RETURN beta.g(100, 200);
             END alpha.)",
         "", "[4,23]: undefined PROCEDURE beta.g"},

        // Wrong parameters for function
        {R"(MODULE alpha;
             IMPORT beta;
             BEGIN
                beta.f(1, 1);
                RETURN beta.f(100);
             END alpha.)",
         "", "[4,23]: calling PROCEDURE beta.f, incorrect number of arguments: 2 instead of 1"},

    };
    do_inspect_fimport_tests(tests);
}

TEST(Inspector, CHAR) {
    std::vector<ParseTests> tests = {
        {R"(MODULE alpha;
            CONST a = 'α';
            TYPE character = CHAR;
            VAR x : CHAR;
                y : character;

            BEGIN
                x := 'a';
                y := 064X;
                RETURN a
            END alpha.)",
         "MODULE alpha;\nCONST\na = '\xCE\xB1';\nTYPE\ncharacter = CHAR;\nVAR\nx: CHAR;\ny: "
         "character;\nBEGIN\nx := 'a';\ny := 064X;\nRETURN a\nEND alpha.",
         ""},

        {R"(MODULE alpha;
            VAR x : ARRAY 3 OF CHAR;

            BEGIN
                x[0] := 'a';
                x[1] := 064X;
                x[2] := 'c';
                RETURN
            END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY 3 OF CHAR;\nBEGIN\nx[0] := 'a';\nx[1] := 064X;\nx[2] := "
         "'c';\nRETURN \nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
            VAR x : CHAR;
            BEGIN
                x := 1;
                RETURN
            END alpha.)",
         "", "[4,20]: Can't assign expression of type INTEGER to x"},

        {R"(MODULE alpha;
            VAR x : CHAR;
            BEGIN
                x := 'a' - 1;
                RETURN
            END alpha.)",
         "", "[4,20]: operator - doesn't takes types CHAR and INTEGER"},

        {R"(MODULE alpha;
            VAR x : ARRAY 3 OF CHAR;
            BEGIN
                x[0] := 1;
                RETURN
            END alpha.)",
         "", "[4,23]: Can't assign expression of type INTEGER to x[0]"},

        {R"(MODULE alpha;
            VAR x : ARRAY 3 OF CHAR;
            BEGIN
                x[0] := 1 * 'a';
                RETURN
            END alpha.)",
         "", "[4,25]: operator * doesn't takes types INTEGER and CHAR"},

    };
    do_inspect_tests(tests);
}

TEST(Inspector, String) {
    std::vector<ParseTests> tests = {
        {R"(MODULE alpha;
            CONST a = "abc";
            TYPE str8 = STRING;
            VAR x : STRING;
                y : str8;
            BEGIN
                x := "hello world!";
                y := 'olá';
                RETURN
            END alpha.)",
         "MODULE alpha;\nCONST\na = \"abc\";\nTYPE\nstr8 = STRING;\nVAR\nx: STRING;\ny: "
         "str8;\nBEGIN\nx := \"hello world!\";\ny := 'ol\xC3\xA1';\nRETURN \nEND alpha.",
         ""},

        {R"(MODULE alpha;
            VAR x : ARRAY 3 OF STRING;
            BEGIN
                x[0] := "hello world!";
                x[1] := 'olá';
                x[2] := 'ça va?';
                RETURN
            END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY 3 OF STRING;\nBEGIN\nx[0] := \"hello world!\";\nx[1] := "
         "'ol\xC3\xA1';\nx[2] := '\xC3\xA7"
         "a va?';\nRETURN \nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
            VAR x : STRING;
            BEGIN
                x := 8;
                RETURN
            END alpha.)",
         "", "[4,20]: Can't assign expression of type INTEGER to x"},

        {R"(MODULE alpha;
            VAR x : INTEGER;
            BEGIN
                x := "HELLO";
                RETURN
            END alpha.)",
         "", "[4,20]: Can't assign expression of type STRING to x"},

        {R"(MODULE alpha;
            VAR x : INTEGER;
            BEGIN
                x := 1 + "HELLO";
                RETURN
            END alpha.)",
         "", "[4,20]: operator + doesn't takes types INTEGER and STRING"},

        {R"(MODULE alpha;
            TYPE strArray = ARRAY 3 OF STRING;
            VAR x : strArray;
            BEGIN
                x[0] := "hello world!";
                x[1] := 6;
                RETURN
            END alpha.)",
         "", "[6,23]: Can't assign expression of type INTEGER to x[1]"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, ArrayCHAR) {
    std::vector<ParseTests> tests = {
        {R"(MODULE string9; (* compatibity ARRAY OF CHAR *)
            VAR x : ARRAY OF CHAR;
                y : ARRAY OF CHAR;
                z : STRING;
            PROCEDURE id(s : ARRAY OF CHAR) : ARRAY OF CHAR;
            BEGIN
                RETURN s;
            END id;
            BEGIN
                x := "Hello!";
                NEW(y, 5);
                z := x;
                x := id(x);
                RETURN 0;
            END string9.)",
         "MODULE string9;\nVAR\nx: ARRAY OF CHAR;\ny: ARRAY OF CHAR;\nz: STRING;\nPROCEDURE id(s "
         ": ARRAY OF CHAR): ARRAY OF CHAR;\nBEGIN\nRETURN s\nEND id;\nBEGIN\nx := "
         "\"Hello!\";\nNEW(y, 5);\nz := x;\nx := id(x);\nRETURN 0\nEND string9.",
         ""},

        // Errors
        {R"(MODULE string9; (* compatibity ARRAY OF CHAR *)
            VAR x : ARRAY OF CHAR;
                y : INTEGER;
            BEGIN
                x := "Hello!";
                y := x;
                RETURN 0;
            END string9.)",
         "", "[6,20]: Can't assign expression of type STRING to y"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, StringCat) {
    std::vector<ParseTests> tests = {
        {R"(MODULE alpha; (* STRING *)
            VAR s1, s2, c: STRING;
            BEGIN
                c := s1 + s2;
                RETURN 0
            END alpha.)",
         "MODULE alpha;\nVAR\ns1: STRING;\ns2: STRING;\nc: STRING;\nBEGIN\nc := s1+s2;\nRETURN "
         "0\nEND alpha."},

        {R"(MODULE alpha; (* STRING *)
            VAR c: STRING;
            BEGIN
                c := "Hello " + "world!";
                RETURN 0
            END alpha.)",
         "MODULE alpha;\nVAR\nc: STRING;\nBEGIN\nc := \"Hello \"+\"world!\";\nRETURN 0\nEND "
         "alpha."},

        {R"(MODULE alpha; (* STRING *)
            VAR s: STRING;
                c: CHAR;
            BEGIN
                s := "Hello " + c;
                s := c + s;
                RETURN 0
            END alpha.)",
         "MODULE alpha;\nVAR\ns: STRING;\nc: CHAR;\nBEGIN\ns := \"Hello \"+c;\ns := c+s;\nRETURN "
         "0\nEND alpha."},

        // Errors
        {R"(MODULE alpha; (* STRING *)
            VAR s1, c: STRING;
            BEGIN
                c := s1 + 1;
                RETURN 0
            END alpha.)",
         "", "[4,20]: operator + doesn't takes types STRING and INTEGER"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, StringCompare) {
    std::vector<ParseTests> tests = {
        {R"(MODULE alpha; (* STRING *)
            VAR s1, s2: STRING;
                c: BOOLEAN;
            BEGIN
                c := s1 = s2;
                c := s1 # s2;
                c := s1 < s2;
                c := s1 <= s2;
                c := s1 > s2;
                c := s1 >= s2;
                RETURN 0
            END alpha.)",
         "MODULE alpha;\nVAR\ns1: STRING;\ns2: STRING;\nc: BOOLEAN;\nBEGIN\nc := s1 = s2;\nc := "
         "s1 # s2;\nc := s1 < s2;\nc := s1 <= s2;\nc := s1 > s2;\nc := s1 >= s2;\nRETURN 0\nEND "
         "alpha.",
         ""},

        // Errors
        {R"(MODULE alpha; (* STRING *)
            VAR s1, s2: STRING;
                c: BOOLEAN;
            BEGIN
                c := c = s2;
                RETURN 0
            END alpha.)",
         "", "[5,20]: operator = doesn't takes types BOOLEAN and STRING"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, String1) {
    std::vector<ParseTests> tests = {
        {R"(MODULE alpha; (* 1 char strings function as CHAR *)
                PROCEDURE print(a : CHAR);
                END print;

                PROCEDURE print2(a : STRING);
                END print2;
                
                BEGIN
                print("C"); print('c');  print2("C");
                END alpha.)",
         "MODULE alpha;\nPROCEDURE print(a : CHAR);\nEND print;\nPROCEDURE print2(a : "
         "STRING);\nEND print2;\nBEGIN\nprint(\"C\");\nprint('c');\nprint2(\"C\")\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha; (* 1 char strings function as CHAR *)
                PROCEDURE print(a : CHAR);
                END print;

                PROCEDURE print2(a : STRING);
                END print2;
                
                BEGIN
                print2('c');
                END alpha.)",
         "", "[9,23]: procedure call print2 has incorrect type CHAR for parameter STRING"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, NIL) {
    std::vector<ParseTests> tests = {
        {R"(MODULE alpha; (* pointers *)
            VAR x : POINTER TO INTEGER;
            BEGIN
                x := NIL;
                RETURN 0;
            END alpha.)",
         "MODULE alpha;\nVAR\nx: POINTER TO INTEGER;\nBEGIN\nx := NIL;\nRETURN 0\nEND alpha.", ""},

        // Errors
        {R"(MODULE alpha; (* pointers *)
            VAR x : INTEGER;
            BEGIN
                x := NIL;
                RETURN 0;
            END alpha.)",
         "", "[4,20]: Can't assign expression of type void to x"},

        {R"(MODULE alpha; (* pointers *)
            VAR x : POINTER TO INTEGER;
            BEGIN
                x := 5;
                RETURN 0;
            END alpha.)",
         "", "[4,20]: Can't assign expression of type INTEGER to x"},

    };
    do_inspect_tests(tests);
}

TEST(Inspector, NILCompare) {
    std::vector<ParseTests> tests = {
        {R"(MODULE alpha; (* pointers *)
            VAR x : POINTER TO INTEGER;
            BEGIN
                RETURN x = NIL;
            END alpha.)",
         "MODULE alpha;\nVAR\nx: POINTER TO INTEGER;\nBEGIN\nRETURN x = NIL\nEND alpha.", ""},

        {R"(MODULE alpha; (* pointers *)
            VAR x : POINTER TO INTEGER;
            BEGIN
                RETURN x # NIL;
            END alpha.)",
         "MODULE alpha;\nVAR\nx: POINTER TO INTEGER;\nBEGIN\nRETURN x # NIL\nEND alpha.", ""},

        // Errors
        {R"(MODULE alpha; (* pointers *)
            VAR x : INTEGER;
            BEGIN
                RETURN x = NIL;
            END alpha.)",
         "", "[4,24]: operator = doesn't takes types INTEGER and void"},

        {R"(MODULE alpha; (* pointers *)
            VAR x : INTEGER;
            BEGIN
                RETURN x # NIL;
            END alpha.)",
         "", "[4,24]: operator # doesn't takes types INTEGER and void"},

    };
    do_inspect_tests(tests);
}

TEST(Inspector, Reference) {
    std::vector<ParseTests> tests = {
        {R"(MODULE alpha; (* pointers *)
            VAR x : POINTER TO INTEGER;
                y : INTEGER;
            BEGIN
                x^ := 5;
                x^ := y + x^;
                RETURN 0;
            END alpha.)",
         "MODULE alpha;\nVAR\nx: POINTER TO INTEGER;\ny: INTEGER;\nBEGIN\nx^ := 5;\nx^ := "
         "y+x^;\nRETURN 0\nEND alpha.",
         ""},

        {R"(MODULE alpha; (* pointers *)
            VAR x : POINTER TO POINTER TO INTEGER;
            BEGIN
                x^^ := 5;
                RETURN  x^^;
            END alpha.)",
         "MODULE alpha;\nVAR\nx: POINTER TO POINTER TO INTEGER;\nBEGIN\nx^^ := "
         "5;\nRETURN x^^\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha; (* pointers *)
            VAR x : INTEGER;
            BEGIN
                x^ := 5;
                RETURN 0;
            END alpha.)",
         "", "[4,17]: variable x is not an indexable type"},

        {R"(MODULE alpha; (* pointers *)
            VAR x : POINTER TO POINTER TO INTEGER;
            BEGIN
                x^ := 5;
                RETURN  x^^;
            END alpha.)",
         "", "[4,21]: Can't assign expression of type INTEGER to x^"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, ForwardPointers) {
    std::vector<ParseTests> tests = {
        {R"(MODULE pointer03; (* pointers *)
            TYPE 
                ListPtr = POINTER TO List;
                List = RECORD
                    value : INTEGER;
                END;
            VAR start : ListPtr;
            BEGIN
                RETURN 0;
            END pointer03.)",
         "MODULE pointer03;\nTYPE\nListPtr = POINTER TO List;\nList = RECORD\nvalue: "
         "INTEGER\nEND;\nVAR\nstart: ListPtr;\nBEGIN\nRETURN 0\nEND pointer03.",
         ""},

    };
    do_inspect_tests(tests);
}

TEST(Inspector, Set) {
    std::vector<ParseTests> tests = {
        {R"(MODULE alpha; (* SET *)
            VAR x: SET;
                BEGIN
                x := {};
                x := {1};
                x := {1,2};
                END alpha.)",
         "MODULE alpha;\nVAR\nx: SET;\nBEGIN\nx := {};\nx := {1};\nx := {1,2}\nEND alpha.", ""},

        {R"(MODULE alpha; (* SET *)
            VAR x: SET;
                y: INTEGER;
                BEGIN
                x := {1+2};
                x := {y};
                x := {y+1, y+2};
                END alpha.)",
         "MODULE alpha;\nVAR\nx: SET;\ny: INTEGER;\nBEGIN\nx := {1+2};\nx := {y};\nx := "
         "{y+1,y+2}\nEND alpha.",
         ""},

        {R"(MODULE alpha; (* SET *)
            VAR x: SET;
            BEGIN
                x := {1..6};
                x := {1..3,4..5,6..7};
                x := {1,2..5,33..44};
            END alpha.)",
         "MODULE alpha;\nVAR\nx: SET;\nBEGIN\nx := {1..6};\nx := {1..3,4..5,6..7};\nx := "
         "{1,2..5,33..44}\nEND alpha.",
         ""},

        {R"(MODULE alpha; (* SET *)
            VAR x: SET;
                a,b: INTEGER;
            BEGIN
                x := {a..b};
            END alpha.)",
         "MODULE alpha;\nVAR\nx: SET;\na: INTEGER;\nb: INTEGER;\nBEGIN\nx := {a..b}\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha; (* SET *)
            VAR x: SET;
                BEGIN
                x := {'c'};
                END alpha.)",
         "", "[4,25]: Expression 'c' is not a integer type"},

        {R"(MODULE alpha; (* SET *)
            VAR x: SET;
                y: REAL;
                BEGIN
                x := {y};
                END alpha.)",
         "", "[5,23]: Expression y is not a integer type"},

        {R"(MODULE alpha; (* SET *)
            VAR x: SET;
            BEGIN
                x := {"hello"..6};
            END alpha.)",
         "", "[4,29]: Expression \"hello\" is not a integer type"},

        {R"(MODULE alpha; (* SET *)
            VAR x: SET;
            BEGIN
                x := {1..'!'};
            END alpha.)",
         "", "[4,25]: Expression '!' is not a integer type"},

        {R"(MODULE alpha; (* SET *)
            VAR x: SET;
                a: INTEGER;
                b: CHAR;
            BEGIN
                x := {a..b};
            END alpha.)",
         "", "[6,25]: Expression b is not a integer type"},

        {R"(MODULE alpha; (* SET *)
            VAR x: SET;
                a: INTEGER;
                b: CHAR;
            BEGIN
                x := {b..a};
            END alpha.)",
         "", "[6,23]: Expression b is not a integer type"},

    };
    do_inspect_tests(tests);
}

TEST(Inspector, SetOps) {
    std::vector<ParseTests> tests = {
        {R"(MODULE alpha; (* SET *)
            VAR s, t: SET;
                b: BOOLEAN;
                x: INTEGER;
                BEGIN
                b := x IN s;
                b := (1 IN s) & (x IN s);
                b := s = t;
                b := s # t; 
                END alpha.)",
         "MODULE alpha;\nVAR\ns: SET;\nt: SET;\nb: BOOLEAN;\nx: INTEGER;\nBEGIN\nb := x IN s;\nb "
         ":=  (1 IN s)  &  (x IN s) ;\nb := s = t;\nb := s # t\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha; (* SET *)
            VAR s, t: SET;
                b: BOOLEAN;
                BEGIN
                b := 1.2 IN s;
                END alpha.)",
         "", "[5,20]: operator IN doesn't takes types REAL and SET"},

        {R"(MODULE alpha; (* SET *)
            VAR s, t: SET;
                b: BOOLEAN;
                BEGIN
                b := s IN s;
                END alpha.)",
         "", "[5,20]: operator IN doesn't takes types SET and SET"},

        {R"(MODULE alpha; (* SET *)
            VAR s, t: SET;
                b: BOOLEAN;
                BEGIN
                b := b = s;
                END alpha.)",
         "", "[5,20]: operator = doesn't takes types BOOLEAN and SET"},

        {R"(MODULE alpha; (* SET *)
            VAR s, t: SET;
                b: BOOLEAN;
                BEGIN
                b := 1 # s;
                END alpha.)",
         "", "[5,20]: operator # doesn't takes types INTEGER and SET"},

    };
    do_inspect_tests(tests);
}

TEST(Inspector, SetOps2) {
    std::vector<ParseTests> tests = {
        {R"(MODULE alpha; (* SET *)
            VAR s, t, u: SET;
                BEGIN
                u := s + t;
                u := s - t;
                u := s * t;
                u := s / t;
                END alpha.)",
         "MODULE alpha;\nVAR\ns: SET;\nt: SET;\nu: SET;\nBEGIN\nu := s+t;\nu := s-t;\nu := "
         "s*t;\nu := s / t\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha; (* SET *)
            VAR s, t: SET;
                u: INTEGER;
                BEGIN
                t := s + u;
                END alpha.)",
         "", "[5,20]: operator + doesn't takes types SET and INTEGER"},

        {R"(MODULE alpha; (* SET *)
            VAR s, t: SET;
                u: INTEGER;
                BEGIN
                u := s + t;
                END alpha.)",
         "", "[5,20]: Can't assign expression of type SET to u"},

    };
    do_inspect_tests(tests);
}

TEST(Inspector, Arrays) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
                VAR x : ARRAY 5 OF BOOLEAN;
                VAR y : ARRAY 5 OF ARRAY 5 OF INTEGER;

                PROCEDURE sum(a : ARRAY 3 OF BOOLEAN) : INTEGER;
                BEGIN
                    RETURN 0
                END sum;

                BEGIN
                    RETURN 0
                END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY 5 OF BOOLEAN;\ny: ARRAY 5 OF ARRAY "
         "5 OF INTEGER;\nPROCEDURE sum(a : ARRAY 3 OF BOOLEAN): "
         "INTEGER;\nBEGIN\nRETURN 0\nEND sum;\nBEGIN\nRETURN 0\nEND alpha."},

        // Errors

        {R"(MODULE alpha;
            VAR x : ARRAY 5 OF complex;
            BEGIN
                RETURN 0
            END alpha.)",
         "", "[2,30]: Unknown type: complex"},
        {R"(MODULE alpha;
            VAR x : ARRAY TRUE OF INTEGER;
            BEGIN
                RETURN 0 
            END alpha.)",
         "", "[2,30]: ARRAY expecting numeric size for dimension 0"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, ARRAY_Dimensions) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
                VAR x : ARRAY 5, 5 OF INTEGER;
                BEGIN
                    RETURN 0 
                END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY 5, 5 OF INTEGER;\nBEGIN\nRETURN 0\nEND "
         "alpha.",
         ""},

        {R"(MODULE alpha;
                VAR tensor : ARRAY 3, 3, 3 OF INTEGER;
                BEGIN
                    RETURN 0 
                END alpha.)",
         "MODULE alpha;\nVAR\ntensor: ARRAY 3, 3, 3 OF INTEGER;\nBEGIN\nRETURN 0\nEND "
         "alpha.",
         ""},

    };
    do_inspect_tests(tests);
}

TEST(Inspector, ArraysIndex) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
                VAR x : ARRAY 5 OF BOOLEAN;
                VAR y : ARRAY 5 OF ARRAY 5 OF INTEGER;

                BEGIN
                    RETURN x[1]
                END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY 5 OF BOOLEAN;\ny: ARRAY 5 OF ARRAY "
         "5 OF INTEGER;\nBEGIN\nRETURN x[1]\nEND alpha."},

        {R"(MODULE alpha;
                VAR x : ARRAY 5 OF BOOLEAN;
                VAR y : ARRAY 5 OF ARRAY 5 OF INTEGER;

                BEGIN
                    RETURN y[1][2] + y[2 + 3][2] 
                END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY 5 OF BOOLEAN;\ny: ARRAY 5 OF ARRAY "
         "5 OF INTEGER;\nBEGIN\nRETURN y[1][2]+y[2+3][2]\nEND alpha."},

        // Errors

        {R"(MODULE alpha;
                VAR x : INTEGER;
                BEGIN
                    RETURN x[1]
                END alpha.)",
         "", "[4,28]: variable x is not an indexable type"},

        {R"(MODULE alpha;
                VAR x : ARRAY 5 OF BOOLEAN;
                BEGIN
                    RETURN x[1] + 1
                END alpha.)",
         "", "[4,28]: operator + doesn't takes types BOOLEAN and INTEGER"},

        {R"(MODULE alpha;
                VAR x3 : ARRAY 6 OF BOOLEAN;
                BEGIN
                    RETURN x3[0] + 1
                END alpha.)",
         "", "[4,29]: operator + doesn't takes types BOOLEAN and INTEGER"},

        {R"(MODULE alpha;
                VAR x2 : ARRAY 5 OF ARRAY 5 OF INTEGER;
                BEGIN
                    RETURN x2[0] + 1
                END alpha.)",
         "", "[4,29]: operator + doesn't takes types INTEGER[5] and INTEGER"},

        {R"(MODULE alpha;
                VAR x2 : ARRAY 5 OF ARRAY 5 OF BOOLEAN;
                BEGIN
                    RETURN x2[0][0] + 1
                END alpha.)",
         "", "[4,29]: operator + doesn't takes types BOOLEAN and INTEGER"},

        {R"(MODULE alpha;
                VAR x2 : ARRAY 5 OF ARRAY 5 OF INTEGER;
                BEGIN
                    RETURN x2[0][2][3]
                END alpha.)",
         "", "[4,29]: value not indexable type"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, ArraysIndexAssign) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
                VAR x : ARRAY 5 OF BOOLEAN;

                BEGIN
                    x[1] := TRUE;
                    RETURN 0
                END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY 5 OF BOOLEAN;\nBEGIN\nx[1] := "
         "TRUE;\nRETURN 0\nEND alpha."},

        {R"(MODULE alpha;
                VAR y : ARRAY 5 OF ARRAY 5 OF INTEGER;

                BEGIN
                    y[1][2] := 8;
                    RETURN 0
                END alpha.)",
         "MODULE alpha;\nVAR\ny: ARRAY 5 OF ARRAY 5 OF "
         "INTEGER;\nBEGIN\ny[1][2] := 8;\nRETURN 0\nEND alpha."},

        // Errors

        {R"(MODULE alpha;
                VAR x : INTEGER;
                BEGIN
                    x[1] := 1;
                    RETURN 0
                END alpha.)",
         "", "[4,21]: variable x is not an indexable type"},

        {R"(MODULE alpha;
                VAR x : ARRAY 5 OF BOOLEAN;
                BEGIN
                    x[0] := 1;
                    RETURN 0
                END alpha.)",
         "", "[4,27]: Can't assign expression of type INTEGER to x[0]"},

        {R"(MODULE alpha;
                VAR x3 : ARRAY 6 OF INTEGER;
                BEGIN
                    x3[2] := TRUE;
                    RETURN 0
                END alpha.)",
         "", "[4,28]: Can't assign expression of type BOOLEAN to x3[2]"},

        {R"(MODULE alpha;
                VAR x2 : ARRAY 5 OF ARRAY 5 OF INTEGER;
                BEGIN
                    x2[1] := 1;
                    RETURN 0
                END alpha.)",
         "", "[4,28]: Can't assign expression of type INTEGER to x2[1]"},

        {R"(MODULE alpha;
                VAR x2 : ARRAY 5 OF ARRAY 5 OF BOOLEAN;
                BEGIN
                    x2[1][2] := 1;
                    RETURN 0
                END alpha.)",
         "", "[4,31]: Can't assign expression of type INTEGER to x2[1][2]"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, ArrayIndex_Dimension) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
            VAR x : ARRAY 5, 5 OF INTEGER;
            BEGIN
                x[2, 3] := 2;
                RETURN x[2, 3]
            END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY 5, 5 OF INTEGER;\nBEGIN\nx[2,3] := 2;\nRETURN x[2,3]\nEND "
         "alpha.",
         ""},

        {R"(MODULE alpha;
            VAR tensor : ARRAY 5, 5 OF INTEGER;
                i,j : INTEGER;
            BEGIN
                FOR i := 0 TO 4 DO
                    FOR j := 0 TO 4 DO
                        tensor[i, j] := i*i + j;
                    END
                END
                RETURN tensor[4, 4]
            END alpha.)",
         "MODULE alpha;\nVAR\ntensor: ARRAY 5, 5 OF INTEGER;\ni: INTEGER;\nj: "
         "INTEGER;\nBEGIN\nFOR i := 0 TO 4 DO\nFOR j := 0 TO 4 DO\ntensor[i,j] := "
         "i*i+j\nEND\nEND;\nRETURN tensor[4,4]\nEND alpha.",
         ""},

        {R"(MODULE alpha;
            VAR tensor : ARRAY 5, 5, 5 OF INTEGER;
                i,j,k : INTEGER;
            BEGIN
                FOR i := 0 TO 4 DO
                    FOR j := 0 TO 4 DO
                        FOR k := 0 TO 4 DO
                            tensor[i,j,k] := i*i*i + j*j + k;
                        END
                    END
                END
                RETURN tensor[4,4,4]
            END alpha.)",
         "MODULE alpha;\nVAR\ntensor: ARRAY 5, 5, 5 OF INTEGER;\ni: INTEGER;\nj: INTEGER;\nk: "
         "INTEGER;\nBEGIN\nFOR i := 0 TO 4 DO\nFOR j := 0 TO 4 DO\nFOR k := 0 TO 4 "
         "DO\ntensor[i,j,k] := i*i*i+j*j+k\nEND\nEND\nEND;\nRETURN tensor[4,4,4]\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
            VAR x : ARRAY 5, 5 OF INTEGER;
            BEGIN
                x[2, 3] := 3.3;
                RETURN x[2, 3]
            END alpha.)",
         "", "[4,26]: Can't assign expression of type REAL to x[2,3]"},

        {R"(MODULE alpha;
            VAR x : ARRAY 5, 5 OF INTEGER;
            BEGIN
                x[2] := 3;
                RETURN 0
            END alpha.)",
         "", "[4,17]: array indexes don't match array dimensions of x"},

    };

    do_inspect_tests(tests);
}

TEST(Inspector, Record) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
                VAR pt : RECORD
                        x : INTEGER;
                        y : INTEGER
                    END;
                BEGIN
                    RETURN 0 
                END alpha.)",
         "MODULE alpha;\nVAR\npt: RECORD\nx: INTEGER;\ny: INTEGER\nEND;\nBEGIN\nRETURN 0\nEND "
         "alpha."},

        // Errors
        {R"(MODULE alpha;
                VAR pt : RECORD
                        x : complex;
                        y : INTEGER
                    END;
                BEGIN
                    RETURN 0 
                END alpha.)",
         "", "[3,27]: Unknown type: complex"},
        {R"(MODULE alpha;
                VAR pt : RECORD
                        x : INTEGER;
                        x : INTEGER
                    END;
                BEGIN
                    RETURN 0 
                END alpha.)",
         "", "[4,25]: RECORD already has field x"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, RecordBase) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
                TYPE pt = RECORD
                        x: INTEGER;
                        y: INTEGER
                    END;
                    pt3 = RECORD (pt)
                        z: INTEGER;
                    END; 
                END alpha.)",
         "MODULE alpha;\nTYPE\npt = RECORD\nx: INTEGER;\ny: INTEGER\nEND;\npt3 = RECORD (pt)\nz: "
         "INTEGER\nEND;\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
                TYPE pt3 = RECORD (pt)
                        z: INTEGER;
                    END; 
                END alpha.)",
         "", "[2,38]: undefined identifier pt"},

        {R"(MODULE alpha;
                TYPE pt3 = RECORD (INTEGER)
                        z: INTEGER;
                    END; 
                END alpha.)",
         "", "[2,35]: RECORD base type INTEGER is not a record"},

        {R"(MODULE alpha;
                TYPE pt = RECORD
                        x: INTEGER;
                        y: INTEGER
                    END;
                    pt3 = RECORD (pt)
                        x: INTEGER;
                    END; 
                END alpha.)",
         "", "[7,25]: RECORD already has field x"},

    };
    do_inspect_tests(tests);
}

TEST(Inspector, RecordFields) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
                VAR pt : RECORD
                        x : INTEGER;
                        y : INTEGER
                    END;
                BEGIN
                    RETURN pt.x + pt.y
                END alpha.)",
         "MODULE alpha;\nVAR\npt: RECORD\nx: INTEGER;\ny: INTEGER\nEND;\nBEGIN\nRETURN "
         "pt.x+pt.y\nEND alpha.",
         ""},

        {R"(MODULE alpha;
                VAR pt : RECORD
                        x, y: INTEGER;
                        z : BOOLEAN;
                    END;
                BEGIN
                    pt.x := 1;
                    pt.y := 2;
                    pt.z := TRUE;
                    RETURN pt.x * pt.y
                END alpha.)",
         "MODULE alpha;\nVAR\npt: RECORD\nx: INTEGER;\ny: INTEGER;\nz: BOOLEAN\nEND;\nBEGIN\npt.x "
         ":= 1;\npt.y := 2;\npt.z := TRUE;\nRETURN pt.x*pt.y\nEND alpha.",
         ""},

        {R"(MODULE alpha;
                VAR pt : RECORD
                        x, y: INTEGER;
                        z :RECORD
                            x : INTEGER;
                            y : INTEGER
                        END;
                    END;
                BEGIN
                    pt.x := 1;
                    pt.y := 2;
                    pt.z.x := 1;
                    pt.z.y := 1;
                    RETURN pt.z.x * pt.z.y
                END alpha.)",
         "MODULE alpha;\nVAR\npt: RECORD\nx: INTEGER;\ny: INTEGER;\nz: RECORD\nx: INTEGER;\ny: "
         "INTEGER\nEND\nEND;\nBEGIN\npt.x := 1;\npt.y := 2;\npt.z.x := 1;\npt.z.y := 1;\nRETURN "
         "pt.z.x*pt.z.y\nEND alpha.",
         ""},

        {R"(MODULE alpha;
                TYPE pt = RECORD
                        x: INTEGER;
                        y: INTEGER
                    END;
                    pt3 = RECORD (pt)
                        z: INTEGER;
                    END;
                VAR a : pt3;
                BEGIN
                    RETURN a.x;
                END alpha.)",
         "MODULE alpha;\nTYPE\npt = RECORD\nx: INTEGER;\ny: INTEGER\nEND;\npt3 = RECORD (pt)\nz: "
         "INTEGER\nEND;\nVAR\na: pt3;\nBEGIN\nRETURN a.x\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
                VAR pt : RECORD
                        x : INTEGER;
                        y : INTEGER
                    END;
                BEGIN
                    RETURN pt.a + pt.y
                END alpha.)",
         "", "[7,29]: no field <a> in RECORD"},

        {R"(MODULE alpha;
                VAR pt : RECORD
                        x : INTEGER;
                        y : INTEGER
                    END;
                BEGIN
                    pt.spin := FALSE;
                    RETURN 0
                END alpha.)",
         "", "[7,22]: no field <spin> in RECORD"},

        {R"(MODULE alpha;
                VAR pt : RECORD
                        x, y: INTEGER;
                        z :RECORD
                            x : INTEGER;
                            y : INTEGER
                        END;
                    END;
                BEGIN
                    pt.f.x := 1;
                    RETURN 0
                END alpha.)",
         "", "[10,22]: no field <f> in RECORD"},

        {R"(MODULE alpha;
                VAR pt : RECORD
                        x, y: INTEGER;
                        z :RECORD
                            x : INTEGER;
                            y : INTEGER
                        END;
                    END;
                BEGIN
                    pt.z.g := 1;
                    RETURN 0
                END alpha.)",
         "", "[10,22]: no field <g> in RECORD"},

        {R"(MODULE alpha;
                VAR pt : RECORD
                        x, y: INTEGER;
                        z :RECORD
                            x : INTEGER;
                            y : INTEGER
                        END;
                    END;
                BEGIN
                    gt.z.x := 1;
                    RETURN 0
                END alpha.)",
         "", "[10,24]: undefined identifier gt"},

        {R"(MODULE alpha;
                TYPE pt = RECORD
                        x: INTEGER;
                        y: INTEGER
                    END;
                    pt3 = RECORD (pt)
                        z: INTEGER;
                    END;
                VAR a : pt3;
                BEGIN
                    RETURN a.d;
                END alpha.)",
         "", "[11,28]: no field <d> in RECORD"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, RecordAssign) {
    std::vector<ParseTests> tests = {

        // Errors

        {R"(MODULE alpha;
                TYPE pt2 = RECORD
                        x: INTEGER;
                        y: INTEGER
                    END;
                    pt3 = RECORD (pt2)
                        z: INTEGER;
                    END;
                VAR a, b : pt3;
                BEGIN
                    b := a;
                    RETURN a.x;
                END alpha.)",
         "", "[11,24]: Can't assign expression of type {({x,y})z} to b"},

        {R"(MODULE alpha;
                TYPE pt2 = RECORD
                        x: INTEGER;
                        y: INTEGER
                    END;
                    pt3 = RECORD (pt2)
                        z: INTEGER;
                    END;
                VAR a : pt3;
                    b : pt2;
                BEGIN
                    a := b; (* no data lost *)
                    RETURN a.x;
                END alpha.)",
         "", "[12,24]: Can't assign expression of type {x,y} to a"},

        {R"(MODULE alpha;
                TYPE pt2 = RECORD
                        x: INTEGER;
                        y: INTEGER
                    END;
                    pt3 = RECORD (pt2)
                        z: INTEGER;
                    END;
                VAR a : pt3;
                    b : pt2;
                BEGIN
                    b := a; (* data lost *)
                    RETURN a.x;
                END alpha.)",
         "", "[12,24]: Can't assign expression of type {({x,y})z} to b"},

        {R"(MODULE alpha;
                TYPE pt2 = RECORD
                        x: CHAR;
                    END;
                    pt3 = RECORD
                        z: INTEGER;
                    END;
                VAR a : pt3;
                    b : pt2;
                BEGIN
                    b := a; (* diferent *)
                    RETURN a.x;
                END alpha.)",
         "", "[11,24]: Can't assign expression of type {z} to b"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, RecordAssignPtr) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
                TYPE pt2 = RECORD
                        x: INTEGER;
                        y: INTEGER
                    END;
                    pt3 = RECORD (pt2)
                        z: INTEGER;
                    END;
                VAR a, aa : POINTER TO pt3;
                    b: POINTER TO pt2;
                BEGIN
                    aa := a;
                    b := a;
                    RETURN a^.x;
                END alpha.)",
         "MODULE alpha;\nTYPE\npt2 = RECORD\nx: INTEGER;\ny: INTEGER\nEND;\npt3 = RECORD "
         "(pt2)\nz: INTEGER\nEND;\nVAR\na: POINTER TO pt3;\naa: POINTER TO pt3;\nb: POINTER TO "
         "pt2;\nBEGIN\naa := a;\nb := a;\nRETURN a^.x\nEND alpha.",
         ""},

        // Errors
        // {R"(MODULE alpha;
        //         TYPE pt2 = RECORD
        //                 x: INTEGER;
        //                 y: INTEGER
        //             END;
        //             pt3 = RECORD (pt2)
        //                 z: INTEGER;
        //             END;
        //         VAR a, aa : POINTER TO pt3;
        //             b: POINTER TO pt2;
        //         BEGIN
        //             a := b;
        //             RETURN a^.x;
        //         END alpha.)",
        //  "MODULE alpha;\nTYPE\npt2 = RECORD\nx: INTEGER;\ny: INTEGER\nEND;\npt3 = RECORD "
        //  "(pt2)\nz: INTEGER\nEND;\nVAR\na: POINTER TO pt3;\naa: POINTER TO pt3;\nb: POINTER TO "
        //  "pt2;\nBEGIN\naa := a;\nb := a;\nRETURN a^.x\nEND alpha.",
        //  ""},

    };
    do_inspect_tests(tests);
}

TEST(Inspector, RecordArrayMix) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
                VAR pt : RECORD
                        x : INTEGER;
                        y : ARRAY 3 OF INTEGER;
                    END;
                BEGIN
                    pt.y[1] := 1;
                    RETURN pt.x + pt.y[2]
                END alpha.)",
         "MODULE alpha;\nVAR\npt: RECORD\nx: INTEGER;\ny: ARRAY 3 OF "
         "INTEGER\nEND;\nBEGIN\npt.y[1] := 1;\nRETURN pt.x+pt.y[2]\nEND alpha.",
         ""},

        {R"(MODULE alpha;
                VAR pt : ARRAY 3 OF RECORD
                        x, y: INTEGER;
                    END;
                BEGIN
                    pt[0].x := 1;
                    pt[0].y := 2;
                    RETURN pt[1].x * pt[1].y
                END alpha.)",
         "MODULE alpha;\nVAR\npt: ARRAY 3 OF RECORD\nx: INTEGER;\ny: "
         "INTEGER\nEND;\nBEGIN\npt[0].x := 1;\npt[0].y := 2;\nRETURN pt[1].x*pt[1].y\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
                VAR pt : RECORD
                        x : INTEGER;
                        y : ARRAY 3 OF INTEGER;
                    END;
                BEGIN
                    pt[1].y := 1;
                    RETURN 0
                END alpha.)",
         "", "[7,22]: value not indexable type"},

        {R"(MODULE alpha;
                VAR pt : ARRAY 3 OF RECORD
                        x, y: INTEGER;
                    END;
                BEGIN
                    pt.x[0] := 1;
                    RETURN 0
                END alpha.)",
         "", "[6,24]: value not RECORD: {x,y}[3]"},

    };
    do_inspect_tests(tests);
}

TEST(Inspector, RecordArrayProcedure) {
    std::vector<ParseTests> tests = {

        {R"(MODULE g11; (* Mix ARRAY and RECORD *)
        
            VAR pt : ARRAY 3 OF INTEGER;
                i: INTEGER;

            PROCEDURE identity(a : ARRAY 3 OF INTEGER) : ARRAY 3 OF INTEGER;
            BEGIN
                RETURN a
            END identity;

            PROCEDURE sum(a : ARRAY 3 OF INTEGER) : INTEGER;
            VAR total, i : INTEGER;
            BEGIN
                FOR i := 0 TO 2 DO
                    total := total + a[i]
                END;
                RETURN total
            END sum;

            BEGIN
                FOR i := 0 TO 2 DO
                    pt[i] := i*i + i + 1
                END;
                RETURN sum(identity(pt))
            END g11.)",
         "MODULE g11;\nVAR\npt: ARRAY 3 OF INTEGER;\ni: INTEGER;\nPROCEDURE identity(a : ARRAY 3 "
         "OF INTEGER): ARRAY 3 OF INTEGER;\nBEGIN\nRETURN a\nEND identity;\nPROCEDURE sum(a : "
         "ARRAY 3 OF INTEGER): INTEGER;\nVAR\ntotal: INTEGER;\ni: INTEGER;\nBEGIN\nFOR i := 0 TO "
         "2 DO\ntotal := total+a[i]\nEND;\nRETURN total\nEND sum;\nBEGIN\nFOR i := 0 TO 2 "
         "DO\npt[i] := i*i+i+1\nEND;\nRETURN sum(identity(pt))\nEND g11.",
         ""},

        {R"(MODULE g12; (* Mix ARRAY and RECORD *)
        
            VAR pt : RECORD x, y : INTEGER; END;

            PROCEDURE identity(a : RECORD x, y : INTEGER END) : RECORD x, y : INTEGER END;
            BEGIN
                RETURN a
            END identity;

            PROCEDURE sum(a : RECORD x, y : INTEGER END) : INTEGER;
            VAR total : INTEGER;
            BEGIN
                RETURN a.x + a.y
            END sum;

            BEGIN
                pt.x := 12;
                pt.y := 24;
                RETURN sum(identity(pt))
            END g12.)",
         "MODULE g12;\nVAR\npt: RECORD\nx: INTEGER;\ny: INTEGER\nEND;\nPROCEDURE identity(a : "
         "RECORD\nx: INTEGER;\ny: INTEGER\nEND): RECORD\nx: INTEGER;\ny: "
         "INTEGER\nEND;\nBEGIN\nRETURN a\nEND identity;\nPROCEDURE sum(a : RECORD\nx: "
         "INTEGER;\ny: INTEGER\nEND): INTEGER;\nVAR\ntotal: INTEGER;\nBEGIN\nRETURN a.x+a.y\nEND "
         "sum;\nBEGIN\npt.x := 12;\npt.y := 24;\nRETURN sum(identity(pt))\nEND g12.",
         ""},

        // Errors

        // Checking calling procedures with incorrect type arguments not
        // implemented yet.

    };
    do_inspect_tests(tests);
}
