//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "gtest/gtest.h"

#include "parse_test.hh"

TEST(Parser, Proc) {
    std::vector<ParseTests> tests = {

        {R"(MODULE x;
        PROCEDURE f;
        BEGIN
            RETURN 12
        END f;
        BEGIN
            RETURN 0
        END x.)",
         "MODULE x;\nPROCEDURE f;\nBEGIN\nRETURN 12\nEND f;\nBEGIN\nRETURN "
         "0\nEND x.",
         ""},

        {R"(MODULE x;
        PROCEDURE f;
        BEGIN
            RETURN 12
        END f;

        PROCEDURE g;
        BEGIN
            RETURN 24
        END g;
        BEGIN
            RETURN 0
        END x.)",
         "MODULE x;\nPROCEDURE f;\nBEGIN\nRETURN 12\nEND f;\nPROCEDURE "
         "g;\nBEGIN\nRETURN 24\nEND g;\nBEGIN\nRETURN 0\nEND x.",
         ""},

        {R"(MODULE x;
        PROCEDURE f;
        END f;

        BEGIN
            RETURN 0
        END x.)",
         "MODULE x;\nPROCEDURE f;\nEND f;\nBEGIN\nRETURN 0\nEND x.", ""},

        // Errors
        {R"(MODULE x;
        PROCEDURE f;
            RETURN 12
        END f;
        BEGIN
            RETURN 0
        END x.
        )",
         "", "3,18: Unexpected token: RETURN - expecting END"},

        {R"(MODULE x;
        PROCEDURE f;
        BEGIN
            RETURN 12
        END ;
        BEGIN
            RETURN 0
        END x.)",
         "", "5,13: Unexpected token: semicolon - expecting indent"},

        {R"(MODULE x;
            f;
            BEGIN
                RETURN 12
            END f;
            BEGIN
                RETURN 0
            END x.)",
         "", "2,13: Unexpected token: f - expecting END"},

        {R"(MODULE x;
            PROCEDURE f;
            BEGIN
                RETURN 12
            f;
            BEGIN
                RETURN 0
            END x.)",
         "", "8,19: Unexpected token: EOF - expecting indent"},

    };
    do_parse_tests(tests);
}

TEST(Parser, Call) {
    std::vector<ParseTests> tests = {

        {R"(MODULE x;
            PROCEDURE f;
            BEGIN
                RETURN 12
            END f;
            BEGIN
                f()
            END x.)",
         "MODULE x;\nPROCEDURE f;\nBEGIN\nRETURN 12\nEND f;\nBEGIN\nf()\nEND "
         "x.",
         ""},

        {R"(MODULE x;
        PROCEDURE f;
        BEGIN
            RETURN 12
        END f;

        PROCEDURE g;
        BEGIN
            f();
            RETURN 24
        END g;
        BEGIN
            g();
            RETURN 0
        END x.)",
         "MODULE x;\nPROCEDURE f;\nBEGIN\nRETURN 12\nEND f;\nPROCEDURE "
         "g;\nBEGIN\nf();\nRETURN 24\nEND g;\nBEGIN\ng();\nRETURN 0\nEND x.",
         ""},

        {R"(MODULE x;
        PROCEDURE f;
        BEGIN
            RETURN 12
        END f;

        PROCEDURE g;
        BEGIN
            f;
            RETURN 24
        END g;
        BEGIN
            g;
            RETURN 0
        END x.)",
         "MODULE x;\nPROCEDURE f;\nBEGIN\nRETURN 12\nEND f;\nPROCEDURE "
         "g;\nBEGIN\nf();\nRETURN 24\nEND g;\nBEGIN\ng();\nRETURN 0\nEND x.",
         ""},

        // Error
        {R"(MODULE x;
            BEGIN
                f(;
            END x.)",
         "", "3,19: Unexpected token: semicolon"},
        {R"(MODULE x;
            BEGIN
                f);
            END x.)",
         "", "3,18: Unexpected token: ) - expecting :="}};
    do_parse_tests(tests);
}

