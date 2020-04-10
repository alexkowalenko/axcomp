//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <string>
#include <vector>

#pragma once

struct ParseTests {
    std::string input;
    std::string output;
    std::string error;
};

void do_parse_tests(std::vector<ParseTests> &tests);
void do_inspect_tests(std::vector<ParseTests> &tests);
void do_def_tests(std::vector<ParseTests> &tests);

inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         [](int ch) { return !std::isspace(ch); })
                .base(),
            s.end());
}
