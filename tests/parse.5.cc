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
                VAR x : ARRAY 5 OF INTEGER;
                BEGIN
                    RETURN 0 
                END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY 5 OF INTEGER;\nBEGIN\nRETURN 0\nEND "
         "alpha.",
         ""},

        {R"(MODULE alpha;
                VAR x : ARRAY 5 OF INTEGER;
                VAR y : ARRAY 5 OF ARRAY 5 OF BOOLEAN;
                BEGIN
                    RETURN 0
                END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY 5 OF INTEGER;\ny: ARRAY 5 OF ARRAY "
         "5 OF BOOLEAN;\nBEGIN\nRETURN 0\nEND alpha.",
         ""},

        {R"(MODULE alpha;
                VAR x : ARRAY 5 OF INTEGER;
                VAR y : ARRAY 5 OF ARRAY 5 OF INTEGER;

                PROCEDURE sum(a : ARRAY 5 OF BOOLEAN) : INTEGER;
                BEGIN
                    RETURN 0
                END sum;

                PROCEDURE add(a : ARRAY 5 OF INTEGER; a : ARRAY 5 OF INTEGER) : ARRAY 5 OF INTEGER;
                BEGIN
                    RETURN 0
                END add;

                BEGIN
                    RETURN 0
                END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY 5 OF INTEGER;\ny: ARRAY 5 OF ARRAY "
         "5 OF INTEGER;\nPROCEDURE sum(a : ARRAY 5 OF BOOLEAN): "
         "INTEGER;\nBEGIN\nRETURN 0\nEND sum;\nPROCEDURE add(a : ARRAY 5 OF "
         "INTEGER; a : ARRAY 5 OF INTEGER): ARRAY 5 OF "
         "INTEGER;\nBEGIN\nRETURN 0\nEND add;\nBEGIN\nRETURN 0\nEND alpha.",
         ""},

        {R"(MODULE alpha;
                VAR x : ARRAY  OF INTEGER;
                BEGIN
                    RETURN 0
                END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY OF INTEGER;\nBEGIN\nRETURN 0\nEND alpha.", ""},

        // Errors

        {R"(MODULE alpha;
                VAR x : ARRAY 5 INTEGER;
                BEGIN
                    RETURN 0
                END alpha.)",
         "", "2,39: Unexpected token: INTEGER - expecting OF"},

        {R"(MODULE alpha;
                VAR x : ARRAY 5 OF ;
                BEGIN
                    RETURN 0
                END alpha.)",
         "", "2,36: Unexpected token: semicolon - expecting indent"},

    };
    do_parse_tests(tests);
}

TEST(Parser, ARRAY_Dimensions) {
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

        // Errors
        {R"(MODULE alpha;
                VAR tensor : ARRAY 3 3 OF INTEGER;
                BEGIN
                    RETURN 0 
                END alpha.)",
         "", "2,38: Unexpected token: integer(3) - expecting OF"},

    };
    do_parse_tests(tests);
}

TEST(Parser, ArrayIndex) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
            VAR x : ARRAY 5 OF INTEGER;
            BEGIN
                RETURN x[2]
            END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY 5 OF INTEGER;\nBEGIN\nRETURN "
         "x[2]\nEND alpha.",
         ""},

        {R"(MODULE alpha;
            VAR x : ARRAY 5 OF INTEGER;
            VAR y : ARRAY 5 OF ARRAY 5 OF BOOLEAN;
            VAR z : INTEGER;
            BEGIN
                RETURN x[2] + y[z+1][0]
            END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY 5 OF INTEGER;\ny: ARRAY 5 OF ARRAY "
         "5 OF BOOLEAN;\nz: INTEGER;\nBEGIN\nRETURN x[2]+y[z+1][0]\nEND "
         "alpha.",
         ""},

        // Errors

        {R"(MODULE alpha;
            VAR x : ARRAY 5 OF INTEGER;
            BEGIN
                RETURN x 2]
            END alpha.)",
         "", "4,26: Unexpected token: integer(2)"},
        {R"(MODULE alpha;
            VAR x : ARRAY 5 OF INTEGER;
            BEGIN
                RETURN x[]
            END alpha.)",
         "", "4,26: Unexpected token: ]"},
        {R"(MODULE alpha;
            VAR x : ARRAY 5 OF INTEGER;
            BEGIN
                RETURN x[2
            END alpha.)",
         "", "5,15: Unexpected token: END"},
    };

    do_parse_tests(tests);
}

