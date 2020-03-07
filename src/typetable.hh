//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <memory>
#include <optional>

#include "symboltable.hh"
#include "type.hh"

namespace ax {

class TypeTable {
  public:
    TypeTable() : table(nullptr){};

    void initialise();

    std::optional<TypePtr> find(std::string const &name);
    void                   put(std::string const &name, TypePtr const &t);

    // Standard types
    static TypePtr IntType;
    static TypePtr ModuleType;
    static TypePtr VoidType;

  private:
    SymbolTable<TypePtr> table;
};

} // namespace ax