//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <map>
#include <memory>

#include <llvm/IR/LLVMContext.h>

#include "symboltable.hh"
#include "token.hh"
#include "types/all.hh"

namespace ax {

struct TypeRule1 {
    Type value;
    Type result;
};

struct TypeRule2 {
    Type L;
    Type R;
    Type result;
};

class TypeTable : public SymbolTable<Type> {
  public:
    TypeTable() : SymbolTable(nullptr) {};

    void        initialise();
    static void setTypes(llvm::LLVMContext &context);

    Type resolve(std::string const &name) const;

    /**
     * @brief Check one argument operator with a type
     *
     * @param op
     * @param type
     * @return true - operator accepts the this type
     * @return false
     */
    Type check(TokenType op, Type const &type);

    /**
     * @brief  Check two argument operator with types
     *
     * @param op
     * @param Lt - Left type
     * @param Rt - Right type
     * @return true - operator accepts the this type
     * @return false
     */
    Type check(TokenType op, Type const &Lt, Type const &Rt);

    static bool is_int_instruct(const llvm::Type *t) {
        return t == IntType->get_llvm() || t == BoolType->get_llvm() || t == CharType->get_llvm();
    }

    // Standard types
    inline static std::shared_ptr<IntegerType>   IntType;
    inline static std::shared_ptr<BooleanType>   BoolType;
    inline static std::shared_ptr<RealCType>     RealType;
    inline static std::shared_ptr<CharacterType> CharType;
    inline static std::shared_ptr<StringType>    StrType;
    inline static std::shared_ptr<StringType>    Str1Type;
    inline static std::shared_ptr<SetCType>      SetType;

    inline static Type VoidType; // For procedures which don't return anything, also arguments
                                 // which can any type (internal for built-ins)
    inline static Type AnyType;  // For procedures which can return any time (built-ins), also
                                 // for non-fixed length types arguments (built-ins).

    static TypeTable *sgl() { return singleton; };

  private:
    // Set singleton
    static void set_singleton(TypeTable *s) { singleton = s; };

    void set_type_alias(char const *name, Type const &t);

    void reg(TokenType op, Type const &type, Type const &result);
    void reg(TokenType op, Type const &L, Type const &R, Type const &result);

    std::multimap<TokenType, TypeRule1> rules1;
    std::multimap<TokenType, TypeRule2> rules2;

    inline static TypeTable *singleton;
};

} // namespace ax