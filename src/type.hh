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
    Type(std::string n) : name(n){};
    virtual ~Type() = default;

    std::string name;
};

class SimpleType : public Type {
  public:
    SimpleType(std::string n) : Type(n){};
    ~SimpleType() override = default;
};

class ModuleType : public Type {
  public:
    ModuleType(std::string n) : Type(n){};
    ~ModuleType() override = default;
};

class FunctionType : public Type {
  public:
    FunctionType(std::string n) : Type(n){};
    ~FunctionType() override = default;
};

} // namespace ax
