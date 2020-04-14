//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <exception>
#include <sstream>

#include "gtest/gtest.h"

#include "defparser.hh"
#include "defprinter.hh"
#include "error.hh"
#include "fake_importer.hh"
#include "inspector.hh"
#include "lexer.hh"
#include "parser.hh"
#include "printer.hh"
#include "symboltable.hh"
#include "type.hh"
#include "typetable.hh"

#include "parse_test.hh"

using namespace ax;

void do_parse_tests(std::vector<ParseTests> &tests) {

    for (auto const &t : tests) {

        std::istringstream is(t.input);
        ErrorManager       errors;
        Lexer              lex(is, errors);
        TypeTable          types;
        types.initialise();

        auto   symbols = make_Symbols(nullptr);
        Parser parser(lex, symbols, types, errors);

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

    for (auto const &t : tests) {

        std::istringstream is(t.input);
        ErrorManager       errors;
        Lexer              lex(is, errors);
        TypeTable          types;
        types.initialise();

        auto   symbols = make_Symbols(nullptr);
        Parser parser(lex, symbols, types, errors);

        std::string result;
        try {
            std::cout << t.input << std::endl;
            auto ast = parser.parse();
            parser.setup_builtins();

            Importer  importer(errors);
            Inspector inpect(symbols, types, errors, importer);
            inpect.check(ast);
            if (errors.has_errors()) {
                errors.print_errors(std::cerr);
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

void do_inspect_fimport_tests(std::vector<ParseTests> &tests) {

    for (auto const &t : tests) {

        std::istringstream is(t.input);
        ErrorManager       errors;
        Lexer              lex(is, errors);
        TypeTable          types;
        types.initialise();

        auto   symbols = make_Symbols(nullptr);
        Parser parser(lex, symbols, types, errors);

        std::string result;
        try {
            std::cout << t.input << std::endl;
            auto ast = parser.parse();
            parser.setup_builtins();

            FakeImporter importer(errors);
            Inspector    inpect(symbols, types, errors, importer);
            inpect.check(ast);
            if (errors.has_errors()) {
                errors.print_errors(std::cerr);
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

void do_def_tests(std::vector<ParseTests> &tests) {

    for (auto const &t : tests) {

        std::istringstream is(t.input);
        ErrorManager       errors;
        Lexer              lex(is, errors);
        TypeTable          types;
        types.initialise();

        auto   symbols = make_Symbols(nullptr);
        Parser parser(lex, symbols, types, errors);

        std::string result;
        try {
            std::cout << t.input << std::endl;
            auto ast = parser.parse();
            parser.setup_builtins();

            Importer  importer(errors);
            Inspector inpect(symbols, types, errors, importer);
            inpect.check(ast);
            if (errors.has_errors()) {
                EXPECT_EQ(errors.first()->error_msg(), t.error);
                continue;
            }

            std::ostringstream outstr;
            DefPrinter         prt(outstr);
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

void do_defparse_tests(std::vector<ParseTests> &tests) {

    for (auto const &t : tests) {

        std::istringstream is(t.input);
        ErrorManager       errors;
        Lexer              lex(is, errors);
        TypeTable          types;
        types.initialise();

        auto   symbols = make_Symbols(nullptr);
        Parser parser(lex, symbols, types, errors);

        std::string result;
        try {
            std::cout << t.input << std::endl;
            auto ast = parser.parse();
            parser.setup_builtins();

            Importer  importer(errors);
            Inspector inpect(symbols, types, errors, importer);
            inpect.check(ast);
            if (errors.has_errors()) {
                EXPECT_EQ(errors.first()->error_msg(), t.error);
                continue;
            }

            std::ostringstream outstr;
            DefPrinter         prt(outstr);
            prt.print(ast);
            result = outstr.str();

            std::cerr << "Result of definition: \n" << result;

            std::istringstream dis(result);
            Lexer              dlex(dis, errors);
            DefParser          dparser(dlex, symbols, types, errors);
            auto               dast = dparser.parse();

            std::ostringstream doutstr;
            DefPrinter         dprt(doutstr);
            dprt.print(ast);
            result = doutstr.str();
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
