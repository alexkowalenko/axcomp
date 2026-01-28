//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include "type.hh"

namespace ax {

class SimpleType : public Type_ {
  public:
    explicit SimpleType(std::string n, const TypeId id) : Type_(id), name(std::move(n)) {};
    ~SimpleType() override = default;

    explicit    operator std::string() override;
    std::string name;
};

class IntegerType : public SimpleType {
  public:
    explicit IntegerType() : SimpleType("INTEGER", TypeId::INTEGER) {};
    ~IntegerType() override = default;

    [[nodiscard]] bool is_numeric() const override { return true; };

    [[nodiscard]] llvm::Constant *make_value(Int i) const;
    [[nodiscard]] size_t          get_size() const override {
        return llvm::dyn_cast<llvm::IntegerType>(get_llvm())->getBitWidth() / CHAR_BIT;
    }

    [[nodiscard]] llvm::Value *min() const override { return make_value(INT64_MIN); };
    [[nodiscard]] llvm::Value *max() const override { return make_value(INT64_MAX); };
};

class BooleanType : public SimpleType {
  public:
    explicit BooleanType() : SimpleType("BOOLEAN", TypeId::BOOLEAN) {};
    ~BooleanType() override = default;

    [[nodiscard]] llvm::Constant *make_value(Bool b) const;
    [[nodiscard]] size_t          get_size() const override {
        return 1; // llvm::dyn_cast<llvm::IntegerType>(get_llvm())->getBitWidth() / CHAR_BIT;
    }

    [[nodiscard]] llvm::Value *min() const override { return make_value(false); };
    [[nodiscard]] llvm::Value *max() const override { return make_value(true); };
};

class RealCType : public SimpleType {
  public:
    explicit RealCType() : SimpleType("REAL", TypeId::REAL) {};
    ~RealCType() override = default;

    [[nodiscard]] llvm::Constant *make_value(const Real f) const {
        return llvm::ConstantFP::get(get_llvm(), f);
    }
    [[nodiscard]] size_t get_size() const override { return sizeof(Real); } // 64-bit floats;

    [[nodiscard]] llvm::Value *min() const override { return make_value(DBL_MIN); };
    [[nodiscard]] llvm::Value *max() const override { return make_value(DBL_MAX); };
};

class CharacterType : public SimpleType {
  public:
    explicit CharacterType() : SimpleType("CHAR", TypeId::CHAR) {};
    ~CharacterType() override = default;

    [[nodiscard]] llvm::Constant *make_value(Char c) const;
    [[nodiscard]] size_t          get_size() const override {
        return llvm::dyn_cast<llvm::IntegerType>(get_llvm())->getBitWidth() / CHAR_BIT;
    }

    [[nodiscard]] llvm::Value *min() const override { return make_value(WCHAR_MIN); };
    [[nodiscard]] llvm::Value *max() const override { return make_value(WCHAR_MAX); };
};

class StringType : public SimpleType {
  public:
    StringType() : SimpleType("STRING", TypeId::STRING) {};
    ~StringType() override = default;

    static llvm::Constant *make_value(std::string const &s);
    static llvm::Type     *make_type(std::string const &s);
    static llvm::Type     *make_type_ptr();
};

} // namespace ax
