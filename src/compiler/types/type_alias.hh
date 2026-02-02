//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include "type.hh"

namespace ax {

class TypeAlias : public Type_ {
  public:
    TypeAlias(std::string n, Type t)
        : Type_(TypeId::ALIAS), name{std::move(n)}, alias{std::move(t)} {};
    ~TypeAlias() override = default;

    explicit             operator std::string() override { return name; };
    [[nodiscard]] size_t get_size() const override { return alias->get_size(); };
    [[nodiscard]] bool   is_numeric() const override { return alias->is_numeric(); };
    [[nodiscard]] bool   is_assignable() const override { return alias->is_assignable(); };

    [[nodiscard]] llvm::Type     *get_llvm() const override { return alias->get_llvm(); };
    [[nodiscard]] llvm::Constant *get_init() const override { return alias->get_init(); };
    [[nodiscard]] llvm::Value    *min() const override { return alias->min(); };
    [[nodiscard]] llvm::Value    *max() const override { return alias->max(); };

    Type get_alias() { return alias; }

  private:
    std::string name;
    Type        alias;
};

} // namespace ax
