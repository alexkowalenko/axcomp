//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "type.hh"

#include <fmt/core.h>

namespace ax {

SimpleType::operator std::string() {
    return string(type);
}

ProcedureType::operator std::string() {
    return fmt::format("{}()", SimpleTypeTag::procedure);
}

} // namespace ax
