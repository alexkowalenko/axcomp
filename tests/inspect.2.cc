//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "gtest/gtest.h"

#include "parse_test.hh"

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
         "", "4,14: IF expression must be type BOOLEAN"},

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
         "", "4,14: ELSIF expression must be type BOOLEAN"},

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
         "", "4,14: ELSIF expression must be type BOOLEAN"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, For) {
    std::vector<ParseTests> tests = {
        {R"(MODULE e06;
            BEGIN 
            FOR i := 0 TO 9 DO
                RETURN i
            END;
            RETURN
            END e06.)",
         "MODULE e06;\nBEGIN\nFOR i := 0 TO 9 DO\nRETURN i\nEND;\nRETURN \nEND "
         "e06.",
         ""},

        // Errors
        {R"(MODULE e06;
            BEGIN 
            FOR i := 0 TO TRUE DO 
                RETURN i
            END;
            RETURN
            END e06.)",
         "", "3,15: FOR end expression must be numeric type"},
        {R"(MODULE e06;
            BEGIN 
            FOR i := FALSE TO TRUE DO 
                RETURN i
            END;
            RETURN
            END e06.)",
         "", "3,15: FOR start expression must be numeric type"},
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
         "", "4,17: WHILE expression must be type BOOLEAN"},
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
         "", "4,18: REPEAT expression must be type BOOLEAN"},
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
         ""},
    };
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
                    1 .. 2 : x := 1;
                |   5, 6 .. 8, 9 :  x := 5;
                |   10 .. 11, 12 .. 15, 16 : x := 10;
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
                |   'D' .. 'F' : c := 'd';
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
         "6,23: CASE expression mismatch type CHAR does not match CASE expression type INTEGER"},

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
         "6,21: CASE expression mismatch type INTEGER does not match CASE expression type CHAR"},

        {R"(MODULE beta;
            VAR x :INTEGER;
            BEGIN
                CASE x OF
                    'a'.. 2 : x := 1;
                END
                RETURN 0
            END beta.)",
         "",
         "5,25: CASE expression range mismatch first type CHAR does not match CASE expression "
         "type INTEGER"},

        {R"(MODULE beta;
            VAR x :INTEGER;
            BEGIN
                CASE x OF
                    1 ..'z' : x := 1;
                END
                RETURN 0
            END beta.)",
         "",
         "5,24: CASE expression range mismatch last type CHAR does not match CASE expression type "
         "INTEGER"},

        {R"(MODULE alpha;
            VAR c : CHAR;
            BEGIN
                CASE c OF
                |   1 ..'F' : c := 'd';
                END
                RETURN 0;
            END alpha.)",
         "",
         "5,24: CASE expression range mismatch first type INTEGER does not match CASE expression "
         "type CHAR"},

        {R"(MODULE alpha;
            VAR c : CHAR;
            BEGIN
                CASE c OF
                |   'D' .. 99 : c := 'd';
                END
                RETURN 0;
            END alpha.)",
         "",
         "5,26: CASE expression range mismatch last type INTEGER does not match CASE expression "
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
         "MODULE g11;\nVAR\npt: ARRAY 3 OF INTEGER;\nPROCEDURE identity(a : "
         "ARRAY 3 OF INTEGER): ARRAY 3 OF INTEGER;\nBEGIN\nRETURN a\nEND "
         "identity;\nPROCEDURE sum(a : ARRAY 3 OF INTEGER): "
         "INTEGER;\nVAR\ntotal: INTEGER;\nBEGIN\nFOR i := 0 TO 2 DO\ntotal := "
         "total+a[i]\nEND;\nRETURN total\nEND sum;\nBEGIN\nFOR i := 0 TO 2 "
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