//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "gtest/gtest.h"

#include "parse_test.hh"

TEST(Parser, Comments) {
    std::vector<ParseTests> tests = {

        {"(*hello*) MODULE y; BEGIN RETURN 12 END y.", "MODULE y;\nBEGIN\nRETURN 12\nEND y.", ""},

        {"(*hello*) MODULE (* hello *) y; (* hello *)BEGIN (* hello *) RETURN "
         "12 (* "
         "hello *)END y.(* hello *)",
         "MODULE y;\nBEGIN\nRETURN 12\nEND y.", ""},

        {"(*hello*) MODULE (* hello *) y; (* hello *)BEGIN RETURN  (* hello "
         "12; 24; *) "
         "36 (*hello *) END y.(* hello *)",
         "MODULE y;\nBEGIN\nRETURN 36\nEND y.", ""},

        {"(*hello*) MODULE (* hello *) y; (* hello *)BEGIN RETURN  (* hello (* "
         "12; 24; "
         "*) *) "
         "36 (*hello *) END y.(* hello *)",
         "MODULE y;\nBEGIN\nRETURN 36\nEND y.", ""},

    };
    do_parse_tests(tests);
}

TEST(Parser, Module) {
    std::vector<ParseTests> tests = {

        {"MODULE y; BEGIN RETURN 12 END y.", "MODULE y;\nBEGIN\nRETURN 12\nEND y.", ""},
        {"MODULE y; BEGIN RETURN 12; RETURN 24 END y.",
         "MODULE y;\nBEGIN\nRETURN 12;\nRETURN 24\nEND y.", ""},
        {"MODULE y; BEGIN RETURN END y.", "MODULE y;\nBEGIN\nRETURN \nEND y.", ""},
        {"MODULE y; BEGIN END y.", "MODULE y;\nEND y.", ""},
        {"MODULE y; END y.", "MODULE y;\nEND y.", ""},

        // Errors
        {"y; BEGIN RETURN 12 END y.", "", "1,1: Unexpected token: y - expecting MODULE"},
        {"MODULE ; BEGIN RETURN 12 END y.", "",
         "1,8: Unexpected token: semicolon - expecting indent"},
        {"MODULE y BEGIN RETURN 12 END y.", "",
         "1,14: Unexpected token: BEGIN - expecting semicolon"},
        {"MODULE y; RETURN 12 END y.", "", "1,16: Unexpected token: RETURN - expecting END"},

        {"MODULE y; BEGIN RETURN 12 y.", "", "1,29: Unexpected token: EOF - expecting indent"},
        {"MODULE y; BEGIN RETURN 12 END .", "",
         "1,31: Unexpected token: period - expecting indent"},
        {"MODULE y; BEGIN RETURN 12 END y", "", "1,32: Unexpected token: EOF - expecting period"},

        {"MODULE x; BEGIN RETURN 12 END y.", "",
         "1,31: END identifier name: y doesn't match module name: x"},
    };
    do_parse_tests(tests);
}

TEST(Parser, Logic) {
    std::vector<ParseTests> tests = {

        {"MODULE y; BEGIN RETURN 1 = 1 END y.", "MODULE y;\nBEGIN\nRETURN 1 = 1\nEND y.", ""},
        {"MODULE y; BEGIN RETURN 1 # 1 END y.", "MODULE y;\nBEGIN\nRETURN 1 # 1\nEND y.", ""},
        {"MODULE y; BEGIN RETURN (1 < 1) = (1 > 1) END y.",
         "MODULE y;\nBEGIN\nRETURN  (1 < 1)  =  (1 > 1) \nEND y.", ""},
        {"MODULE y; BEGIN RETURN (1 >= 1) # (1 <= 1) END y.",
         "MODULE y;\nBEGIN\nRETURN  (1 >= 1)  #  (1 <= 1) \nEND y.", ""},

        {"MODULE y; BEGIN RETURN 1 OR 1 END y.", "MODULE y;\nBEGIN\nRETURN 1 OR 1\nEND y.", ""},
        {"MODULE y; BEGIN RETURN 1 & 1 END y.", "MODULE y;\nBEGIN\nRETURN 1 & 1\nEND y.", ""},
        {"MODULE y; BEGIN RETURN ~ TRUE END y.", "MODULE y;\nBEGIN\nRETURN ~ TRUE\nEND y.", ""},
        {"MODULE y; BEGIN RETURN ~ ~ TRUE END y.", "MODULE y;\nBEGIN\nRETURN ~ ~ TRUE\nEND y.",
         ""},
        {"MODULE y; BEGIN RETURN ~ TRUE & ~ FALSE END y.",
         "MODULE y;\nBEGIN\nRETURN ~ TRUE & ~ FALSE\nEND y.", ""},

    };
    do_parse_tests(tests);
}

