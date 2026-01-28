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

    Type get_alias() { return alias; }

  private:
    std::string name;
    Type        alias;
};

} // namespace ax
