//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include "type.hh"

namespace ax {

class ModuleType : public Type_ {
  public:
    explicit ModuleType(std::string n) : Type_(TypeId::MODULE), name{std::move(n)} {};
    ~ModuleType() override = default;

    explicit operator std::string() override { return "MODULE: " + name; };

    [[nodiscard]] bool is_assignable() const override { return false; };

    std::string &module_name() { return name; };

  private:
    std::string name;
};

} // namespace ax
