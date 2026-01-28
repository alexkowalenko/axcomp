//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <exception>
#include <sstream>

#include <format>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcharacter-conversion"
#include <gtest/gtest.h>
#pragma clang diagnostic pop

#include "builtin.hh"
#include "defparser.hh"
#include "defprinter.hh"
#include "error.hh"
#include "fake_importer.hh"
#include "inspector.hh"
#include "lexer.hh"
#include "parser.hh"
#include "printer.hh"
#include "symboltable.hh"
#include "token.hh"
#include "typetable.hh"

#include "parse_test.hh"

using namespace ax;

void do_lex_tests(std::vector<LexTests> &tests) {
    for (auto const &t : tests) {

        ErrorManager errors;
        LexerUTF8    lex(t.input, errors);

        try {
            auto token = lex.get_token();
            std::cout << std::string(std::format("Scan {0} get {1}", t.input, token.val))
                      << std::endl;
            EXPECT_EQ(token.type, t.token);
            if (t.token == TokenType::CHR) {
                EXPECT_EQ(token.val_int, t.val_int);
            } else {
                EXPECT_EQ(token.val, t.val);
            }
        } catch (LexicalException &l) {
            std::cerr << "Exception: " << l.error_msg() << std::endl;
            if (l.error_msg() != t.val) {
                FAIL();
            }
        } catch (...) {
            std::cerr << "Unknown Exception" << std::endl;
            FAIL();
        }
    }
}

// Different lexers return slightly different positions
// check the message.
bool check_errors(std::string const &e1, std::string const &e2) {
    const auto s1 = e1.substr(e1.find(':'));
    const auto s2 = e2.substr(e2.find(':'));
    return s1 == s2;
}

void do_parse_tests(std::vector<ParseTests> &tests) {

    for (auto const &t : tests) {

        ErrorManager errors;
        LexerUTF8    lex(t.input, errors);
        TypeTable    types;
        types.initialise();

        SymbolFrameTable symbols;
        Parser           parser(lex, symbols, types, errors);

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
            if (t.error.empty()) {
                std::cout << "Expect: " << t.error << "\ngot   : " << e.error_msg() << std::endl;
                EXPECT_TRUE(false);
                continue;
            }
            bool x = check_errors(e.error_msg(), t.error);
            if (!x) {
                std::cout << "Expect: " << t.error << "\ngot   : " << e.error_msg() << std::endl;
            }
            EXPECT_TRUE(x);
        } catch (std::exception &e) {
            std::cerr << "Exception: " << e.what() << std::endl;
            FAIL();
        }
    }
}

void do_inspect_tests(std::vector<ParseTests> &tests) {

    for (auto const &t : tests) {

        ErrorManager errors;
        LexerUTF8    lex(t.input, errors);
        TypeTable    types;
        types.initialise();

        SymbolFrameTable symbols;
        Builtin::initialise(symbols);
        Parser parser(lex, symbols, types, errors);

        std::string result;
        try {
            std::cout << t.input << std::endl;
            auto ast = parser.parse();

            Importer  importer(errors);
            Inspector inspect(symbols, types, errors, importer);
            inspect.check(ast);
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

        ErrorManager errors;
        LexerUTF8    lex(t.input, errors);
        TypeTable    types;
        types.initialise();

        SymbolFrameTable symbols;
        Builtin::initialise(symbols);
        Parser parser(lex, symbols, types, errors);

        std::string result;
        try {
            std::cout << t.input << std::endl;
            auto ast = parser.parse();

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

        ErrorManager errors;
        LexerUTF8    lex(t.input, errors);
        TypeTable    types;
        types.initialise();

        SymbolFrameTable symbols;
        Builtin::initialise(symbols);
        Parser parser(lex, symbols, types, errors);

        std::string result;
        try {
            std::cout << t.input << std::endl;
            auto ast = parser.parse();

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

        ErrorManager errors;
        LexerUTF8    lex(t.input, errors);
        TypeTable    types;
        types.initialise();

        SymbolFrameTable symbols;
        Builtin::initialise(symbols);
        Parser parser(lex, symbols, types, errors);

        std::string result;
        try {
            std::cout << t.input << std::endl;
            auto ast = parser.parse();

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

            LexerUTF8 dlex(result, errors);
            DefParser dparser(dlex, symbols, types, errors);
            auto      dast = dparser.parse();

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
