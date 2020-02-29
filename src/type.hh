//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <string>

namespace ax {
class Type {
  public:
    explicit Type(std::string n) : name(std::move(n)){};
    virtual ~Type() = default;

    std::string name;
};

class SimpleType : public Type {
  public:
    explicit SimpleType(std::string n) : Type(std::move(n)){};
    ~SimpleType() override = default;
};

class ModuleType : public Type {
  public:
    explicit ModuleType(std::string n) : Type(std::move(n)){};
    ~ModuleType() override = default;
};

class FunctionType : public Type {
  public:
    explicit FunctionType(std::string n) : Type(std::move(n)){};
    ~FunctionType() override = default;
};

} // namespace ax
