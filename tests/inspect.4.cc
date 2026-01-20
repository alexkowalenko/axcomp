//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "gtest/gtest.h"

#include "parse_test.hh"

#pragma clang diagnostic ignored "-Wmissing-field-initializers"

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