TEST(Parser, Plus) {
    std::vector<ParseTests> tests = {

        {"MODULE y; BEGIN RETURN - 1 END y.", "MODULE y;\nBEGIN\nRETURN -1\nEND y.", ""},
        {"MODULE y; BEGIN RETURN + 1 END y.", "MODULE y;\nBEGIN\nRETURN +1\nEND y.", ""},

        {"MODULE y; BEGIN RETURN 1 + 1 END y.", "MODULE y;\nBEGIN\nRETURN 1+1\nEND y.", ""},
        {"MODULE y; BEGIN RETURN 2 - 2 END y.", "MODULE y;\nBEGIN\nRETURN 2-2\nEND y.", ""},
        {"MODULE y; BEGIN RETURN 1 + 2 - 3 END y.", "MODULE y;\nBEGIN\nRETURN 1+2-3\nEND y.", ""},
        {"MODULE y; BEGIN RETURN 1 + 2 - 3 + 4 END y.", "MODULE y;\nBEGIN\nRETURN 1+2-3+4\nEND y.",
         ""},

        {"MODULE y; BEGIN RETURN - 1 + 2 END y.", "MODULE y;\nBEGIN\nRETURN -1+2\nEND y.", ""},

        // Errors
        {"MODULE y; BEGIN RETURN -  END y.", "", "1,29: Unexpected token: END"},
        {"MODULE y; BEGIN RETURN 1 - END y.", "", "1,30: Unexpected token: END"},
        {"MODULE y; BEGIN RETURN - 1 + 2 +  END y.", "", "1,37: Unexpected token: END"},
    };
    do_parse_tests(tests);
}

TEST(Parser, Mult) {
    std::vector<ParseTests> tests = {

        {"MODULE y; BEGIN RETURN 2 * 2 END y.", "MODULE y;\nBEGIN\nRETURN 2*2\nEND y.", ""},
        {"MODULE y; BEGIN RETURN 4 DIV 2 END y.", "MODULE y;\nBEGIN\nRETURN 4 DIV 2\nEND y.", ""},
        {"MODULE y; BEGIN RETURN 7 MOD 2 END y.", "MODULE y;\nBEGIN\nRETURN 7 MOD 2\nEND y.", ""},

        {"MODULE y; BEGIN RETURN 1 * 2 * 3 * 4 END y.", "MODULE y;\nBEGIN\nRETURN 1*2*3*4\nEND y.",
         ""},

        {"MODULE y; BEGIN RETURN 3 * 3 + 4 * 4 END y.", "MODULE y;\nBEGIN\nRETURN 3*3+4*4\nEND y.",
         ""},

        // Errors
        {"MODULE y; BEGIN RETURN *  END y.", "", "1,24: Unexpected token: *"},
        {"MODULE y; BEGIN RETURN 1 MOD END y.", "", "1,32: Unexpected token: END"},
        {"MODULE y; BEGIN RETURN - 1 + 2 DIV  END y.", "", "1,39: Unexpected token: END"},
    };
    do_parse_tests(tests);
}

