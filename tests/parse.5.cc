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
VAR x : ARRAY [5] OF INTEGER;
BEGIN
    RETURN 0; 
END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY [5] OF INTEGER;\nBEGIN\nRETURN 0;\nEND "
         "alpha.",
         ""},

        {R"(MODULE alpha;
VAR x : ARRAY [5] OF INTEGER;
VAR y : ARRAY [5] OF ARRAY [5] OF BOOLEAN;
BEGIN
    RETURN 0; 
END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY [5] OF INTEGER;\ny: ARRAY [5] OF ARRAY "
         "[5] OF BOOLEAN;\nBEGIN\nRETURN 0;\nEND alpha.",
         ""},

        {R"(MODULE alpha;
VAR x : ARRAY [5] OF INTEGER;
VAR y : ARRAY [5] OF ARRAY [5] OF INTEGER;

PROCEDURE sum(a : ARRAY [5] OF BOOLEAN) : INTEGER;
BEGIN
    RETURN 0;
END sum;

PROCEDURE add(a : ARRAY [5] OF INTEGER; a : ARRAY [5] OF INTEGER) : ARRAY [5] OF INTEGER;
BEGIN
    RETURN 0;
END add;

BEGIN
    RETURN 0; 
END alpha.)",
         "MODULE alpha;\nVAR\nx: ARRAY [5] OF INTEGER;\ny: ARRAY [5] OF ARRAY "
         "[5] OF INTEGER;\nPROCEDURE sum(a : ARRAY [5] OF BOOLEAN): "
         "INTEGER;\nBEGIN\nRETURN 0;\nEND sum.\nPROCEDURE add(a : ARRAY [5] OF "
         "INTEGER; a : ARRAY [5] OF INTEGER): ARRAY [5] OF "
         "INTEGER;\nBEGIN\nRETURN 0;\nEND add.\nBEGIN\nRETURN 0;\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
VAR x : ARRAY 5] OF INTEGER;
BEGIN
    RETURN 0; 
END alpha.)",
         "", "2,15: Unexpected token: integer(5) - expecting ["},

        {R"(MODULE alpha;
VAR x : ARRAY [] OF INTEGER;
BEGIN
    RETURN 0; 
END alpha.)",
         "", "2,16: Unexpected token: ] - expecting integer"},

        {R"(MODULE alpha;
VAR x : ARRAY [5 OF INTEGER;
BEGIN
    RETURN 0; 
END alpha.)",
         "", "2,19: Unexpected token: OF - expecting ]"},

        {R"(MODULE alpha;
VAR x : ARRAY [5]  INTEGER;
BEGIN
    RETURN 0; 
END alpha.)",
         "", "2,26: Unexpected token: INTEGER - expecting OF"},

        {R"(MODULE alpha;
VAR x : ARRAY [5] OF ;
BEGIN
    RETURN 0; 
END alpha.)",
         "", "2,22: Unexpected token: semicolon - expecting indent"},

    };
    do_parse_tests(tests);
}
