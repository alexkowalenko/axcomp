//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "gtest/gtest.h"

#include "parse_test.hh"

TEST(Definitions, DefPrinter) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
                CONST a* = 10; 
                      c = 30;
                TYPE second* = INTEGER;
                     hour = INTEGER;
                VAR x* : ARRAY 5 OF INTEGER;
                    z : ARRAY 5 OF INTEGER;
                VAR pt : RECORD x*, y* : INTEGER END;

                PROCEDURE f* : INTEGER;
                BEGIN
                    RETURN 0
                END f;

                PROCEDURE g : INTEGER;
                BEGIN
                    RETURN 0
                END g;

                BEGIN
                    RETURN 0 
                END alpha.)",
         "DEFINITION alpha;\nCONST\na* = 10;\nTYPE\nsecond* = "
         "INTEGER;\nVAR\nx*: ARRAY 5 OF INTEGER;\nPROCEDURE f*(): "
         "INTEGER;\nEND alpha.",
         ""},

        {R"(MODULE alpha;
                VAR x- : ARRAY 5 OF INTEGER;
                VAR pt : RECORD x-, y- : INTEGER END;

                BEGIN
                    RETURN 0 
                END alpha.)",
         "DEFINITION alpha;\nVAR\nx-: ARRAY 5 OF INTEGER;\nEND alpha.", ""},

        {R"(MODULE alpha;
               
                VAR pt : RECORD x-, y- : INTEGER END;

                BEGIN
                    RETURN 0 
                END alpha.)",
         "DEFINITION alpha;\nEND alpha.", ""},

    };
    do_def_tests(tests);
}

TEST(Definitions, DefParser) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
                CONST a* = 10 + 3; 
                      c = 30;
                TYPE second* = INTEGER;
                     hour = INTEGER;
                VAR x* : ARRAY 5 OF INTEGER;
                    z : ARRAY 5 OF INTEGER;
                VAR pt : RECORD x*, y* : INTEGER END;

                PROCEDURE f* : INTEGER;
                BEGIN
                    RETURN 0
                END f;

                PROCEDURE g : INTEGER;
                BEGIN
                    RETURN 0
                END g;

                BEGIN
                    RETURN 0 
                END alpha.)",
         "DEFINITION alpha;\nCONST\na* = 10+3;\nTYPE\nsecond* = "
         "INTEGER;\nVAR\nx*: ARRAY 5 OF INTEGER;\nPROCEDURE f*(): "
         "INTEGER;\nEND alpha.",
         ""},

        {R"(MODULE alpha;
                VAR x- : ARRAY 5 OF INTEGER;
                VAR pt : RECORD x-, y- : INTEGER END;

                BEGIN
                    RETURN 0 
                END alpha.)",
         "DEFINITION alpha;\nVAR\nx-: ARRAY 5 OF INTEGER;\nEND alpha.", ""},

        {R"(MODULE alpha;
               
                VAR pt : RECORD x-, y- : INTEGER END;

                BEGIN
                    RETURN 0 
                END alpha.)",
         "DEFINITION alpha;\nEND alpha.", ""},

    };
    do_defparse_tests(tests);
}