TEST(Parser, ReturnType) {
    std::vector<ParseTests> tests = {

        {R"(MODULE x;
            PROCEDURE f(): INTEGER;
            BEGIN RETURN 12
            END f;
            BEGIN
            f()
            END x.)",
         "MODULE x;\nPROCEDURE f(): INTEGER;\nBEGIN\nRETURN 12\nEND "
         "f;\nBEGIN\nf()\nEND x.",
         ""},

        {R"(MODULE x;
        PROCEDURE f(): INTEGER;
        BEGIN RETURN 12
        END f;
        PROCEDURE g;
        BEGIN RETURN 24
        END g;
        BEGIN
            g();
        RETURN 0
        END x.)",
         "MODULE x;\nPROCEDURE f(): INTEGER;\nBEGIN\nRETURN 12\nEND "
         "f;\nPROCEDURE g;\nBEGIN\nRETURN 24\nEND g;\nBEGIN\ng();\nRETURN "
         "0\nEND x.",
         ""},

        // Error
        {R"(MODULE x;
        PROCEDURE f() INTEGER;
        BEGIN
            RETURN 12
        END f;
        BEGIN
            f()
        END x.)",
         "", "2,29: Unexpected token: INTEGER - expecting semicolon"},
        {R"(MODULE x;
        PROCEDURE f() :
        BEGIN
            RETURN 12
        END f;
        BEGIN
            f()
        END x.)",
         "", "3,13: Unexpected token: BEGIN - expecting indent"},
    };
    do_parse_tests(tests);
}

TEST(Parser, FunctionCall) {
    std::vector<ParseTests> tests = {

        {R"(MODULE x;
            PROCEDURE f(): INTEGER;
            BEGIN RETURN 12
            END f;
            BEGIN
            RETURN f() 
                + (f() * f())
            END x.)",
         "MODULE x;\nPROCEDURE f(): INTEGER;\nBEGIN\nRETURN 12\nEND "
         "f;\nBEGIN\nRETURN f()+ (f()*f()) \nEND x.",
         ""},

        // Error
        {R"(MODULE x;
            PROCEDURE f(): INTEGER;
            BEGIN RETURN 12
            END f;
            BEGIN
            RETURN f(
            END x.)",
         "", "7,15: Unexpected token: END"},
    };
    do_parse_tests(tests);
}

TEST(Parser, FunctionParams) {
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

        {R"(MODULE xxx;
            VAR z : INTEGER;
            PROCEDURE f(x, y: INTEGER) : INTEGER;
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

        {R"(MODULE xxx;
            VAR z : INTEGER;
            PROCEDURE f(x, y: INTEGER; bx, by : BOOLEAN) : INTEGER;
            VAR zz : INTEGER;
            BEGIN
            RETURN zz
            END f;
            BEGIN
            RETURN 3
            END xxx.)",
         "MODULE xxx;\nVAR\nz: INTEGER;\nPROCEDURE f(x : INTEGER; y : INTEGER; "
         "bx : BOOLEAN; by : BOOLEAN): INTEGER;\nVAR\nzz: "
         "INTEGER;\nBEGIN\nRETURN zz\nEND f;\nBEGIN\nRETURN 3\nEND xxx.",
         ""},

        // Errors
        {R"(MODULE xxx;
            VAR z : INTEGER;
            PROCEDURE f(x  INTEGER) : INTEGER;
            VAR zz : INTEGER;
            BEGIN
            RETURN zz
            END f;
            BEGIN
            RETURN 3
            END xxx.)",
         "", "3,34: Unexpected token: INTEGER - expecting :"},
        {R"(MODULE xxx;
            VAR z : INTEGER;
            PROCEDURE f(x : ) : INTEGER;
            VAR zz : INTEGER;
            BEGIN
            RETURN zz
            END f;
            BEGIN
            RETURN 3
            END xxx.)",
         "", "3,29: Unexpected token: ) - expecting indent"},
        {R"(MODULE xxx;
            VAR z : INTEGER;
            PROCEDURE f(x : INTEGER  y: INTEGER) : INTEGER;
            VAR zz : INTEGER;
            BEGIN
            RETURN zz
            END f;
            BEGIN
            RETURN 3
            END xxx.)",
         "", "3,38: expecting ; or ) in parameter list"},
    };
    do_parse_tests(tests);
}

