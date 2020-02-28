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

#include "parse_test.hh"

using namespace ax;

static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         [](int ch) { return !std::isspace(ch); })
                .base(),
            s.end());
}

void do_parse_tests(std::vector<ParseTests> &tests) {
    for (auto const &t : tests) {

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
            EXPECT_EQ(e.error_msg(), t.error);
        } catch (std::exception &e) {
            std::cerr << "Exception: " << e.what() << std::endl;
            FAIL();
        }
    }
}
