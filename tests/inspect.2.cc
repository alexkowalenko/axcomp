//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "gtest/gtest.h"

#include "parse_test.hh"

void do_inspect_tests(std::vector<ParseTests> &tests);

TEST(Inspector, IF) {
    std::vector<ParseTests> tests = {
        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF x > 12 THEN
                x := 1;
                RETURN 1;
            END;
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nIF x > 12 THEN\nx := "
         "1;\nRETURN 1;\nEND;\nEND alpha.",
         ""},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF x = 12 THEN
                x := 1;
                RETURN 1;
            ELSIF x < 3 THEN
                RETURN 3;
            ELSE
                RETURN 2;
            END;
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nIF x = 12 THEN\nx := "
         "1;\nRETURN 1;\nELSIF x < 3 THEN\nRETURN 3;\nELSE\nRETURN "
         "2;\nEND;\nEND alpha.",
         ""},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF x = 12 THEN
                x := 1;
                RETURN 1;
            ELSIF (x < 12) OR (x > 12) THEN
                RETURN 3;
            ELSIF x > 12 THEN
                RETURN 4;
            ELSIF x + x * x > 20 THEN
                RETURN 5;
            ELSE
                RETURN 2;
            END;
        END alpha.)",
         "MODULE alpha;\nVAR\nx: INTEGER;\nBEGIN\nIF x = 12 THEN\nx := "
         "1;\nRETURN 1;\nELSIF  (x < 12)  OR  (x > 12)  THEN\nRETURN 3;\nELSIF "
         "x > 12 THEN\nRETURN 4;\nELSIF x+x*x > 20 THEN\nRETURN "
         "5;\nELSE\nRETURN 2;\nEND;\nEND alpha.",
         ""},

        // Errors
        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF x THEN
                x := 1;
                RETURN 1;
            END;
        END alpha.)",
         "", "0: IF expression must be type BOOLEAN"},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF TRUE THEN
                x := 1;
                RETURN 1;
            ELSIF 34345 + 38489 THEN
                RETURN 3;
            ELSE
                RETURN 2;
            END;
        END alpha.)",
         "", "0: ELSIF expression must be type BOOLEAN"},

        {R"(MODULE alpha;
        VAR x : INTEGER;
        BEGIN
            IF TRUE THEN
                x := 1;
                RETURN 1;
            ELSIF TRUE THEN
                RETURN 3;
            ELSIF TRUE THEN
                RETURN 4;
            ELSIF 3747 * (x - 3) THEN
                RETURN 5;
            ELSE
                RETURN 2;
            END;
        END alpha.)",
         "", "0: ELSIF expression must be type BOOLEAN"},
    };
    do_inspect_tests(tests);
}
