//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <fmt/core.h>

#include "error.hh"

namespace ax {

std::string AXException::error_msg() {
    return fmt::format("{}: {}", lineno, msg);
}

} // namespace ax