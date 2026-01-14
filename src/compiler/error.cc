//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <format>
#include <iostream>

#include "error.hh"

namespace ax {

std::string AXException::error_msg() const {
    return std::string(std::format("{0}: {1}", std::string(location), msg));
}

void ErrorManager::print_errors(std::ostream &out) const {
    for (const auto &e : error_list) {
        out << e.error_msg() << std::endl;
    };
}

} // namespace ax