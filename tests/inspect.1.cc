//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "gtest/gtest.h"

#include "parse_test.hh"

void do_inspect_tests(std::vector<ParseTests> &tests);

TEST(Inspector, VarType) {
    std::vector<ParseTests> tests = {
        {"MODULE x; VAR z: complex; BEGIN x := 10; END x.", "",
         "1,15: Unknown type: complex for identifier z"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, UnknownExpr) {
    std::vector<ParseTests> tests = {
        {"MODULE y; VAR x : INTEGER; BEGIN RETURN x; END y.",
         "MODULE y;\nVAR\nx: INTEGER;\nBEGIN\nRETURN x;\nEND y.", ""},

        // Errors
        {"MODULE y; VAR x : INTEGER; BEGIN RETURN z; END y.", "",
         "1,42: undefined identifier z"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, Return) {
    std::vector<ParseTests> tests = {
        {"MODULE x; VAR z: INTEGER; BEGIN z := 10; RETURN z; END x.",
         "MODULE x;\nVAR\nz: INTEGER;\nBEGIN\nz := 10;\nRETURN z;\nEND x.", ""},

        // Errors
        {"MODULE x; VAR z: INTEGER; BEGIN z := 10; END x.", "",
         "1,0: MODULE x has no RETURN function"},
        {"MODULE x; VAR z: INTEGER; PROCEDURE y; BEGIN z := 1; END y; "
         "BEGIN x := 10; END x.",
         "", "1,35: PROCEDURE y has no RETURN function"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, ReturnType) {
    std::vector<ParseTests> tests = {
        {"MODULE x; PROCEDURE f(): INTEGER; BEGIN RETURN 0; END f; BEGIN "
         "RETURN 333; END x.",
         "MODULE x;\nPROCEDURE f(): INTEGER;\nBEGIN\nRETURN 0;\nEND "
         "f.\nBEGIN\nRETURN 333;\nEND x.",
         ""},
        {"MODULE x; PROCEDURE f; BEGIN RETURN; END f; BEGIN "
         "RETURN 333; END x.",
         "MODULE x;\nPROCEDURE f;\nBEGIN\nRETURN ;\nEND f.\nBEGIN\nRETURN "
         "333;\nEND x.",
         ""},

        {R"(MODULE xxx;
            PROCEDURE f : BOOLEAN;
            BEGIN
            RETURN TRUE;
            END f;
            BEGIN
            RETURN 3;
            END xxx.)",
         "MODULE xxx;\nPROCEDURE f(): BOOLEAN;\nBEGIN\nRETURN TRUE;\nEND "
         "f.\nBEGIN\nRETURN 3;\nEND xxx.",
         ""},

        // Error
        {"MODULE x; PROCEDURE f(): complex; BEGIN RETURN 0; END f; BEGIN "
         "RETURN 333; END x.",
         "", "1,19: Unknown type: complex for return from function f"},

        {"MODULE x; PROCEDURE f(): INTEGER; BEGIN RETURN; END f; BEGIN "
         "RETURN 333; END x.",
         "", "1,46: RETURN does not match return type for function f"},

        {"MODULE x; PROCEDURE f; BEGIN RETURN 0; END f; BEGIN "
         "RETURN 333; END x.",
         "", "1,35: RETURN does not match return type for function f"},
        {R"(MODULE xxx;
            PROCEDURE f : BOOLEAN;
            BEGIN
            RETURN 123456;
            END f;
            BEGIN
            RETURN 3;
            END xxx.)",
         "", "4,18: RETURN does not match return type for function f"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, Call) {
    std::vector<ParseTests> tests = {
        {"MODULE y; VAR x : INTEGER; PROCEDURE f; BEGIN RETURN; END f; BEGIN "
         "f(); RETURN x; END y.",
         "MODULE y;\nVAR\nx: INTEGER;\nPROCEDURE f;\nBEGIN\nRETURN ;\nEND "
         "f.\nBEGIN\nf();\nRETURN x;\nEND y.",
         ""},
        {"MODULE y; VAR x : INTEGER; "
         "PROCEDURE f():INTEGER; BEGIN RETURN 0; END f; "
         "BEGIN f(); RETURN f(); END y.",
         "MODULE y;\nVAR\nx: INTEGER;\nPROCEDURE f(): INTEGER;\nBEGIN\nRETURN "
         "0;\nEND f.\nBEGIN\nf();\nRETURN f();\nEND y.",
         ""},
        {R"(MODULE xxx;
            PROCEDURE f(x : INTEGER) : INTEGER;
            BEGIN
            RETURN 0;
            END f;
            BEGIN
                RETURN f(1);
            END xxx.)",
         "MODULE xxx;\nPROCEDURE f(x : INTEGER): INTEGER;\nBEGIN\nRETURN "
         "0;\nEND f.\nBEGIN\nRETURN f(1);\nEND xxx.",
         ""},

        // Errors
        {"MODULE y; VAR x : INTEGER; PROCEDURE f; BEGIN RETURN; END f; BEGIN "
         "x(); RETURN x; END y.",
         "", "1,69: x is not a PROCEDURE"},
        {"MODULE y; VAR x : INTEGER; PROCEDURE f; BEGIN RETURN; END f; BEGIN "
         "g(); RETURN x; END y.",
         "", "1,69: undefined PROCEDURE g"},
        {"MODULE y; VAR x : INTEGER; "
         "PROCEDURE f():INTEGER; BEGIN RETURN 0; END f; "
         "BEGIN RETURN g(); END y.",
         "", "1,88: undefined PROCEDURE g"},

        {R"(MODULE xxx;
            PROCEDURE f(x : INTEGER) : INTEGER;
            BEGIN
            RETURN 0;
            END f;
            BEGIN
                RETURN f();
            END xxx.)",
         "",
         "7,25: calling PROCEDURE f, incorrect number of arguments: 0 instead "
         "of "
         "1"},
        {R"(MODULE xxx;
            PROCEDURE f() : INTEGER;
            BEGIN
            RETURN 0;
            END f;
            BEGIN
                RETURN f(1,2,3,4);
            END xxx.)",
         "",
         "7,25: calling PROCEDURE f, incorrect number of arguments: 4 instead "
         "of "
         "0"},

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
            RETURN zz;
            END f;
            BEGIN
            RETURN 3;
            END xxx.)",
         "MODULE xxx;\nVAR\nz: INTEGER;\nPROCEDURE f(x : INTEGER): "
         "INTEGER;\nVAR\nzz: INTEGER;\nBEGIN\nRETURN zz;\nEND "
         "f.\nBEGIN\nRETURN 3;\nEND xxx.",
         ""},

        {R"(MODULE xxx;
            VAR z : INTEGER;
            PROCEDURE f(x : INTEGER; y: INTEGER) : INTEGER;
            VAR zz : INTEGER;
            BEGIN
            RETURN zz;
            END f;
            BEGIN
            RETURN 3;
            END xxx.)",
         "MODULE xxx;\nVAR\nz: INTEGER;\nPROCEDURE f(x : INTEGER; y : "
         "INTEGER): INTEGER;\nVAR\nzz: INTEGER;\nBEGIN\nRETURN zz;\nEND "
         "f.\nBEGIN\nRETURN 3;\nEND xxx.",
         ""},

        // Errors
        {R"(MODULE xxx;
            VAR z : INTEGER;
            PROCEDURE f(x : UNDEF) : INTEGER;
            VAR zz : INTEGER;
            BEGIN
            RETURN zz;
            END f;
            BEGIN
            RETURN 3;
            END xxx.)",
         "", "3,21: Unknown type: UNDEF for paramater x from function f"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, Assignment) {
    std::vector<ParseTests> tests = {

        {R"(MODULE xxx;
            VAR z : INTEGER;
            BEGIN
            z := 33;
            RETURN z;
            END xxx.)",
         "MODULE xxx;\nVAR\nz: INTEGER;\nBEGIN\nz := 33;\nRETURN z;\nEND xxx.",
         ""},

        {R"(MODULE xxx;
            VAR z : BOOLEAN;
            BEGIN
            z := TRUE;
            RETURN z;
            END xxx.)",
         "MODULE xxx;\nVAR\nz: BOOLEAN;\nBEGIN\nz := TRUE;\nRETURN z;\nEND "
         "xxx.",
         ""},

        // Errors
        {R"(MODULE xxx;
            VAR z : BOOLEAN;
            BEGIN
            z := 4;
            RETURN z;
            END xxx.)",
         "", "4,16: Can't assign expression of type INTEGER to z"},
        {R"(MODULE xxx;
            VAR z : INTEGER;
            BEGIN
            z := TRUE;
            RETURN z;
            END xxx.)",
         "", "4,16: Can't assign expression of type BOOLEAN to z"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, ExprCompare) {
    std::vector<ParseTests> tests = {

        {R"(MODULE xxx;
            BEGIN
            RETURN TRUE = FALSE;
            END xxx.)",
         "MODULE xxx;\nBEGIN\nRETURN TRUE = FALSE;\nEND xxx.", ""},

        // Errors
        {R"(MODULE xxx;
            BEGIN
            RETURN TRUE = 1;
            END xxx.)",
         "", "3,23: types in expression don't match BOOLEAN and INTEGER"},
        {R"(MODULE xxx;
            BEGIN
            RETURN 0 # FALSE;
            END xxx.)",
         "", "3,20: types in expression don't match INTEGER and BOOLEAN"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, SimpleExpr) {
    std::vector<ParseTests> tests = {

        {R"(MODULE xxx;
            BEGIN
            RETURN TRUE OR FALSE;
            END xxx.)",
         "MODULE xxx;\nBEGIN\nRETURN TRUE OR FALSE;\nEND xxx.", ""},

        // Errors
        {R"(MODULE xxx;
            BEGIN
            RETURN TRUE + 1;
            END xxx.)",
         "", "3,23: types in expression don't match BOOLEAN and INTEGER"},
        {R"(MODULE xxx;
            BEGIN
            RETURN 0 OR FALSE;
            END xxx.)",
         "", "3,20: types in expression don't match INTEGER and BOOLEAN"},
        {R"(MODULE xxx;
            BEGIN
            RETURN 0 OR 0;
            END xxx.)",
         "", "3,20: types in OR expression must be BOOLEAN"},
        {R"(MODULE xxx;
            BEGIN
            RETURN FALSE + FALSE;
            END xxx.)",
         "", "3,24: types in + expression must be numeric"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, Term) {
    std::vector<ParseTests> tests = {

        {R"(MODULE xxx;
            BEGIN
            RETURN TRUE & FALSE;
            END xxx.)",
         "MODULE xxx;\nBEGIN\nRETURN TRUE & FALSE;\nEND xxx.", ""},

        // Errors
        {R"(MODULE xxx;
            BEGIN
            RETURN TRUE * 1;
            END xxx.)",
         "", "3,23: types in expression don't match BOOLEAN and INTEGER"},
        {R"(MODULE xxx;
            BEGIN
            RETURN 0 & FALSE;
            END xxx.)",
         "", "3,20: types in expression don't match INTEGER and BOOLEAN"},
        {R"(MODULE xxx;
            BEGIN
            RETURN TRUE DIV FALSE;
            END xxx.)",
         "", "3,23: types in DIV expression must be numeric"},
        {R"(MODULE xxx;
            BEGIN
            RETURN 0 & 23;
            END xxx.)",
         "", "3,20: types in & expression must be BOOLEAN"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, Factor) {
    std::vector<ParseTests> tests = {

        {R"(MODULE xxx;
            BEGIN
            RETURN ~ TRUE;
            END xxx.)",
         "MODULE xxx;\nBEGIN\nRETURN ~ TRUE;\nEND xxx.", ""},

        // Errors
        {R"(MODULE xxx;
            BEGIN
            RETURN ~ 1;
            END xxx.)",
         "", "3,20: type in ~ expression must be BOOLEAN"},
    };
    do_inspect_tests(tests);
}
