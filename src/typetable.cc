//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko

#include "typetable.hh"

namespace ax {

void TypeTable::initialise() {
    auto t = std::make_shared<SimpleType>(SimpleTypeTag::integer);
    table.put(to_string(t->type), t);

    t = std::make_shared<SimpleType>(SimpleTypeTag::module);
    table.put(to_string(t->type), t);
}

std::optional<std::shared_ptr<Type>> TypeTable::find(std::string const &name) {
    return table.find(name);
}

} // namespace ax