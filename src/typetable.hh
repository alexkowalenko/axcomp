//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <map>
#include <memory>
#include <optional>
#include <utility>

#include <llvm/IR/LLVMContext.h>

#include "ast.hh"
#include "symboltable.hh"
#include "token.hh"
#include "type.hh"

namespace ax {

struct TypeRule1 {
    TypePtr value;
    TypePtr result;
};

struct TypeRule2 {
    TypePtr L;
    TypePtr R;
    TypePtr result;
};

class TypeTable : public SymbolTable<TypePtr> {
  public:
    TypeTable() : SymbolTable(nullptr){};

    void        initialise();
    static void setTypes(llvm::LLVMContext &context);

    std::optional<TypePtr> resolve(std::string const &name);

    /**
     * @brief Check one argument operator with a type
     *
     * @param op
     * @param type
     * @return true - operator accepts the this type
     * @return false
     */
    std::optional<TypePtr> check(TokenType op, TypePtr const &type);

    /**
     * @brief  Check two argument operator with types
     *
     * @param op
     * @param L
     * @param R
     * @return true - operator accepts the this type
     * @return false
     */

    std::optional<TypePtr> check(TokenType op, TypePtr const &L, TypePtr const &R);

    // Standard types
    static std::shared_ptr<IntegerType>   IntType;
    static std::shared_ptr<BooleanType>   BoolType;
    static std::shared_ptr<RealCType>     RealType;
    static std::shared_ptr<CharacterType> CharType;
    static std::shared_ptr<StringType>    StrType;
    static TypePtr                        VoidType;
    static TypePtr                        AnyType;

  private:
    void reg(TokenType op, TypePtr const &type, TypePtr const &result);
    void reg(TokenType op, TypePtr const &L, TypePtr const &R, TypePtr const &result);

    std::multimap<TokenType, TypeRule1> rules1;
    std::multimap<TokenType, TypeRule2> rules2;
};

} // namespace ax