//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <memory>
#include <optional>

#include <llvm/IR/LLVMContext.h>

#include "ast.hh"
#include "symboltable.hh"
#include "type.hh"

namespace ax {

class TypeTable {
  public:
    TypeTable() : table(nullptr){};

    void        initialise();
    static void setTypes(llvm::LLVMContext &context);

    std::optional<TypePtr> find(std::string const &name);
    std::optional<TypePtr> resolve(std::string const &name);
    void                   put(std::string const &name, TypePtr const &t);

    // Standard types
    static std::shared_ptr<IntegerType> IntType;
    static std::shared_ptr<BooleanType> BoolType;
    static TypePtr                      ModuleType;
    static TypePtr                      VoidType;

  private:
    SymbolTable<TypePtr> table;
};

} // namespace ax