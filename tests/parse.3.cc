//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "gtest/gtest.h"

#include "parse_test.hh"

TEST(Parser, Proc) {
    std::vector<ParseTests> tests = {

        {R"(
MODULE x;
  PROCEDURE f;
  BEGIN
      RETURN 12;
  END f;
BEGIN
    RETURN 0;
END x.)",
         "MODULE x;\nPROCEDURE f;\nBEGIN\nRETURN 12;\nEND f.\nBEGIN\nRETURN "
         "0;\nEND x.",
         ""},

        {R"(
MODULE x;
  PROCEDURE f;
  BEGIN
      RETURN 12;
  END f;

  PROCEDURE g;
  BEGIN
      RETURN 24;
  END g;
BEGIN
    RETURN 0;
END x.)",
         "MODULE x;\nPROCEDURE f;\nBEGIN\nRETURN 12;\nEND f.\nPROCEDURE "
         "g;\nBEGIN\nRETURN 24;\nEND g.\nBEGIN\nRETURN 0;\nEND x.",
         ""},

        // Errors
        {R"(
MODULE x;
  PROCEDURE f;
      RETURN 12;
  END f;
BEGIN
    RETURN 0;
END x.
        )",
         "", "4: Unexpected token: RETURN - expecting BEGIN"},

        {R"(MODULE x;
  PROCEDURE f;
  BEGIN
      RETURN 12;
  END ;
BEGIN
    RETURN 0;
END x.)",
         "", "5: Unexpected token: semicolon - expecting indent"},

        {R"(MODULE x;
   f;
  BEGIN
      RETURN 12;
  END f;
BEGIN
    RETURN 0;
END x.)",
         "", "2: Unexpected token: f - expecting BEGIN"},

        {R"(MODULE x;
  PROCEDURE f;
  BEGIN
      RETURN 12;
   f;
BEGIN
    RETURN 0;
END x.)",
         "", "5: Unexpected token: semicolon"},

    };
    do_parse_tests(tests);
}

TEST(Parser, Call) {
    std::vector<ParseTests> tests = {

        {R"(
MODULE x;
  PROCEDURE f;
  BEGIN
      RETURN 12;
  END f;
BEGIN
    f();
END x.)",
         "MODULE x;\nPROCEDURE f;\nBEGIN\nRETURN 12;\nEND f.\nBEGIN\nf();\nEND "
         "x.",
         ""},

        {R"(
MODULE x;
  PROCEDURE f;
  BEGIN
      RETURN 12;
  END f;

  PROCEDURE g;
  BEGIN
      f();
      RETURN 24;
  END g;
BEGIN
    g();
    RETURN 0;
END x.)",
         "MODULE x;\nPROCEDURE f;\nBEGIN\nRETURN 12;\nEND f.\nPROCEDURE "
         "g;\nBEGIN\nf();\nRETURN 24;\nEND g.\nBEGIN\ng();\nRETURN 0;\nEND x.",
         ""},

        // Error
        {R"(MODULE x;
    BEGIN
        f(;
    END x.)",
         "", "3: Unexpected token: semicolon"},
        // Error
        {R"(MODULE x;
    BEGIN
        f);
    END x.)",
         "", "3: Unexpected token: )"}};
    do_parse_tests(tests);
}

TEST(Parser, ReturnType) {
    std::vector<ParseTests> tests = {

        {R"(MODULE x;
            PROCEDURE f(): INTEGER;
            BEGIN RETURN 12;
            END f;
            BEGIN
            f();
            END x.)",
         "MODULE x;\nPROCEDURE f(): INTEGER;\nBEGIN\nRETURN 12;\nEND "
         "f.\nBEGIN\nf();\nEND x.",
         ""},

        {R"(MODULE x;
        PROCEDURE f(): INTEGER;
        BEGIN RETURN 12;
        END f;
        PROCEDURE g;
        BEGIN RETURN 24;
        END g;
        BEGIN
            g();
        RETURN 0;
        END x.)",
         "MODULE x;\nPROCEDURE f(): INTEGER;\nBEGIN\nRETURN 12;\nEND "
         "f.\nPROCEDURE g;\nBEGIN\nRETURN 24;\nEND g.\nBEGIN\ng();\nRETURN "
         "0;\nEND x.",
         ""},

        // Error
        {R"(MODULE x;
        PROCEDURE f() INTEGER;
        BEGIN
            RETURN 12;
        END f;
        BEGIN
            f();
        END x.)",
         "", "2: Unexpected token: INTEGER - expecting semicolon"},
        {R"(MODULE x;
        PROCEDURE f() :
        BEGIN
            RETURN 12;
        END f;
        BEGIN
            f();
        END x.)",
         "", "3: Unexpected token: BEGIN - expecting indent"},
    };
    do_parse_tests(tests);
}

TEST(Parser, FunctionCall) {
    std::vector<ParseTests> tests = {

        {R"(MODULE x;
            PROCEDURE f(): INTEGER;
            BEGIN RETURN 12;
            END f;
            BEGIN
            RETURN f() 
                + (f() * f());
            END x.)",
         "MODULE x;\nPROCEDURE f(): INTEGER;\nBEGIN\nRETURN 12;\nEND "
         "f.\nBEGIN\nRETURN f()+ (f()*f()) ;\nEND x.",
         ""},

        // Error
        {R"(MODULE x;
            PROCEDURE f(): INTEGER;
            BEGIN RETURN 12;
            END f;
            BEGIN
            RETURN f(;
            END x.)",
         "", "6: Unexpected token: semicolon"},
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
            PROCEDURE f(x  INTEGER) : INTEGER;
            VAR zz : INTEGER;
            BEGIN
            RETURN zz;
            END f;
            BEGIN
            RETURN 3;
            END xxx.)",
         "", "3: Unexpected token: INTEGER - expecting :"},
        {R"(MODULE xxx;
            VAR z : INTEGER;
            PROCEDURE f(x : ) : INTEGER;
            VAR zz : INTEGER;
            BEGIN
            RETURN zz;
            END f;
            BEGIN
            RETURN 3;
            END xxx.)",
         "", "3: Unexpected token: ) - expecting indent"},
        {R"(MODULE xxx;
            VAR z : INTEGER;
            PROCEDURE f(x : INTEGER  y: INTEGER) : INTEGER;
            VAR zz : INTEGER;
            BEGIN
            RETURN zz;
            END f;
            BEGIN
            RETURN 3;
            END xxx.)",
         "", "3: expecting ; or ) in parameter list"},
    };
    do_parse_tests(tests);
}