TEST(Parser, ArrayIndexAssign) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
            VAR x : ARRAY 5 OF INTEGER;
            BEGIN
                x[2] := 2;
                RETURN x[2]
            END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY 5 OF INTEGER;\nBEGIN\nx[2] := "
         "2;\nRETURN x[2]\nEND alpha.",
         ""},

        {R"(MODULE alpha;
            VAR x : ARRAY 5 OF INTEGER;
            VAR y : ARRAY 5 OF ARRAY 5 OF BOOLEAN;
            BEGIN
                 x[1 + 1] := 2;
                 y[1][2] := TRUE;
                RETURN x[2] + y[z+1][0]
            END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY 5 OF INTEGER;\ny: ARRAY 5 OF ARRAY "
         "5 OF BOOLEAN;\nBEGIN\nx[1+1] := 2;\ny[1][2] := TRUE;\nRETURN "
         "x[2]+y[z+1][0]\nEND alpha.",
         ""},

        // Errors

        {R"(MODULE alpha;
            VAR x : ARRAY 5 OF INTEGER;
            BEGIN
                 x 2] := 2;
                RETURN 0
            END alpha.)",
         "", "4,20: Unexpected token: integer(2) - expecting :="},
        {R"(MODULE alpha;
            VAR x : ARRAY 5 OF INTEGER;
            BEGIN
                x[] := 2;
                RETURN x[2]
            END alpha.)",
         "", "4,19: Unexpected token: ]"},
        {R"(MODULE alpha;
            VAR x : ARRAY 5 OF INTEGER;
            BEGIN
                x[2 := 2;
                RETURN x[2]
            END alpha.)",
         "", "4,22: Unexpected token: :="},
    };

    do_parse_tests(tests);
}

TEST(Parser, ArrayIndex_Dimension) {
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
            BEGIN
                FOR i := 0 TO 4 DO
                    FOR j := 0 TO 4 DO
                        tensor[i, j] := i*i + j;
                    END
                END
                RETURN tensor[4, 4]
            END alpha.)",
         "MODULE alpha;\nVAR\ntensor: ARRAY 5, 5 OF INTEGER;\nBEGIN\nFOR i := 0 TO 4 DO\nFOR j := "
         "0 TO 4 DO\ntensor[i,j] := i*i+j\nEND\nEND;\nRETURN tensor[4,4]\nEND alpha.",
         ""},

        {R"(MODULE alpha;
            VAR tensor : ARRAY 5, 5, 5 OF INTEGER;
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
         "MODULE alpha;\nVAR\ntensor: ARRAY 5, 5, 5 OF INTEGER;\nBEGIN\nFOR i := 0 TO 4 DO\nFOR j "
         ":= 0 TO 4 DO\nFOR k := 0 TO 4 DO\ntensor[i,j,k] := i*i*i+j*j+k\nEND\nEND\nEND;\nRETURN "
         "tensor[4,4,4]\nEND alpha.",
         ""},

        //
        {R"(MODULE alpha;
            VAR x : ARRAY 5, 5 OF INTEGER;
            BEGIN
                RETURN x[2, ]
            END alpha.)",
         "", "4,29: Unexpected token: ]"},
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
         "MODULE alpha;\nVAR\npt: RECORD\nx: INTEGER;\ny: INTEGER\nEND;\nBEGIN\nRETURN 0\nEND "
         "alpha.",
         ""},

        {R"(MODULE alpha;
                VAR pt : RECORD
                        x, y, z : INTEGER;
                    END;
                BEGIN
                    RETURN 0 
                END alpha.)",
         "MODULE alpha;\nVAR\npt: RECORD\nx: INTEGER;\ny: INTEGER;\nz: "
         "INTEGER\nEND;\nBEGIN\nRETURN 0\nEND alpha.",
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

TEST(Parser, RecordBase) {
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
                    pt3 = RECORD pt
                        z: INTEGER;
                    END; 
                END alpha.)",
         "", "2,23: Unexpected token: pt3 - expecting END"},

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
         "MODULE alpha;\nVAR\npt: RECORD\nx: INTEGER;\ny: INTEGER\nEND;\nBEGIN\nRETURN "
         "pt.x+pt.y\nEND alpha.",
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
         "MODULE alpha;\nVAR\npt: RECORD\nx: INTEGER;\ny: INTEGER;\nz: INTEGER\nEND;\nBEGIN\npt.x "
         ":= 1;\npt.y := 2;\npt.z := 3;\nRETURN pt.x*pt.y*pt.z\nEND alpha.",
         ""},

        {R"(MODULE alpha;
                VAR a : RECORD
                        x : INTEGER;
                        y : ARRAY 3 OF INTEGER;
                    END;
                BEGIN
                    a.x := 1;
                    a.y[1] := 2;
                    RETURN a.y[2]
                END alpha.)",
         "MODULE alpha;\nVAR\na: RECORD\nx: INTEGER;\ny: ARRAY 3 OF INTEGER\nEND;\nBEGIN\na.x := "
         "1;\na.y[1] := 2;\nRETURN a.y[2]\nEND alpha.",
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

