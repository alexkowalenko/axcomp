//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "gtest/gtest.h"

#include "parse_test.hh"

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
         "", "4,20: Can't assign expression of type INTEGER to x"},

        {R"(MODULE alpha;
            VAR x : CHAR;
            BEGIN
                x := 'a' - 1;
                RETURN
            END alpha.)",
         "", "4,20: operator - doesn't takes types CHAR and INTEGER"},

        {R"(MODULE alpha;
            VAR x : ARRAY 3 OF CHAR;
            BEGIN
                x[0] := 1;
                RETURN
            END alpha.)",
         "", "4,23: Can't assign expression of type INTEGER to x[0]"},

        {R"(MODULE alpha;
            VAR x : ARRAY 3 OF CHAR;
            BEGIN
                x[0] := 1 * 'a';
                RETURN
            END alpha.)",
         "", "4,25: operator * doesn't takes types INTEGER and CHAR"},

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
         "", "4,20: Can't assign expression of type INTEGER to x"},

        {R"(MODULE alpha;
            VAR x : INTEGER;
            BEGIN
                x := "HELLO";
                RETURN
            END alpha.)",
         "", "4,20: Can't assign expression of type STRING to x"},

        {R"(MODULE alpha;
            VAR x : INTEGER;
            BEGIN
                x := 1 + "HELLO";
                RETURN
            END alpha.)",
         "", "4,20: operator + doesn't takes types INTEGER and STRING"},

        {R"(MODULE alpha;
            TYPE strArray = ARRAY 3 OF STRING;
            VAR x : strArray;
            BEGIN
                x[0] := "hello world!";
                x[1] := 6;
                RETURN
            END alpha.)",
         "", "6,23: Can't assign expression of type INTEGER to x[1]"},
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
         "", "4,20: Can't assign expression of type void to x"},

        {R"(MODULE alpha; (* pointers *)
            VAR x : POINTER TO INTEGER;
            BEGIN
                x := 5;
                RETURN 0;
            END alpha.)",
         "", "4,20: Can't assign expression of type INTEGER to x"},

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

        // {R"(MODULE alpha; (* pointers *)
        //     VAR x : POINTER TO POINTER TO INTEGER;
        //     BEGIN
        //         x^^ := 5;
        //         RETURN  x^^;
        //     END alpha.)",
        // "MODULE alpha;\nVAR\nx: POINTER TO POINTER TO INTEGER;\nBEGIN\nx^^ := "
        // "5;\nRETURN x^^\nEND alpha.",
        // ""},

        // Errors
        {R"(MODULE alpha; (* pointers *)
            VAR x : INTEGER;
            BEGIN
                x^ := 5;
                RETURN 0;
            END alpha.)",
         "", "4,17: variable x is not an indexable type"},

        // {R"(MODULE alpha; (* pointers *)
        //     VAR x : POINTER TO POINTER TO INTEGER;
        //     BEGIN
        //         x^ := 5;
        //         RETURN  x^^;
        //     END alpha.)",
        //  "", "4,17: value not indexable"},
    };
    do_inspect_tests(tests);
}