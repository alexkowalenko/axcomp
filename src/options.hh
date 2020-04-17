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
    bool output_main{false};
    bool only_ll{false};
    bool output_defs{false};
    bool print_symbols{false};

    std::string axlib_path;
    std::string file_name;
};

} // namespace ax