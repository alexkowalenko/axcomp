//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko

#include "typetable.hh"

namespace ax {

std::string string(SimpleTypeTag &t) {
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

void TypeTable::initialise() {
    auto t = std::make_shared<SimpleType>(SimpleTypeTag::integer);
    table.put(string(t->type), t);

    t = std::make_shared<SimpleType>(SimpleTypeTag::module);
    table.put(string(t->type), t);
}

std::optional<std::shared_ptr<Type>> TypeTable::find(std::string const &name) {
    return table.find(name);
}

} // namespace ax