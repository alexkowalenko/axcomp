//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <optional>
#include <unordered_map>
#include <vector>

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

class EnumType : public SimpleType {
  public:
    explicit EnumType(std::string n) : SimpleType(std::move(n), TypeId::ENUMERATION) {};
    ~EnumType() override = default;

    void add_value(std::string name) {
        const auto ordinal = static_cast<Int>(values.size());
        values.push_back(name);
        ordinals.emplace(std::move(name), ordinal);
    }

    std::optional<Int> get_ordinal(std::string const &name) const {
        const auto it = ordinals.find(name);
        if (it == ordinals.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    [[nodiscard]] llvm::Constant *make_value(const Int i) const {
        return llvm::ConstantInt::get(get_llvm(), static_cast<uint64_t>(i));
    }

    [[nodiscard]] llvm::Value *min() const override {
        if (values.empty()) {
            return make_value(0);
        }
        return make_value(0);
    }

    [[nodiscard]] llvm::Value *max() const override {
        if (values.empty()) {
            return make_value(0);
        }
        return make_value(static_cast<Int>(values.size() - 1));
    }

    std::vector<std::string> values;

  private:
    std::unordered_map<std::string, Int> ordinals;
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
