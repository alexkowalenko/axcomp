//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko

#include "typetable.hh"

namespace ax {

TypePtr TypeTable::IntType;
TypePtr TypeTable::ModuleType;
TypePtr TypeTable::VoidType;

void TypeTable::initialise() {
    IntType = std::make_shared<SimpleType>("INTEGER");
    table.put(std::string(*IntType), IntType);

    ModuleType = std::make_shared<SimpleType>("MODULE");
    table.put(std::string(*ModuleType), ModuleType);

    VoidType = std::make_shared<SimpleType>("void");
    table.put(std::string(*VoidType), VoidType);
}

std::optional<TypePtr> TypeTable::find(std::string const &name) {
    return table.find(name);
}

void TypeTable::put(std::string const &name, TypePtr const &t) {
    table.put(name, t);
}

} // namespace ax