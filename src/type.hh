//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>

#include "attr.hh"

namespace ax {

class Type;
using TypePtr = std::shared_ptr<Type>;

enum class TypeId {
    null, // void
    integer,
    boolean,
    procedure,
    array,
    record,
    alias,
    module
};

inline bool is_referencable(TypeId &id) {
    return !(id == TypeId::procedure || id == TypeId::alias ||
             id == TypeId::module);
}

class Type {
  public:
    explicit Type(TypeId i) : id(i){};
    virtual ~Type() = default;

    TypeId id = TypeId::null;

    bool equiv(TypePtr const &t) const;

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
    explicit SimpleType(std::string n, TypeId id)
        : Type(id), name(std::move(n)){};
    ~SimpleType() override = default;

    explicit operator std::string() override;

    std::string name;
};

class IntegerType : public SimpleType {
  public:
    explicit IntegerType() : SimpleType("INTEGER", TypeId::integer){};
    ~IntegerType() override = default;

    bool is_numeric() override { return true; };

    llvm::Constant *make_value(long i);
};

class BooleanType : public SimpleType {
  public:
    explicit BooleanType() : SimpleType("BOOLEAN", TypeId::boolean){};
    ~BooleanType() override = default;

    llvm::Constant *make_value(bool b);
};

class ProcedureType : public Type {
  public:
    explicit ProcedureType() : Type(TypeId::procedure){};
    ProcedureType(TypePtr returns, std::vector<std::pair<TypePtr, Attr>> params)
        : Type(TypeId::procedure), ret(std::move(returns)),
          params(std::move(params)){};
    ~ProcedureType() override = default;

    explicit operator std::string() override;

    llvm::Type *get_llvm() override;

    TypePtr ret{nullptr};
    using ParamsList = std::vector<std::pair<TypePtr, Attr>>;
    ParamsList params{};
};

class ArrayType : public Type {
  public:
    ArrayType(TypePtr b, long s)
        : Type(TypeId::array), base_type(std::move(b)), size(s){};
    ~ArrayType() override = default;

    explicit operator std::string() override;

    llvm::Type *    get_llvm() override;
    llvm::Constant *get_init() override;

    TypePtr base_type;
    long    size;
};

class RecordType : public Type {
  public:
    RecordType() : Type(TypeId::record){};
    ~RecordType() override = default;

    explicit operator std::string() override;

    llvm::Type *    get_llvm() override;
    llvm::Constant *get_init() override;

    void                   insert(std::string const &field, TypePtr type);
    bool                   has_field(std::string const &field);
    std::optional<TypePtr> get_type(std::string const &field);
    int                    get_index(std::string const &field);

  private:
    std::map<std::string, TypePtr> fields;
    std::vector<std::string>       index;
};

class TypeAlias : public Type {
  public:
    TypeAlias(std::string n, TypePtr t)
        : Type(TypeId::alias), name{std::move(n)}, alias{std::move(t)} {};
    ~TypeAlias() override = default;

    explicit operator std::string() override { return name; };

    TypePtr get_alias() { return alias; }

  private:
    std::string name;
    TypePtr     alias;
};

class ModuleType : public Type {
  public:
    explicit ModuleType(std::string n)
        : Type(TypeId::module), name{std::move(n)} {};
    ~ModuleType() override = default;

    explicit operator std::string() override { return "MODULE: " + name; };

  private:
    std::string name;
};

} // namespace ax
