//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <string>

namespace ax {

struct Options {
    bool debug_parse{false};

    bool output_funct{false};
    bool only_ll{false};
    bool print_symbols{false};

    std::string file_name;
};

} // namespace ax