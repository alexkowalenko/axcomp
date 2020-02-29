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

TEST(Inspector, Type) {
    std::vector<ParseTests> tests = {
        {"MODULE x; VAR z: INT; BEGIN x := 10; END x.", "",
         "0: Unknown type: INT"},
    };
    do_inspect_tests(tests);
}

TEST(Inspector, Return) {
    std::vector<ParseTests> tests = {
        {"MODULE x; VAR z: INTEGER; BEGIN x := 10; END x.", "",
         "0: MODULE x has no RETURN function"},
        {"MODULE x; VAR z: INTEGER; PROCEDURE y; BEGIN x := 1; END y; "
         "BEGIN x := 10; END x.",
         "", "0: PROCEDURE y has no RETURN function"},
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