TEST(Parser, Parentheses) {
    std::vector<ParseTests> tests = {

        {"MODULE y; BEGIN RETURN (2) END y.", "MODULE y;\nBEGIN\nRETURN  (2) \nEND y.", ""},
        {"MODULE y; BEGIN RETURN (2 + 1) END y.", "MODULE y;\nBEGIN\nRETURN  (2+1) \nEND y.", ""},
        {"MODULE y; BEGIN RETURN (2 + (4 * 3)) END y.",
         "MODULE y;\nBEGIN\nRETURN  (2+ (4*3) ) \nEND y.", ""},
        {"MODULE y; BEGIN RETURN (2 + (4 * (3 DIV 1))) END y.",
         "MODULE y;\nBEGIN\nRETURN  (2+ (4* (3 DIV 1) ) ) \nEND y.", ""},

        // Errors
        {"MODULE y; BEGIN RETURN (2 ; END y.", "",
         "1,27: Unexpected token: semicolon - expecting )"},
        {"MODULE y; BEGIN RETURN (2 + 4) * (3 DIV 1)) ; END y.", "", "1,43: Unexpected token: )"},
    };
    do_parse_tests(tests);
}

TEST(Parser, HexDigits) {
    std::vector<ParseTests> tests = {
        {R"(MODULE x;
            BEGIN
                RETURN 0dH
            END x.)",
         "MODULE x;\nBEGIN\nRETURN 0dH\nEND x.", ""},

        {R"(MODULE x;
            BEGIN
                RETURN 0cafeH + 0babeH * 0deadH;
            END x.)",
         "MODULE x;\nBEGIN\nRETURN 0cafeH+0babeH*0deadH\nEND x.", ""},
    };
    do_parse_tests(tests);
}

TEST(Parser, REAL) {
    std::vector<ParseTests> tests = {
        {R"(MODULE alpha; (* REAL *)
            CONST c = 1.2;
                d = 1.2E+2;
                f = 2.3D+2;
                h = 0.23D-8;
            BEGIN
                RETURN 0;
            END alpha.)",
         "MODULE alpha;\nCONST\nc = 1.2;\nd = 1.2E+2;\nf = 2.3D+2;\nh = 0.23D-8;\nBEGIN\nRETURN "
         "0\nEND alpha.",
         ""},

        // Errors

        {R"(MODULE alpha; (* REAL *)
            CONST c = .E;
            BEGIN
                RETURN 0;
            END alpha.)",
         "", "2,23: Unexpected token: period"},
    };
    do_parse_tests(tests);
}

TEST(Parser, Const) {
    std::vector<ParseTests> tests = {

        {"MODULE y; CONST x = 1; BEGIN RETURN 12 END y.",
         "MODULE y;\nCONST\nx = 1;\nBEGIN\nRETURN 12\nEND y.", ""},
        {"MODULE y; CONST x = 1; y=2; BEGIN RETURN 12 END y.",
         "MODULE y;\nCONST\nx = 1;\ny = 2;\nBEGIN\nRETURN 12\nEND y.", ""},
        {"MODULE y; CONST x = 12X; END y.", "MODULE y;\nCONST\nx = 012X;\nEND y.", ""},

        // Errors
        {"MODULE y; x = 1; BEGIN RETURN 12 END y.", "",
         "1,11: Unexpected token: x - expecting END"},

    };
    do_parse_tests(tests);
}

TEST(Parser, Identifiers) {
    std::vector<ParseTests> tests = {

        {"MODULE y; CONST x = 1; BEGIN RETURN x END y.",
         "MODULE y;\nCONST\nx = 1;\nBEGIN\nRETURN x\nEND y.", ""},
        {"MODULE y; CONST x = 1; y=2; BEGIN RETURN x - y END y.",
         "MODULE y;\nCONST\nx = 1;\ny = 2;\nBEGIN\nRETURN x-y\nEND y.", ""},

        {"MODULE y; CONST x = 1; y=2; "
         "BEGIN RETURN (aa * bb) + ((zero + (dev + jones)) * 4) "
         "END y.",

         "MODULE y;\nCONST\nx = 1;\ny = 2;\nBEGIN\nRETURN  (aa*bb) + ( (zero+ "
         "(dev+jones) ) *4) \nEND y.",
         ""},
    };
    do_parse_tests(tests);
}

