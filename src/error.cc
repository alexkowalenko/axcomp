//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "error.hh"

namespace ax {

std::string AXException::error_msg() {
    std::string err("Error: ");
    err += msg;
    err += " in line: ";
    err += std::to_string(lineno);
    return err;
}

} // namespace ax