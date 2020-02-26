//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "gtest/gtest.h"

#include "parse_test.hh"

TEST(Parser, Proc_1) {
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

        {R"(
MODULE x;
  CONST
        y = 3;
  VAR
    z : INTEGER;
    
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
         "MODULE x;\nCONST\ny = 3;\nVAR\nz: INTEGER;\nPROCEDURE "
         "f;\nBEGIN\nRETURN 12;\nEND f.\nPROCEDURE g;\nBEGIN\nRETURN 24;\nEND "
         "g.\nBEGIN\nRETURN 0;\nEND x.",
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
         "", "5: Unexpected token: semicolon - expecting :="},

    };
    do_parse_tests(tests);
}
