//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <llvm/IR/DerivedTypes.h>

namespace ax {

class Type;
using TypePtr = std::shared_ptr<Type>;

class Type {
  public:
    Type() : llvm_type(nullptr){};
    virtual ~Type() = default;

    bool equiv(TypePtr const &t);

    virtual explicit operator std::string() = 0;

    void        set_llvm(llvm::Type *t) { llvm_type = t; };
    llvm::Type *get_llvm() { return llvm_type; };

  private:
    llvm::Type *llvm_type;
};

class SimpleType : public Type {
  public:
    explicit SimpleType(std::string n) : name(std::move(n)){};
    ~SimpleType() override{};

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