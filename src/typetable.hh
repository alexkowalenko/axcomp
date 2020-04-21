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

class TypeTable : public SymbolTable<TypePtr> {
  public:
    TypeTable() : SymbolTable(nullptr){};

    void        initialise();
    static void setTypes(llvm::LLVMContext &context);

    std::optional<TypePtr> resolve(std::string const &name);

    // Standard types
    static std::shared_ptr<IntegerType>   IntType;
    static std::shared_ptr<BooleanType>   BoolType;
    static std::shared_ptr<CharacterType> CharType;
    static std::shared_ptr<StringType>    StrType;
    static TypePtr                        VoidType;
    static TypePtr                        AnyType;
};

} // namespace ax