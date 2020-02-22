//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

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

std::vector<ParseTests> tests = {
    {"1", "1;", ""},
    {"12", "12;", ""},

    {"12;", "12;", ""},
    {"12;24;", "12;\n24;", ""},

    {"(*hello*) 12;", "12;", ""},
    {"(*hello*) 12; (*hello*) 24; (*hello*)", "12;\n24;", ""},
    {"(*hello 12; hello 24; hello*) 36", "36;", ""},
    {"(*hello*) (* 12; (*hello*) 24; (*hello*) *) 36", "36;", ""},

    // Error
    {"12;;", "12;\n24;", "1: Expecting an integer: ;"},
};

static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         [](int ch) { return !std::isspace(ch); })
                .base(),
            s.end());
}

TEST(Parser, Tests) {
    for (auto t : tests) {

        std::istringstream is(t.input);
        Lexer              lex(is);
        Parser             parser(lex);

        try {
            auto ast = parser.parse();

            std::ostringstream outstr;
            ASTPrinter         prt(outstr);
            prt.print(ast);
            auto result = outstr.str();
            rtrim(result);
            std::cout << result << std::endl;
            EXPECT_EQ(result, t.output);
        } catch (AXException &e) {
            std::cout << t.error << std::endl;
            EXPECT_EQ(e.error_msg(), t.error);
        } catch (...) {
            std::cerr << "Unknown error " << std::endl;
            FAIL();
        }
    }
}
