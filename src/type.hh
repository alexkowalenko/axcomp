//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <cfloat>
#include <climits>
#include <cstddef>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

#include <llvm/ADT/StringMap.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>

#include "astvisitor.hh"
#include "attr.hh"
#include "ax.hh"

namespace ax {

class Type;
using TypePtr = std::shared_ptr<Type>;

enum class TypeId {
    null, // void
    any,  // any - used to for multi type return values like MIN, MAX
    integer,
    real,
    boolean,
    chr,
    procedure,
    procedureFwd,
    array,
    string,
    str1, // One char STRING which can be a CHAR
    set,
    record,
    alias,
    pointer,
    module
};

std::string string(TypeId t);

inline bool is_referencable(TypeId &id) {
    return !(id == TypeId::procedure || id == TypeId::alias || id == TypeId::module);
}

class Type {
  public:
    explicit Type(TypeId i) : id(i){};
    virtual ~Type() = default;

    TypeId id = TypeId::null;

    [[nodiscard]] bool equiv(TypePtr const &t);

    virtual explicit operator std::string() = 0;

    std::string get_name() { return std::string(*this); }

    virtual bool is_numeric() { return false; };

    void                set_llvm(llvm::Type *t) { llvm_type = t; };
    virtual llvm::Type *get_llvm() { return llvm_type; };

    void                    set_init(llvm::Constant *t) { llvm_init = t; };
    virtual llvm::Constant *get_init() { return llvm_init; };

    virtual size_t get_size() { return 0; };

    virtual llvm::Value *min();
    virtual llvm::Value *max();

  private:
    llvm::Type *    llvm_type{nullptr};
    llvm::Constant *llvm_init{nullptr};
};

class SimpleType : public Type {
  public:
    explicit SimpleType(std::string n, TypeId id) : Type(id), name(std::move(n)){};
    ~SimpleType() override = default;

    explicit    operator std::string() override;
    std::string name;
};

class IntegerType : public SimpleType {
  public:
    explicit IntegerType() : SimpleType("INTEGER", TypeId::integer){};
    ~IntegerType() override = default;

    bool is_numeric() override { return true; };

    llvm::Constant *make_value(Int i);
    size_t          get_size() override {
        return llvm::dyn_cast<llvm::IntegerType>(get_llvm())->getBitWidth() / CHAR_BIT;
    }

    llvm::Value *min() override { return make_value(INT64_MIN); };
    llvm::Value *max() override { return make_value(INT64_MAX); };
};

class BooleanType : public SimpleType {
  public:
    explicit BooleanType() : SimpleType("BOOLEAN", TypeId::boolean){};
    ~BooleanType() override = default;

    llvm::Constant *make_value(Bool b);
    size_t          get_size() override {
        return 1; // llvm::dyn_cast<llvm::IntegerType>(get_llvm())->getBitWidth() / CHAR_BIT;
    }

    llvm::Value *min() override { return make_value(false); };
    llvm::Value *max() override { return make_value(true); };
};

class RealCType : public SimpleType {
  public:
    explicit RealCType() : SimpleType("REAL", TypeId::real){};
    ~RealCType() override = default;

    llvm::Constant *make_value(Real f) { return llvm::ConstantFP::get(get_llvm(), f); }
    size_t          get_size() override { return sizeof(Real); } // 64 bit floats;

    llvm::Value *min() override { return make_value(DBL_MIN); };
    llvm::Value *max() override { return make_value(DBL_MAX); };
};

class CharacterType : public SimpleType {
  public:
    explicit CharacterType() : SimpleType("CHAR", TypeId::chr){};
    ~CharacterType() override = default;

    llvm::Constant *make_value(Char c);
    size_t          get_size() override {
        return llvm::dyn_cast<llvm::IntegerType>(get_llvm())->getBitWidth() / CHAR_BIT;
    }

    llvm::Value *min() override { return make_value(WCHAR_MIN); };
    llvm::Value *max() override { return make_value(WCHAR_MAX); };
};

class ProcedureType : public Type {
  public:
    explicit ProcedureType() : Type(TypeId::procedure){};
    ProcedureType(TypePtr returns, std::vector<std::pair<TypePtr, Attr>> params)
        : Type(TypeId::procedure), ret(std::move(returns)), params(std::move(params)){};
    ~ProcedureType() override = default;

    explicit operator std::string() override;

    llvm::Type *get_llvm() override;

    TypePtr get_closure_struct() const;

