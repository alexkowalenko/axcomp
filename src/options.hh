//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <string>

namespace ax {

struct Options {
    bool debug_parse = false;

    bool        main_module = false;
    std::string file_name;
};

} // namespace ax