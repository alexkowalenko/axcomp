//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <exception>
#include <sstream>

#include "gtest/gtest.h"

#include "error.hh"
#include "lexer.hh"
#include "parser.hh"
#include "printer.hh"

using namespace ax;

struct ParseTests {
    std::string input;
    std::string output;
    std::string error;
};

void do_parse_tests(std::vector<ParseTests> &tests);

TEST(Parser, Comments) {
    std::vector<ParseTests> tests = {

        {"(*hello*) MODULE y; BEGIN 12; END y.",
         "MODULE y;\nBEGIN\n12;\nEND y.", ""},
        {"(*hello*) MODULE (* hello *) y; (* hello *)BEGIN (* hello *)12; (* "
         "hello *)END y.(* hello *)",
         "MODULE y;\nBEGIN\n12;\nEND y.", ""},
        {"(*hello*) MODULE (* hello *) y; (* hello *)BEGIN (* hello 12; 24; *) "
         "36; (*hello *) END y.(* hello *)",
         "MODULE y;\nBEGIN\n36;\nEND y.", ""},
        {"(*hello*) MODULE (* hello *) y; (* hello *)BEGIN (* hello (* 12; 24; "
         "*) *) "
         "36; (*hello *) END y.(* hello *)",
         "MODULE y;\nBEGIN\n36;\nEND y.", ""},

    };
    do_parse_tests(tests);
}

TEST(Parser, Module) {
    std::vector<ParseTests> tests = {

        {"MODULE y; BEGIN 12; END y.", "MODULE y;\nBEGIN\n12;\nEND y.", ""},
        {"MODULE y; BEGIN 12; 24; END y.", "MODULE y;\nBEGIN\n12;\n24;\nEND y.",
         ""},

        // Errors
        {"y; BEGIN 12; END y.", "",
         "1: Unexpected token: indent(y) - expecting MODULE"},
        {"MODULE ; BEGIN 12; END y.", "",
         "1: Unexpected token: semicolon - expecting indent"},
        {"MODULE y BEGIN 12; END y.", "",
         "1: Unexpected token: BEGIN - expecting semicolon"},
        {"MODULE y; 12; END y.", "",
         "1: Unexpected token: integer(12) - expecting BEGIN"},
        {"MODULE y; BEGIN ; END y.", "", "1: Expecting an integer: ;"},
        {"MODULE y; BEGIN 12; y.", "", "1: Expecting an integer: y"},
        {"MODULE y; BEGIN 12; END .", "",
         "1: Unexpected token: period - expecting indent"},
        {"MODULE y; BEGIN 12; END y", "",
         "1: Unexpected token: EOF - expecting period"},

        {"MODULE x; BEGIN 12; END y.", "",
         "1: END identifier name: y doesn't match module name: x"},
    };
    do_parse_tests(tests);
}

TEST(Parser, Plus) {
    std::vector<ParseTests> tests = {

        {"MODULE y; BEGIN - 1; END y.", "MODULE y;\nBEGIN\n-1;\nEND y.", ""},
        {"MODULE y; BEGIN + 1; END y.", "MODULE y;\nBEGIN\n+1;\nEND y.", ""},

        {"MODULE y; BEGIN 1 + 1; END y.", "MODULE y;\nBEGIN\n1+1;\nEND y.", ""},
        {"MODULE y; BEGIN 2 - 2; END y.", "MODULE y;\nBEGIN\n2-2;\nEND y.", ""},
        {"MODULE y; BEGIN 1 + 2 - 3; END y.",
         "MODULE y;\nBEGIN\n1+2-3;\nEND y.", ""},
        {"MODULE y; BEGIN 1 + 2 - 3 + 4; END y.",
         "MODULE y;\nBEGIN\n1+2-3+4;\nEND y.", ""},

        {"MODULE y; BEGIN - 1 + 2; END y.", "MODULE y;\nBEGIN\n-1+2;\nEND y.",
         ""},

        // Errors
        {"MODULE y; BEGIN - ; END y.", "", "1: Expecting an integer: ;"},
        {"MODULE y; BEGIN 1 -; END y.", "", "1: Expecting an integer: ;"},
        {"MODULE y; BEGIN - 1 + 2 + ; END y.", "",
         "1: Expecting an integer: ;"},
    };
    do_parse_tests(tests);
}

static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         [](int ch) { return !std::isspace(ch); })
                .base(),
            s.end());
}

void do_parse_tests(std::vector<ParseTests> &tests) {
    for (auto t : tests) {

        std::istringstream is(t.input);
        Lexer              lex(is);
        Parser             parser(lex);

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
            std::cout << t.error << std::endl;
            EXPECT_EQ(e.error_msg(), t.error);
        } catch (std::exception &e) {
            std::cerr << "Exception: " << e.what() << std::endl;
            FAIL();
        }
    }
}
