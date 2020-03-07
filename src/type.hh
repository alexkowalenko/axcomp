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

class Type {
  public:
    virtual ~Type() = default;

    bool equiv(std::shared_ptr<Type> t);

    virtual explicit operator std::string() = 0;
};

class SimpleType : public Type {
  public:
    explicit SimpleType(std::string n) : name(n){};
    ~SimpleType() override = default;

    explicit operator std::string() override;

    std::string name;
};

class ProcedureType : public Type {
  public:
    ProcedureType(std::shared_ptr<Type> const &             r,
                  std::vector<std::shared_ptr<Type>> const &p)
        : ret(r), params(p){};
    ~ProcedureType() override = default;

    explicit operator std::string() override;

    std::shared_ptr<Type>              ret;
    std::vector<std::shared_ptr<Type>> params;
};

} // namespace ax
