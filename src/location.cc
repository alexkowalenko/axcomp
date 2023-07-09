//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "location.hh"

#include <fmt/core.h>

namespace ax {

Location::operator std::string() const {
    return fmt::format("{0},{1}", lineno, charpos);
}

}; // namespace ax