TEST(Parser, Var) {
    std::vector<ParseTests> tests = {

        {"MODULE y; VAR x : INTEGER; BEGIN RETURN 12 END y.",
         "MODULE y;\nVAR\nx: INTEGER;\nBEGIN\nRETURN 12\nEND y.", ""},
        {"MODULE y; VAR x : INTEGER; y: INTEGER; BEGIN RETURN 12 END y.",
         "MODULE y;\nVAR\nx: INTEGER;\ny: INTEGER;\nBEGIN\nRETURN 12\nEND y.", ""},
        {"MODULE y; "
         "CONST z = 10; "
         "VAR x : INTEGER; y: INTEGER; "
         "BEGIN RETURN 12 END y.",
         "MODULE y;\nCONST\nz = 10;\nVAR\nx: INTEGER;\ny: "
         "INTEGER;\nBEGIN\nRETURN 12\nEND y.",
         ""},

        // Errors
        {"MODULE y; VAR x : INTEGER BEGIN RETURN 12 END y.", "",
         "1,31: Unexpected token: BEGIN - expecting semicolon"},
        {"MODULE y; VAR : INTEGER; BEGIN RETURN 12 END y.", "",
         "1,15: Unexpected token: : - expecting END"},

    };
    do_parse_tests(tests);
}

TEST(Parser, VarList) {
    std::vector<ParseTests> tests = {

        {R"(MODULE y;
            VAR x, y: INTEGER;
            BEGIN 
                RETURN 12 
            END y.)",
         "MODULE y;\nVAR\nx: INTEGER;\ny: INTEGER;\nBEGIN\nRETURN 12\nEND y.", ""},
    };
    do_parse_tests(tests);
}

TEST(Parser, Assignment) {
    std::vector<ParseTests> tests = {

        {"MODULE y; VAR x : INTEGER; BEGIN x := 12 END y.",
         "MODULE y;\nVAR\nx: INTEGER;\nBEGIN\nx := 12\nEND y.", ""},
        {"MODULE y; VAR x : INTEGER; y: INTEGER; BEGIN "
         "x := 3; y := x + 5 END y.",
         "MODULE y;\nVAR\nx: INTEGER;\ny: INTEGER;\nBEGIN\nx := 3;\ny := "
         "x+5\nEND y.",
         ""},
        {"MODULE y; "
         "CONST z = 10; "
         "VAR x : INTEGER; y: INTEGER; "
         "BEGIN x := z * (2 + z) END y.",
         "MODULE y;\nCONST\nz = 10;\nVAR\nx: INTEGER;\ny: "
         "INTEGER;\nBEGIN\nx := z* (2+z) \nEND y.",
         ""},

        // Errors
        {"MODULE y; VAR x : INTEGER; BEGIN := 2 END y.", "", "1,35: Unexpected token: :="},
        {"MODULE y; VAR x : INTEGER;  BEGIN x 12 END y.", "",
         "1,38: Unexpected token: integer(12) - expecting :="},

    };
    do_parse_tests(tests);
}

TEST(Parser, Bools) {
    std::vector<ParseTests> tests = {

        {"MODULE y; VAR x : BOOLEAN; BEGIN x := TRUE END y.",
         "MODULE y;\nVAR\nx: BOOLEAN;\nBEGIN\nx := TRUE\nEND y.", ""},
        {"MODULE y; VAR x : BOOLEAN; BEGIN x := FALSE END y.",
         "MODULE y;\nVAR\nx: BOOLEAN;\nBEGIN\nx := FALSE\nEND y.", ""},
    };
    do_parse_tests(tests);
}

