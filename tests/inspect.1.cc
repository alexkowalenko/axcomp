//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <sstream>

#include "gtest/gtest.h"

#include "error.hh"
#include "inspector.hh"
#include "lexer.hh"
#include "parser.hh"
#include "printer.hh"
#include "symboltable.hh"
#include "token.hh"
#include "typetable.hh"

#include "parse_test.hh"

using namespace ax;

void do_inspect_tests(std::vector<ParseTests> &tests);

TEST(Inspector, VarType) {
    std::vector<ParseTests> tests = {
        {"MODULE x; VAR z: complex; BEGIN x := 10; END x.", "",
         "0: Unknown type: complex"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, UnknownExpr) {
    std::vector<ParseTests> tests = {
        {"MODULE y; VAR x : INTEGER; BEGIN RETURN x; END y.",
         "MODULE y;\nVAR\nx: INTEGER;\nBEGIN\nRETURN x;\nEND y.", ""},

        // Errors
        {"MODULE y; VAR x : INTEGER; BEGIN RETURN z; END y.", "",
         "0: undefined identifier z"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, Return) {
    std::vector<ParseTests> tests = {
        {"MODULE x; VAR z: INTEGER; BEGIN x := 10; RETURN x; END x.",
         "MODULE x;\nVAR\nz: INTEGER;\nBEGIN\nx := 10;\nRETURN x;\nEND x.", ""},

        // Errors
        {"MODULE x; VAR z: INTEGER; BEGIN x := 10; END x.", "",
         "0: MODULE x has no RETURN function"},
        {"MODULE x; VAR z: INTEGER; PROCEDURE y; BEGIN x := 1; END y; "
         "BEGIN x := 10; END x.",
         "", "0: PROCEDURE y has no RETURN function"},
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

        // Error
        {"MODULE x; PROCEDURE f(): complex; BEGIN RETURN 0; END f; BEGIN "
         "RETURN 333; END x.",
         "", "0: Unknown type: complex for return from function f"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, Call) {
    std::vector<ParseTests> tests = {
        {"MODULE y; VAR x : INTEGER; PROCEDURE f; BEGIN RETURN 0; END f; BEGIN "
         "f(); RETURN x; END y.",
         "MODULE y;\nVAR\nx: INTEGER;\nPROCEDURE f;\nBEGIN\nRETURN 0;\nEND "
         "f.\nBEGIN\nf();\nRETURN x;\nEND y.",
         ""},
        {"MODULE y; VAR x : INTEGER; "
         "PROCEDURE f():INTEGER; BEGIN RETURN 0; END f; "
         "BEGIN f(); RETURN f(); END y.",
         "MODULE y;\nVAR\nx: INTEGER;\nPROCEDURE f(): INTEGER;\nBEGIN\nRETURN "
         "0;\nEND f.\nBEGIN\nf();\nRETURN f();\nEND y.",
         ""},

        // Errors
        {"MODULE y; VAR x : INTEGER; PROCEDURE f; BEGIN RETURN 0; END f; BEGIN "
         "x(); RETURN x; END y.",
         "", "0: x is not a PROCEDURE"},
        {"MODULE y; VAR x : INTEGER; PROCEDURE f; BEGIN RETURN 0; END f; BEGIN "
         "g(); RETURN x; END y.",
         "", "0: undefined PROCEDURE g"},
        {"MODULE y; VAR x : INTEGER; "
         "PROCEDURE f():INTEGER; BEGIN RETURN 0; END f; "
         "BEGIN RETURN g(); END y.",
         "", "0: undefined PROCEDURE g"},
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
         "", "0: Unknown type: UNDEF for paramater x from function f"},
    };
    do_inspect_tests(tests);
}

void do_inspect_tests(std::vector<ParseTests> &tests) {
    TypeTable types;
    types.initialise();

    for (auto const &t : tests) {

        std::istringstream is(t.input);
        Lexer              lex(is);

        SymbolTable<Symbol> symbols(nullptr);
        Parser              parser(lex, symbols);

        std::string result;
        try {
            std::cout << t.input << std::endl;
            auto ast = parser.parse();

            Inspector inpect(symbols, types);
            inpect.check(ast);

            std::ostringstream outstr;
            ASTPrinter         prt(outstr);
            prt.print(ast);
            result = outstr.str();
            rtrim(result);

            EXPECT_EQ(result, t.output);
        } catch (AXException &e) {
            EXPECT_EQ(e.error_msg(), t.error);
        } catch (std::exception &e) {
            std::cerr << "Exception: " << e.what() << std::endl;
            FAIL();
        }
    }
}