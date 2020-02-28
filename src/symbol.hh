//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <memory.h>
#include <string>

#include <fmt/core.h>

namespace ax {

class Symbol {
  public:
    Symbol(std::string const &n, std::string const &t) : name(n), type(t){};

    Symbol(const Symbol &s) = default;
    Symbol() = default;

    explicit operator std::string() const {
        return fmt::format("({} : {})", name, type);
    };

    std::string name;
    std::string type;
};

} // namespace ax