//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <memory>
#include <string>
#include <vector>

namespace ax {

class Type;
using TypePtr = std::shared_ptr<Type>;

class Type {
  public:
    virtual ~Type() = default;

    bool equiv(TypePtr const &t);

    virtual explicit operator std::string() = 0;
};

class SimpleType : public Type {
  public:
    explicit SimpleType(std::string n) : name(std::move(n)){};
    ~SimpleType() override = default;

    explicit operator std::string() override;

    std::string name;
};

class ProcedureType : public Type {
  public:
    ProcedureType(TypePtr r, std::vector<TypePtr> p)
        : ret(std::move(r)), params(std::move(p)){};
    ~ProcedureType() override = default;

    explicit operator std::string() override;

    TypePtr              ret;
    std::vector<TypePtr> params;
};

} // namespace ax
