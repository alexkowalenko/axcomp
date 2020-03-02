//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "type.hh"

#include <fmt/core.h>

namespace ax {

std::string to_string(SimpleTypeTag t) {
    switch (t) {
    case SimpleTypeTag::void_t:
        return "void";
    case SimpleTypeTag::integer:
        return "INTEGER";
    case SimpleTypeTag::module:
        return "MODULE";
    case SimpleTypeTag::procedure:
        return "PROCEDURE";
    default:
        return "unknown type";
    }
}

SimpleType::operator std::string() {
    return to_string(type);
}

ProcedureType::operator std::string() {
    return fmt::format("{}()", SimpleTypeTag::procedure);
}

} // namespace ax
