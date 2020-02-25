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