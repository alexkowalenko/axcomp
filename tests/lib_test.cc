//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <exception>
#include <sstream>

#include "gtest/gtest.h"

#include "error.hh"
#include "inspector.hh"
#include "lexer.hh"
#include "parser.hh"
#include "printer.hh"
#include "symboltable.hh"
#include "typetable.hh"

#include "parse_test.hh"

using namespace ax;

void do_parse_tests(std::vector<ParseTests> &tests) {
    TypeTable types;
    types.initialise();

    for (auto const &t : tests) {

        std::istringstream is(t.input);
        ErrorManager       errors;
        Lexer              lex(is, errors);

        auto   symbols = std::make_shared<SymbolTable<TypePtr>>(nullptr);
        Parser parser(lex, symbols, errors);

        std::string result;
        try {
            std::cout << t.input << std::endl;
            auto ast = parser.parse();

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

void do_inspect_tests(std::vector<ParseTests> &tests) {
    TypeTable types;
    types.initialise();

    for (auto const &t : tests) {

        std::istringstream is(t.input);
        ErrorManager       errors;
        Lexer              lex(is, errors);

        auto   symbols = std::make_shared<SymbolTable<TypePtr>>(nullptr);
        Parser parser(lex, symbols, errors);

        std::string result;
        try {
            std::cout << t.input << std::endl;
            auto ast = parser.parse();
            parser.setup_builtins();

            Inspector inpect(symbols, types, errors);
            inpect.check(ast);
            if (errors.has_errors()) {
                EXPECT_EQ(errors.first()->error_msg(), t.error);
                continue;
            }

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