TEST(Parser, TYPES) {
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
    };
    do_parse_tests(tests);
}

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
            PROCEDURE f(x, y: INTEGER; bx, bby : BOOLEAN) : INTEGER;
            VAR zz : INTEGER;
            BEGIN
            RETURN zz
            END f;
            BEGIN
            RETURN 3
            END xxx.)",
         "MODULE xxx;\nVAR\nz: INTEGER;\nPROCEDURE f(x : INTEGER; y : INTEGER; "
         "bx : BOOLEAN; bby : BOOLEAN): INTEGER;\nVAR\nzz: "
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

TEST(Parser, ProcReceiver) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha; (* Procedures with recievers *)
                TYPE pt = RECORD x, y : INTEGER; END;

                PROCEDURE (x : pt) print();
                BEGIN
                END print;

                PROCEDURE (VAR a : pt) clear();
                BEGIN
                END clear;

                END alpha.)",
         "MODULE alpha;\nTYPE\npt = RECORD\nx: INTEGER;\ny: INTEGER\nEND;\nPROCEDURE (x : pt) "
         "print;\nEND print;\nPROCEDURE (VAR a : pt) clear;\nEND clear;\nEND alpha.",
         ""},

        {R"(MODULE alpha; (* Procedures with recievers *)
                TYPE pt = RECORD x, y : INTEGER; END;

                PROCEDURE ^ (x : pt) print;

                PROCEDURE ^ (VAR a : pt) clear;

                END alpha.)",
         "MODULE alpha;\nTYPE\npt = RECORD\nx: INTEGER;\ny: INTEGER\nEND;\nPROCEDURE ^ (x : pt) "
         "print;\nPROCEDURE ^ (VAR a : pt) clear;\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha; (* Procedures with recievers *)
                TYPE pt = RECORD x, y : INTEGER; END;

                PROCEDURE () print();
                BEGIN
                END print;

                END alpha.)",
         "", "4,28: Unexpected token: ) - expecting indent"},

        {R"(MODULE alpha; (* Procedures with recievers *)
                TYPE pt = RECORD x, y : INTEGER; END;

                PROCEDURE x : pt print();
                BEGIN
                END print;

                END alpha.)",
         "", "4,38: Unexpected token: print - expecting semicolon"},

        {R"(MODULE alpha; (* Procedures with recievers *)
                TYPE pt = RECORD x, y : INTEGER; END;

                PROCEDURE (x) print();
                BEGIN
                END print;

                END alpha.)",
         "", "4,29: Unexpected token: ) - expecting :"},

        {R"(MODULE alpha; (* Procedures with recievers *)
                TYPE pt = RECORD x, y : INTEGER; END;

                PROCEDURE ^ (VAR ) clear;

                END alpha.)",
         "", "4,34: Unexpected token: ) - expecting indent"},
    };
    do_parse_tests(tests);
}

