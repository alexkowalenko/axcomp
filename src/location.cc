//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "location.hh"

#include <fmt/core.h>

namespace ax {

Location::operator std::string() {
    return fmt::format("{},{}", lineno, charpos);
}

}; // namespace ax