TEST(Parser, CallArgs) {
    std::vector<ParseTests> tests = {

        {R"(MODULE xxx;
            VAR z : INTEGER;
            PROCEDURE f(x : INTEGER) : INTEGER;
            VAR zz : INTEGER;
            BEGIN
            RETURN zz
            END f;
            BEGIN
            RETURN f(3)
            END xxx.)",
         "MODULE xxx;\nVAR\nz: INTEGER;\nPROCEDURE f(x : INTEGER): "
         "INTEGER;\nVAR\nzz: INTEGER;\nBEGIN\nRETURN zz\nEND "
         "f;\nBEGIN\nRETURN f(3)\nEND xxx.",
         ""},

        {R"(MODULE xxx;
            VAR z : INTEGER;
            PROCEDURE f(x : INTEGER; y: INTEGER) : INTEGER;
            VAR zz : INTEGER;
            BEGIN
            RETURN zz
            END f;
            BEGIN
            RETURN f(3 , 4) + f(2, f(3 + 4))
            END xxx.)",
         "MODULE xxx;\nVAR\nz: INTEGER;\nPROCEDURE f(x : INTEGER; y : "
         "INTEGER): INTEGER;\nVAR\nzz: INTEGER;\nBEGIN\nRETURN zz\nEND "
         "f;\nBEGIN\nRETURN f(3, 4)+f(2, f(3+4))\nEND xxx.",
         ""},

        // Errors
        {R"(MODULE xxx;
            VAR z : INTEGER;
            BEGIN
            RETURN f(3, )
            END xxx.)",
         "", "4,25: Unexpected token: )"},
        {R"(MODULE xxx;
            VAR z : INTEGER;
            BEGIN
            RETURN f(3 
            END xxx.)",
         "", "5,15: Unexpected END expecting , or )"},
    };
    do_parse_tests(tests);
}

TEST(Parser, VarArgs) {
    std::vector<ParseTests> tests = {

        {R"(MODULE xxx;
            VAR z : INTEGER;
            PROCEDURE f(VAR x : INTEGER) : INTEGER;
            BEGIN
            RETURN x
            END f;
            BEGIN
            RETURN f(3)
            END xxx.)",
         "MODULE xxx;\nVAR\nz: INTEGER;\nPROCEDURE f(VAR x : INTEGER): "
         "INTEGER;\nBEGIN\nRETURN x\nEND f;\nBEGIN\nRETURN f(3)\nEND xxx.",
         ""},

        {R"(MODULE xxx;
            VAR z : INTEGER;
            PROCEDURE f(VAR x ,y: INTEGER) : INTEGER;
            BEGIN
            RETURN x
            END f;
            BEGIN
            RETURN 0
            END xxx.)",
         "MODULE xxx;\nVAR\nz: INTEGER;\nPROCEDURE f(VAR x : INTEGER; VAR y : "
         "INTEGER): INTEGER;\nBEGIN\nRETURN x\nEND f;\nBEGIN\nRETURN 0\nEND "
         "xxx.",
         ""},

        {R"(MODULE xxx;
            VAR z : INTEGER;
            PROCEDURE f(VAR x ,y: INTEGER; z : BOOLEAN) : INTEGER;
            BEGIN
            RETURN x
            END f;
            BEGIN
            RETURN 0
            END xxx.)",
         "MODULE xxx;\nVAR\nz: INTEGER;\nPROCEDURE f(VAR x : INTEGER; VAR y : "
         "INTEGER; z : BOOLEAN): INTEGER;\nBEGIN\nRETURN x\nEND "
         "f;\nBEGIN\nRETURN 0\nEND xxx.",
         ""},

        // Errors

        {R"(MODULE xxx;
            VAR z : INTEGER;
            PROCEDURE f(VAR VAR x: INTEGER) : INTEGER;
            BEGIN
            RETURN x
            END f;
            BEGIN
            RETURN 0
            END xxx.)",
         "", "3,31: Unexpected token: VAR - expecting indent"},
    };
    do_parse_tests(tests);
}

TEST(Parser, NestedProcs) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha; (* Nested Procedures *)
                VAR x: SET;

                PROCEDURE f(): INTEGER;
                    CONST x = 1;
                    
                    PROCEDURE g(): INTEGER;
                    CONST x = 2;
                    BEGIN 
                    END g;
                BEGIN
                    g();
                END f;

                BEGIN
                    f();
                END alpha.)",
         "MODULE alpha;\nVAR\nx: SET;\nPROCEDURE f(): INTEGER;\nCONST\nx = 1;\nPROCEDURE g(): "
         "INTEGER;\nCONST\nx = 2;\nEND g;\nBEGIN\ng()\nEND f;\nBEGIN\nf()\nEND alpha.",
         ""},

        // Errors

        {R"(MODULE alpha; (* Nested Procedures *)
                VAR x: SET;

                PROCEDURE f(): INTEGER;
                    CONST x = 1;
                    
                    PROCEDURE g(): INTEGER;
                    CONST x = 2;
                    BEGIN 
                    END;
                BEGIN
                    g();
                END f;

                BEGIN
                    f();
                END alpha.)",
         "", "10,24: Unexpected token: semicolon - expecting indent"},

    };
    do_parse_tests(tests);
}