TEST(Parser, IF) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF TRUE THEN
                x := 1;
                RETURN 1
            END
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nIF TRUE THEN\nx := "
         "1;\nRETURN 1\nEND\nEND alpha.",
         ""},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF TRUE THEN
                x := 1;
                RETURN 1
            ELSE
                RETURN 2
            END
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nIF TRUE THEN\nx := "
         "1;\nRETURN 1\nELSE\nRETURN 2\nEND\nEND alpha.",
         ""},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF TRUE THEN
                x := 1;
                RETURN 1
            ELSIF TRUE THEN
                RETURN 3
            ELSE
                RETURN 2
            END
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nIF TRUE THEN\nx := "
         "1;\nRETURN 1\nELSIF TRUE THEN\nRETURN 3\nELSE\nRETURN "
         "2\nEND\nEND alpha.",
         ""},

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
            ELSIF TRUE THEN
                RETURN 5
            ELSE
                RETURN 2
            END
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nIF TRUE THEN\nx := "
         "1;\nRETURN 1\nELSIF TRUE THEN\nRETURN 3\nELSIF TRUE THEN\nRETURN "
         "4\nELSIF TRUE THEN\nRETURN 5\nELSE\nRETURN 2\nEND\nEND alpha.",
         ""},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF TRUE THEN
            ELSE
            END
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nIF TRUE THEN\nELSE\nEND\nEND alpha.", ""},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF TRUE THEN
            ELSIF TRUE THEN
            ELSIF TRUE THEN
            ELSIF TRUE THEN
            ELSE
            END
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nIF TRUE THEN\nELSIF TRUE THEN\nELSIF TRUE "
         "THEN\nELSIF TRUE THEN\nELSE\nEND\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF TRUE 
                x := 1;
                RETURN 1
            END
        END alpha.)",
         "", "5,17: Unexpected token: x - expecting THEN"},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF TRUE THEN
                x := 1;
                RETURN 1
            
        END alpha.)",
         "", "8,19: Unexpected token: EOF - expecting indent"},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF TRUE THEN
                x := 1;
                RETURN 1
            ELSIF TRUE 
                RETURN 3
            ELSE
                RETURN 2
            END
        END alpha.)",
         "", "8,22: Unexpected token: RETURN - expecting THEN"},
    };
    do_parse_tests(tests);
}

TEST(Parser, FOR) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            FOR i := 0 TO 10 DO
                x := x + i
            END;
            RETURN x
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nFOR i := 0 TO 10 DO\nx := "
         "x+i\nEND;\nRETURN x\nEND alpha.",
         ""},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            FOR i := 0 TO 19 BY 2 DO
                x := x + i
            END;
            RETURN x
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nFOR i := 0 TO 19 BY 2 DO\nx "
         ":= x+i\nEND;\nRETURN x\nEND alpha.",
         ""},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            FOR i := 0 TO 19 BY 2 DO
            END;
            RETURN x
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nFOR i := 0 TO 19 BY 2 DO\nEND;\nRETURN x\nEND "
         "alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            FOR i 0 TO 19 BY 2 DO
                x := x + i
            END;
            RETURN x
        END alpha.)",
         "", "4,19: Unexpected token: integer(0) - expecting :="},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            FOR i := 0 19 BY 2 DO
                x := x + i
            END;
            RETURN x
        END alpha.)",
         "", "4,25: Unexpected token: integer(19) - expecting TO"},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            FOR i := 0 TO 19 2 DO
                x := x + i
            END;
            RETURN x
        END alpha.)",
         "", "4,30: Unexpected token: integer(2) - expecting DO"},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            FOR i := 0 TO 19 BY DO
                x := x + i
            END;
            RETURN x
        END alpha.)",
         "", "4,34: Unexpected token: DO"},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            FOR i := 0 TO 19 BY 2
                x := x + i
            END;
            RETURN x
        END alpha.)",
         "", "5,17: Unexpected token: x - expecting DO"},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            FOR i := 0 TO 19 BY 2 DO
                x := x + i
            RETURN x
        END alpha.)",
         "", "7,19: Unexpected token: EOF - expecting indent"},
    };
    do_parse_tests(tests);
}

TEST(Parser, WHILE) {
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

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            WHILE x < 10 DO
            END;
            RETURN x
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nWHILE x < 10 DO\nEND;\nRETURN x\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            WHILE x < 10
                x := x + 1
            END;
            RETURN x
        END alpha.)",
         "", "5,17: Unexpected token: x - expecting DO"},
    };
    do_parse_tests(tests);
}

TEST(Parser, REPEAT) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            REPEAT
                x := x+1
            UNTIL x > 10
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nREPEAT\nx := x+1\nUNTIL x > "
         "10\nEND alpha.",
         ""},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            REPEAT
            UNTIL x > 10
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nREPEAT\nUNTIL x > 10\nEND alpha.", ""},

        // Errors
        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            REPEAT
                x := x+1
             x > 10
        END alpha.)",
         "", "6,16: Unexpected token: > - expecting :="},
    };
    do_parse_tests(tests);
}

