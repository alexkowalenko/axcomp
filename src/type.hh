//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>

namespace ax {

class Type;
using TypePtr = std::shared_ptr<Type>;

class Type {
  public:
    Type() = default;
    virtual ~Type() = default;

    bool equiv(TypePtr const &t);

    virtual explicit operator std::string() = 0;

    std::string get_name() { return std::string(*this); }

    virtual bool is_numeric() { return false; };

    void                set_llvm(llvm::Type *t) { llvm_type = t; };
    virtual llvm::Type *get_llvm() { return llvm_type; };

    void                    set_init(llvm::Constant *t) { llvm_init = t; };
    virtual llvm::Constant *get_init() { return llvm_init; };

  private:
    llvm::Type *    llvm_type{nullptr};
    llvm::Constant *llvm_init{nullptr};
};

class SimpleType : public Type {
  public:
    explicit SimpleType(std::string n) : name(std::move(n)){};
    ~SimpleType() override = default;

    explicit operator std::string() override;

    std::string name;
};

class IntegerType : public SimpleType {
  public:
    explicit IntegerType() : SimpleType("INTEGER"){};
    ~IntegerType() override = default;

    bool is_numeric() override { return true; };

    llvm::Constant *make_value(long i);
};

class BooleanType : public SimpleType {
  public:
    explicit BooleanType() : SimpleType("BOOLEAN"){};
    ~BooleanType() override = default;

    llvm::Constant *make_value(bool b);
};

class ProcedureType : public Type {
  public:
    ProcedureType(TypePtr returns, std::vector<TypePtr> params)
        : ret(std::move(returns)), params(std::move(params)){};
    ~ProcedureType() override = default;

    explicit operator std::string() override;

    TypePtr              ret{nullptr};
    std::vector<TypePtr> params{};
};

class ArrayType : public Type {
  public:
    ArrayType(TypePtr b, long s) : base_type(std::move(b)), size(s){};
    ~ArrayType() override = default;

    explicit operator std::string() override;

    llvm::Type *    get_llvm() override;
    llvm::Constant *get_init() override;

    TypePtr base_type;
    long    size;
};

class RecordType : public Type {
  public:
    RecordType() = default;
    ~RecordType() override = default;

    explicit operator std::string() override;

    llvm::Type *    get_llvm() override;
    llvm::Constant *get_init() override;

    std::vector<TypePtr> fields;
};

} // namespace ax
