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

    std::optional<std::shared_ptr<Type>> find(std::string const &name);

  private:
    SymbolTable<std::shared_ptr<Type>> table;
};

} // namespace ax