TEST(Parser, LOOP) {
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

        {R"(MODULE alpha;
        BEGIN
            LOOP
            END;
            RETURN x
        END alpha.)",
         "MODULE alpha;\nBEGIN\nLOOP\nEND;\nRETURN x\nEND alpha.", ""},
    };
    do_parse_tests(tests);
}

TEST(Parser, BEGIN) {
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

        {R"(MODULE alpha;
        BEGIN
            BEGIN
            END;
            RETURN x
        END alpha.)",
         "MODULE alpha;\nBEGIN\nBEGIN\nEND;\nRETURN x\nEND alpha.", ""},
    };
    do_parse_tests(tests);
}

TEST(Parser, CASE) {
    std::vector<ParseTests> tests = {

        {R"(MODULE alpha;
            BEGIN
                CASE i OF
                    1 : Out.String('One');
                |   2 : Out.String('Two');
                |   3 : Out.String('Three');
                ELSE
                    Out.String("More")
                END
                RETURN 0;
            END alpha.)",
         "MODULE alpha;\nBEGIN\nCASE i OF\n1 : Out.String('One');\n| 2 : Out.String('Two');\n| 3 "
         ": Out.String('Three');\nELSE\nOut.String(\"More\")\nEND;\nRETURN 0\nEND alpha.",
         ""},

        {R"(MODULE alpha;
            BEGIN
                CASE i OF
                    1 : Out.String('One');
                |   2 : Out.String('Two');
                |   3 : Out.String('Three');
                END
                RETURN 0;
            END alpha.)",
         "MODULE alpha;\nBEGIN\nCASE i OF\n1 : Out.String('One');\n| 2 : Out.String('Two');\n| 3 "
         ": Out.String('Three');\nEND;\nRETURN 0\nEND alpha.",
         ""},

        {R"(MODULE alpha;
            BEGIN
                CASE i OF
                |   1 : Out.String('One');
                |   2 : Out.String('Two');
                END
                RETURN 0;
            END alpha.)",
         "MODULE alpha;\nBEGIN\nCASE i OF\n1 : Out.String('One');\n| 2 : "
         "Out.String('Two');\nEND;\nRETURN 0\nEND alpha.",
         ""},

        {R"(MODULE alpha; (* CASE *)
            IMPORT Out;
            BEGIN
                FOR i := 1 TO 4 DO
                    CASE i OF
                        1 : Out.String('One');
                    |   2 : Out.String('Two');
                    |   3, 4, 5: Out.String('More');
                    END
                    Out.Ln;
                END
                RETURN 0;
            END alpha.)",
         "MODULE alpha;\nIMPORT Out;\nBEGIN\nFOR i := 1 TO 4 DO\nCASE i OF\n1 : "
         "Out.String('One');\n| 2 : Out.String('Two');\n| 3, 4, 5 : "
         "Out.String('More');\nEND;\nOut.Ln()\nEND;\nRETURN 0\nEND alpha.",
         ""},

        {R"(MODULE beta;
            IMPORT Out;
            VAR x :INTEGER;

            BEGIN
                CASE x OF
                    1..2 : Out.String("A"); Out.Ln;
                |   5, 6..8, 9 : Out.String("B,C"); Out.Ln;
                |   10..11, 12..15, 16 : Out.String("D-F"); Out.Ln;
                ELSE
                    Out.String('D-Z'); Out.Ln;
                END
                RETURN 0
            END beta.)",
         "MODULE beta;\nIMPORT Out;\nVAR\nx: INTEGER;\nBEGIN\nCASE x OF\n1..2 : "
         "Out.String(\"A\");\nOut.Ln();\n| 5, 6..8, 9 : Out.String(\"B,C\");\nOut.Ln();\n| "
         "10..11, 12..15, 16 : "
         "Out.String(\"D-F\");\nOut.Ln();\nELSE\nOut.String('D-Z');\nOut.Ln()\nEND;\nRETURN "
         "0\nEND beta.",
         ""},

        {R"(MODULE alpha; (* CASE *)
            IMPORT Out;
            VAR c : CHAR;

            BEGIN
                CASE c OF
                    'A' : Out.String("A"); Out.Ln;
                |   'B', 'C' : Out.String("B,C"); Out.Ln;
                |   'D'..'F' : Out.String("D-F"); Out.Ln;
                ELSE
                    Out.String('D-Z'); Out.Ln;
                END
                RETURN 0;
            END alpha.)",
         "MODULE alpha;\nIMPORT Out;\nVAR\nc: CHAR;\nBEGIN\nCASE c OF\n'A' : "
         "Out.String(\"A\");\nOut.Ln();\n| 'B', 'C' : Out.String(\"B,C\");\nOut.Ln();\n| 'D'..'F' "
         ": Out.String(\"D-F\");\nOut.Ln();\nELSE\nOut.String('D-Z');\nOut.Ln()\nEND;\nRETURN "
         "0\nEND alpha.",
         ""},

        {R"(MODULE alpha; (* CASE *)
            IMPORT Out;
            VAR c : CHAR;

            BEGIN
                CASE c OF
                    'A' :
                |   'B', 'C' : 
                |   'D' .. 'F' : 
                ELSE
                END
                RETURN 0;
            END alpha.)",
         "MODULE alpha;\nIMPORT Out;\nVAR\nc: CHAR;\nBEGIN\nCASE c OF\n'A' : | 'B', 'C' : | "
         "'D'..'F' : END;\nRETURN 0\nEND alpha.",
         ""},

        // Errors

        {R"(MODULE alpha;
            BEGIN
                CASE i
                |   1 : Out.String('One');
                |   2 : Out.String('Two');
                END
                RETURN 0;
            END alpha.)",
         "", "4,17: Unexpected token: | - expecting OF"},

        {R"(MODULE alpha;
            BEGIN
                CASE i OF
                |   1 : Out.String('One');
                   2 : Out.String('Two');
                END
                RETURN 0;
            END alpha.)",
         "", "5,20: Unexpected token: integer(2)"},

        {R"(MODULE alpha;
            BEGIN
                CASE i OF
                |   1 : Out.String('One');
                |   2 : Out.String('Two');
                
                RETURN 0;
            END alpha.)",
         "", "9,0: Unexpected token: EOF - expecting indent"},

        {R"(MODULE alpha; (* CASE *)
            IMPORT Out;
            BEGIN
                CASE i OF
                    1 : Out.String('One');
                |   2 : Out.String('Two');
                |   3, 4, : Out.String('More');
                END
                Out.Ln;
                RETURN 0;
            END alpha.)",
         "", "7,27: Unexpected token: :"},

        {R"(MODULE alpha; (* CASE *)
            IMPORT Out;
            BEGIN
                CASE i OF
                    1 : Out.String('One');
                |   2 : Out.String('Two');
                |   3 4 : Out.String('More');
                END
                Out.Ln;
                RETURN 0;
            END alpha.)",
         "", "7,23: Unexpected token: integer(4) - expecting :"},

        {R"(MODULE alpha; (* CASE *)
            IMPORT Out;
            BEGIN
                CASE i OF
                    1.. : Out.String('One');
                END
                Out.Ln;
                RETURN 0;
            END alpha.)",
         "", "5,26: Unexpected token: :"},

        {R"(MODULE alpha; (* CASE *)
            IMPORT Out;
            BEGIN
                CASE i OF
                    ..3 : Out.String('One');
                END
                Out.Ln;
                RETURN 0;
            END alpha.)",
         "", "5,22: Unexpected token: .."},

    };
    do_parse_tests(tests);
}

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
