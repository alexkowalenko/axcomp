//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include "type.hh"

namespace ax {

class ArrayType : public Type_ {
  public:
    explicit ArrayType(Type b) : Type_(TypeId::ARRAY), base_type(std::move(b)) {};
    ~ArrayType() override = default;

    explicit operator std::string() override;

    [[nodiscard]] bool is_assignable() const override { return false; };

    [[nodiscard]] llvm::Type     *get_llvm() const override;
    [[nodiscard]] llvm::Constant *get_init() const override;

    [[nodiscard]] size_t get_size() const override {
        return std::accumulate(begin(dimensions), end(dimensions), static_cast<size_t>(1),
                               std::multiplies<>()) *
               base_type->get_size();
    }

    Type                base_type;
    std::vector<size_t> dimensions;
};

class OpenArrayType : public ArrayType {
  public:
    explicit OpenArrayType(Type b) : ArrayType(std::move(b)) { id = TypeId::OPENARRAY; };
    ~OpenArrayType() override = default;

    explicit operator std::string() override;

    [[nodiscard]] bool is_assignable() const override { return true; };

    [[nodiscard]] llvm::Type     *get_llvm() const override;
    [[nodiscard]] llvm::Constant *get_init() const override;
};

} // namespace ax
