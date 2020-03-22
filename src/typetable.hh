//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <memory>
#include <optional>

#include <llvm/IR/LLVMContext.h>

#include "astmod.hh"
#include "symboltable.hh"
#include "type.hh"

namespace ax {

class TypeTable {
  public:
    TypeTable() : table(nullptr){};

    void initialise();
    void setTypes(llvm::LLVMContext &context);

    std::optional<TypePtr> find(std::string name);
    void                   put(std::string const &name, TypePtr const &t);

    // Standard types
    static TypePtr IntType;
    static TypePtr BoolType;
    static TypePtr ModuleType;
    static TypePtr VoidType;

  private:
    SymbolTable<TypePtr> table;
};

} // namespace ax