TEST(Parser, RecordArrayProcedures) {
    std::vector<ParseTests> tests = {

        {R"(MODULE g11; (* Mix ARRAY and RECORD *)
        
            VAR pt : ARRAY 3 OF INTEGER;

            PROCEDURE identity(a : ARRAY 3 OF INTEGER) : ARRAY 3 OF INTEGER;
            BEGIN
                RETURN a
            END identity;

            PROCEDURE sum(a : ARRAY 3 OF INTEGER) : INTEGER;
            VAR total : INTEGER;
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
         "MODULE g11;\nVAR\npt: ARRAY 3 OF INTEGER;\nPROCEDURE identity(a : ARRAY 3 OF INTEGER): "
         "ARRAY 3 OF INTEGER;\nBEGIN\nRETURN a\nEND identity;\nPROCEDURE sum(a : ARRAY 3 OF "
         "INTEGER): INTEGER;\nVAR\ntotal: INTEGER;\nBEGIN\nFOR i := 0 TO 2 DO\ntotal := "
         "total+a[i]\nEND;\nRETURN total\nEND sum;\nBEGIN\nFOR i := 0 TO 2 DO\npt[i] := "
         "i*i+i+1\nEND;\nRETURN sum(identity(pt))\nEND g11.",
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
    };
    do_parse_tests(tests);
}

TEST(Parser, POINTER) {
    std::vector<ParseTests> tests = {

        {R"(MODULE g11; (* POINTER *)
           TYPE Frame = POINTER TO FrameDesc;
                FrameDesc = RECORD
                    col: INTEGER
                END ;
            END g11.)",
         "MODULE g11;\nTYPE\nFrame = POINTER TO FrameDesc;\nFrameDesc = RECORD\ncol: "
         "INTEGER\nEND;\nEND g11.",
         ""},

        {R"(MODULE g11; (* POINTER *)
           VAR Frame : POINTER TO INTEGER;
               FrameSet : POINTER TO ARRAY OF INTEGER;
               FrameTable : POINTER TO ARRAY OF POINTER TO INTEGER;
            END g11.)",
         "MODULE g11;\nVAR\nFrame: POINTER TO INTEGER;\nFrameSet: POINTER TO ARRAY OF "
         "INTEGER;\nFrameTable: POINTER TO ARRAY OF POINTER TO INTEGER;\nEND g11.",
         ""},

        // Errors

        {R"(MODULE g11; (* POINTER *)
           VAR Frame : POINTER INTEGER;
            END g11.)",
         "", "2,38: Unexpected token: INTEGER - expecting TO"},

        {R"(MODULE g11; (* POINTER *)
           VAR Frame : POINTER TO ;
            END g11.)",
         "", "2,35: Unexpected token: semicolon - expecting indent"},
    };
    do_parse_tests(tests);
}

TEST(Parser, NIL) {
    std::vector<ParseTests> tests = {

        {R"(MODULE g11; (* POINTER *)
           VAR Frame : POINTER TO INTEGER;
               FrameSet : POINTER TO ARRAY OF INTEGER;
               FrameTable : POINTER TO ARRAY OF POINTER TO INTEGER;
            BEGIN
                Frame := NIL;
                IF Frame # NIL THEN
                END
            END g11.)",
         "MODULE g11;\nVAR\nFrame: POINTER TO INTEGER;\nFrameSet: POINTER TO ARRAY OF "
         "INTEGER;\nFrameTable: POINTER TO ARRAY OF POINTER TO INTEGER;\nBEGIN\nFrame := NIL;\nIF "
         "Frame # NIL THEN\nEND\nEND g11.",
         ""},

        // Errors

        {R"(MODULE g11; (* POINTER *)
           VAR Frame : POINTER TO INTEGER;
            BEGIN
                FRAME := NIL NIL;
            END g11.)",
         "", "4,32: Unexpected token: NIL"},
    };
    do_parse_tests(tests);
}