    TypePtr ret{nullptr};
    using ParamsList = std::vector<std::pair<TypePtr, Attr>>;
    ParamsList params{};
    using FreeList = std::vector<std::pair<std::string, TypePtr>>;
    FreeList free_vars{};

  protected:
    std::string get_print(bool forward);
};

class ProcedureFwdType : public ProcedureType {
  public:
    explicit ProcedureFwdType() { id = TypeId::procedureFwd; };
    ~ProcedureFwdType() override = default;

    explicit operator std::string() override;
};

class ArrayType : public Type {
  public:
    explicit ArrayType(TypePtr b) : Type(TypeId::array), base_type(std::move(b)){};
    ~ArrayType() override = default;

    explicit operator std::string() override;

    llvm::Type *    get_llvm() override;
    llvm::Constant *get_init() override;

    size_t get_size() override { return dimensions[0] * base_type->get_size(); }

    TypePtr             base_type;
    std::vector<size_t> dimensions;
};

class StringType : public SimpleType {
  public:
    StringType() : SimpleType("STRING", TypeId::string){};
    ~StringType() override = default;

    llvm::Constant *make_value(std::string const &s);
    llvm::Type *    make_type(std::string const &s); // Type dependant on string size
    llvm::Type *    make_type_ptr();                 // Zero size
};

class RecordType : public Type {
  public:
    RecordType() : Type(TypeId::record){};
    ~RecordType() override = default;

    explicit operator std::string() override;

    std::vector<llvm::Type *>     get_fieldTypes();
    std::vector<llvm::Constant *> get_fieldInit();

    llvm::Type *    get_llvm() override;
    llvm::Constant *get_init() override;

    void   insert(std::string const &field, TypePtr type);
    bool   has_field(std::string const &field);
    size_t count() { return index.size(); };

    void                        set_baseType(std::shared_ptr<RecordType> const &b) { base = b; };
    std::shared_ptr<RecordType> baseType() { return base; };
    bool                        is_base(TypePtr t);

    std::optional<TypePtr> get_type(std::string const &field);
    int                    get_index(std::string const &field);

    size_t get_size() override;

    void        set_identified(std::string const &s) { identified = s; };
    std::string get_identified() { return identified; };

    bool equiv(std::shared_ptr<RecordType> r);

  private:
    std::string                 identified{}; // identified records
    std::shared_ptr<RecordType> base{nullptr};
    llvm::StringMap<TypePtr>    fields;
    std::vector<std::string>    index;

    llvm::Type *cache{nullptr};
};

class TypeAlias : public Type {
  public:
    TypeAlias(std::string n, TypePtr t)
        : Type(TypeId::alias), name{std::move(n)}, alias{std::move(t)} {};
    ~TypeAlias() override = default;

    explicit operator std::string() override { return name; };
    size_t   get_size() override { return alias->get_size(); };

    TypePtr get_alias() { return alias; }

  private:
    std::string name;
    TypePtr     alias;
};

class PointerType : public Type {
  public:
    explicit PointerType(std::string r) : Type(TypeId::pointer), ref_name{std::move(r)} {};
    explicit PointerType(TypePtr const &r) : Type(TypeId::pointer), reference{r} {
        ref_name = r->get_name();
    };

    ~PointerType() override = default;

    explicit operator std::string() override { return '^' + ref_name; };
    size_t   get_size() override {
        return reference->get_llvm()->getPointerTo()->getPrimitiveSizeInBits() / CHAR_BIT;
    };

    llvm::Type *    get_llvm() override;
    llvm::Constant *get_init() override;

    TypePtr get_reference() { return reference; }
    void    set_reference(TypePtr &r) { reference = r; }

    std::string &get_ref_name() { return ref_name; };

  private:
    TypePtr     reference = nullptr;
    std::string ref_name;
};

class SetCType : public SimpleType {
  public:
    SetCType() : SimpleType("SET", TypeId::set){};
    ~SetCType() override = default;

    llvm::Type *    get_llvm() override;
    llvm::Constant *get_init() override;

    llvm::Value *min() override;
    llvm::Value *max() override;
};

class ModuleType : public Type {
  public:
    explicit ModuleType(std::string n) : Type(TypeId::module), name{std::move(n)} {};
    ~ModuleType() override = default;

    explicit operator std::string() override { return "MODULE: " + name; };

    std::string &module_name() { return name; };

  private:
    std::string name;
};

} // namespace ax
