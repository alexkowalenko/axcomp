//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "gtest/gtest.h"

#include "parse_test.hh"

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
         "", "2,30: Unknown type: complex"},
        {R"(MODULE alpha;
            VAR x : ARRAY TRUE OF complex;
            BEGIN
                RETURN 0 
            END alpha.)",
         "", "2,30: Unexpected token: TRUE - expecting integer"},
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
         "", "4,28: variable x is not an indexable type"},

        {R"(MODULE alpha;
                VAR x : ARRAY 5 OF BOOLEAN;
                BEGIN
                    RETURN x[1] + 1
                END alpha.)",
         "", "4,28: operator + doesn't takes types BOOLEAN and INTEGER"},

        {R"(MODULE alpha;
                VAR x3 : ARRAY 6 OF BOOLEAN;
                BEGIN
                    RETURN x3[0] + 1
                END alpha.)",
         "", "4,29: operator + doesn't takes types BOOLEAN and INTEGER"},

        {R"(MODULE alpha;
                VAR x2 : ARRAY 5 OF ARRAY 5 OF INTEGER;
                BEGIN
                    RETURN x2[0] + 1
                END alpha.)",
         "", "4,29: operator + doesn't takes types INTEGER[5] and INTEGER"},

        {R"(MODULE alpha;
                VAR x2 : ARRAY 5 OF ARRAY 5 OF BOOLEAN;
                BEGIN
                    RETURN x2[0][0] + 1
                END alpha.)",
         "", "4,29: operator + doesn't takes types BOOLEAN and INTEGER"},

        {R"(MODULE alpha;
                VAR x2 : ARRAY 5 OF ARRAY 5 OF INTEGER;
                BEGIN
                    RETURN x2[0][2][3]
                END alpha.)",
         "", "4,29: value not indexable type"},
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
         "", "4,21: variable x is not an indexable type"},

        {R"(MODULE alpha;
                VAR x : ARRAY 5 OF BOOLEAN;
                BEGIN
                    x[0] := 1;
                    RETURN 0
                END alpha.)",
         "", "4,27: Can't assign expression of type INTEGER to x[0]"},

        {R"(MODULE alpha;
                VAR x3 : ARRAY 6 OF INTEGER;
                BEGIN
                    x3[2] := TRUE;
                    RETURN 0
                END alpha.)",
         "", "4,28: Can't assign expression of type BOOLEAN to x3[2]"},

        {R"(MODULE alpha;
                VAR x2 : ARRAY 5 OF ARRAY 5 OF INTEGER;
                BEGIN
                    x2[1] := 1;
                    RETURN 0
                END alpha.)",
         "", "4,28: Can't assign expression of type INTEGER to x2[1]"},

        {R"(MODULE alpha;
                VAR x2 : ARRAY 5 OF ARRAY 5 OF BOOLEAN;
                BEGIN
                    x2[1][2] := 1;
                    RETURN 0
                END alpha.)",
         "", "4,31: Can't assign expression of type INTEGER to x2[1][2]"},
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
         "", "4,26: Can't assign expression of type REAL to x[2,3]"},

        {R"(MODULE alpha;
            VAR x : ARRAY 5, 5 OF INTEGER;
            BEGIN
                x[2] := 3;
                RETURN 0
            END alpha.)",
         "", "4,17: array indexes don't match array dimensions of x"},

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
         "", "3,27: Unknown type: complex"},
        {R"(MODULE alpha;
                VAR pt : RECORD
                        x : INTEGER;
                        x : INTEGER
                    END;
                BEGIN
                    RETURN 0 
                END alpha.)",
         "", "4,25: RECORD already has field x"},
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
         "", "2,38: undefined identifier pt"},

        {R"(MODULE alpha;
                TYPE pt3 = RECORD (INTEGER)
                        z: INTEGER;
                    END; 
                END alpha.)",
         "", "2,35: RECORD base type INTEGER is not a record"},

        {R"(MODULE alpha;
                TYPE pt = RECORD
                        x: INTEGER;
                        y: INTEGER
                    END;
                    pt3 = RECORD (pt)
                        x: INTEGER;
                    END; 
                END alpha.)",
         "", "7,25: RECORD already has field x"},

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
         "", "7,29: no field <a> in RECORD"},

        {R"(MODULE alpha;
                VAR pt : RECORD
                        x : INTEGER;
                        y : INTEGER
                    END;
                BEGIN
                    pt.spin := FALSE;
                    RETURN 0
                END alpha.)",
         "", "7,22: no field <spin> in RECORD"},

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
         "", "10,22: no field <f> in RECORD"},

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
         "", "10,22: no field <g> in RECORD"},

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
         "", "10,24: undefined identifier gt"},

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
         "", "11,28: no field <d> in RECORD"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, RecordAssign) {
    std::vector<ParseTests> tests = {

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
         "MODULE alpha;\nTYPE\npt2 = RECORD\nx: INTEGER;\ny: INTEGER\nEND;\npt3 = RECORD "
         "(pt2)\nz: "
         "INTEGER\nEND;\nVAR\na: pt3;\nb: pt3;\nBEGIN\nb := a;\nRETURN a.x\nEND alpha.",
         ""},

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
         "MODULE alpha;\nTYPE\npt2 = RECORD\nx: INTEGER;\ny: INTEGER\nEND;\npt3 = RECORD "
         "(pt2)\nz: INTEGER\nEND;\nVAR\na: pt3;\nb: pt2;\nBEGIN\na := b;\nRETURN a.x\nEND alpha.",
         ""},

        // Errors

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
         "", "12,24: Can't assign expression of type {({x,y})z} to b"},

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
         "", "11,24: Can't assign expression of type {z} to b"},
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
         "", "7,22: value not indexable type"},

        {R"(MODULE alpha;
                VAR pt : ARRAY 3 OF RECORD
                        x, y: INTEGER;
                    END;
                BEGIN
                    pt.x[0] := 1;
                    RETURN 0
                END alpha.)",
         "", "6,24: value not RECORD: {x,y}[3]"},

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