TEST(Parser, Reference) {
    std::vector<ParseTests> tests = {

        {R"(MODULE g11; (* POINTER *)
           VAR Frame : POINTER TO INTEGER;
               x : INTEGER;
            BEGIN
                Frame := NEW(INTEGER);
                Frame^ := 1;
                IF (Frame # NIL) & (Frame^ # 2) THEN
                    x := Frame^;
                END
            END g11.)",
         "MODULE g11;\nVAR\nFrame: POINTER TO INTEGER;\nx: INTEGER;\nBEGIN\nFrame := "
         "NEW(INTEGER);\nFrame^ := 1;\nIF  (Frame # NIL)  &  (Frame^ # 2)  THEN\nx := "
         "Frame^\nEND\nEND g11.",
         ""},

        // Errors
        {R"(MODULE g11; (* POINTER *)
           VAR Frame : POINTER TO INTEGER;
               x : INTEGER;
            BEGIN
                Frame^x := 1;
            END g11.)",
         "", "5,23: Unexpected token: x - expecting :="},

    };
    do_parse_tests(tests);
}

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
         "MODULE alpha;\nIMPORT System,\nB := A;\nBEGIN\nRETURN 0\nEND alpha.", ""},

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
         "System.Jones;\nBEGIN\nRETURN y\nEND f;\nBEGIN\nSystem.x := "
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

TEST(Parser, Char) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
                CONST a= 'a';
                VAR x : CHAR;
                    y : CHAR;
                    z : CHAR;
                    z1 : CHAR;

                BEGIN
                    x := 'α';
                    y := '四';
                    z := '👾';
                     z1 := 1F47EX;

                    RETURN a 
                END alpha.)",
         "MODULE alpha;\nCONST\na = 'a';\nVAR\nx: CHAR;\ny: CHAR;\nz: CHAR;\nz1: "
         "CHAR;\nBEGIN\nx := '\xCE\xB1';\ny := '\xE5\x9B\x9B';\nz := '\xF0\x9F\x91\xBE';\nz1 "
         ":= 01f47eX;\nRETURN a\nEND alpha.",
         ""},
        // Errors
    };
    do_parse_tests(tests);
}

TEST(Parser, String) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
                VAR x : STRING;
                    y : STRING;
                BEGIN
                    x := 'αβγ';
                    y := "Hello's there!";
                    RETURN 0 
                END alpha.)",
         "MODULE alpha;\nVAR\nx: STRING;\ny: STRING;\nBEGIN\nx := '\xCE\xB1\xCE\xB2\xCE\xB3';\ny "
         ":= \"Hello's there!\";\nRETURN 0\nEND alpha.",
         ""},
        // Errors

        {R"(MODULE alpha;
                VAR x : STRING;
                BEGIN
                    x := 'αβγ;
                    RETURN 0 
                END alpha.)",
         "", "4,31: Unterminated string"},

        {R"(MODULE alpha;
                VAR x : STRING;
                BEGIN
                    x := "Hello there!;
                    RETURN 0 
                END alpha.)",
         "", "4,31: Unterminated string"},
    };
    do_parse_tests(tests);
}

TEST(Parser, Set) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha; (* SET *)
                BEGIN
                x := {};
                x := {1};
                x := {1,2};
                END alpha.)",
         "MODULE alpha;\nBEGIN\nx := {};\nx := {1};\nx := {1,2}\nEND alpha.", ""},

        // Errors

        {R"(MODULE alpha; (* SET *)
                BEGIN
                x := {;
                END alpha.)",
         "", "3,23: Unexpected token: semicolon"},

        {R"(MODULE alpha; (* SET *)
                BEGIN
                x := {1;
                END alpha.)",
         "", "3,24: Unexpected token: semicolon - expecting ,"},

        {R"(MODULE alpha; (* SET *)
                BEGIN
                x := {1,;
                END alpha.)",
         "", "3,25: Unexpected token: semicolon"},

        {R"(MODULE alpha; (* SET *)
                BEGIN
                x := {,;
                END alpha.)",
         "", "3,23: Unexpected token: ,"},

        {R"(MODULE alpha; (* SET *)
                BEGIN
                x := };
                END alpha.)",
         "", "3,22: Unexpected token: }"},

        {R"(MODULE alpha; (* SET *)
                BEGIN
                x := ,};
                END alpha.)",
         "", "3,22: Unexpected token: ,"},
    };
    do_parse_tests(tests);
}

TEST(Parser, Set2) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha; (* SET *)
                BEGIN
                x := {1..6};
                x := {1..3,4..5,6..7};
                x := {1,2..5,33..44};
                END alpha.)",
         "MODULE alpha;\nBEGIN\nx := {1..6};\nx := {1..3,4..5,6..7};\nx := {1,2..5,33..44}\nEND "
         "alpha.",
         ""},

        // Errors

        {R"(MODULE alpha; (* SET *)
                BEGIN
                x := {1..};
                END alpha.)",
         "", "3,26: Unexpected token: }"},

        {R"(MODULE alpha; (* SET *)
                BEGIN
                x := {..6};
                END alpha.)",
         "", "3,24: Unexpected token: .."},

        {R"(MODULE alpha; (* SET *)
                BEGIN
                x := {1 6};
                END alpha.)",
         "", ",25: Unexpected token: integer(6) - expecting ,"},
    };
    do_parse_tests(tests);
}