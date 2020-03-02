//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <string>

namespace ax {

enum class SimpleTypeTag { void_t, integer, module, procedure };

std::string to_string(SimpleTypeTag t);

class Type {
  public:
    virtual ~Type() = default;

    virtual operator std::string() = 0;
};

class SimpleType : public Type {
  public:
    explicit SimpleType(SimpleTypeTag n) : type(n){};
    ~SimpleType(){};

    operator std::string() override;

    SimpleTypeTag type;
};

class ProcedureType : public Type {
  public:
    explicit ProcedureType(std::string n){};
    ~ProcedureType(){};

    operator std::string();
};

} // namespace ax
