//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "location.hh"

#include <format>

namespace ax {

Location::operator std::string() const {
    return std::format("{0},{1}", lineno, charpos);
}

}; // namespace ax