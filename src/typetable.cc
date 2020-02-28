//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko

#include "typetable.hh"

namespace ax {

void TypeTable::initialise() {
    auto t = std::make_shared<SimpleType>("INTEGER");
    table.put(t->name, t);

    auto tt = std::make_shared<ModuleType>("MODULE");
    table.put(tt->name, tt);
}

std::optional<std::shared_ptr<Type>> TypeTable::find(std::string const &name) {
    return table.find(name);
}

} // namespace ax