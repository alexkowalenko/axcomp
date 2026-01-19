//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <cfloat>
#include <climits>
#include <cstddef>
#include <functional>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshadow"
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wunused-parameter"
#include <llvm/ADT/StringMap.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#pragma clang diagnostic pop

#include "astvisitor.hh"
#include "attr.hh"
#include "ax.hh"

namespace ax {

enum class TypeId : std::uint8_t {
    null, // void
    any,  // any - used to for multi type return values like MIN, MAX
    integer,
    real,
    boolean,
    chr,
    procedure,
    procedureFwd,
    array,
    openarray,
    string,
    str1, // One char STRING which can be a CHAR
    set,
    record,
    alias,
    pointer,
    module
};

std::string const &string(TypeId t);

class Type_;
using Type = std::shared_ptr<Type_>;

class Type_ {
  public:
    explicit Type_(TypeId i) : id(i) {};
    virtual ~Type_() = default;

    TypeId id = TypeId::null;

    [[nodiscard]] bool equiv(Type const &t);

    virtual explicit operator std::string() = 0;

    std::string get_name() { return std::string(*this); }

    [[nodiscard]] virtual bool   is_numeric() const { return false; };
    [[nodiscard]] constexpr bool is_array() const {
        return id == TypeId::array || id == TypeId::openarray;
    };
    [[nodiscard]] virtual bool is_assignable() const { return true; };
    [[nodiscard]] bool constexpr is_referencable() const {
        return !(id == TypeId::procedure || id == TypeId::alias || id == TypeId::module);
    }

    void                              set_llvm(llvm::Type *t) { llvm_type = t; };
    [[nodiscard]] virtual llvm::Type *get_llvm() const { return llvm_type; };

    void                                  set_init(llvm::Constant *t) { llvm_init = t; };
    [[nodiscard]] virtual llvm::Constant *get_init() const { return llvm_init; };

    [[nodiscard]] virtual size_t get_size() const { return 0; };

    [[nodiscard]] virtual llvm::Value *min() const;
    [[nodiscard]] virtual llvm::Value *max() const;

  private:
    llvm::Type     *llvm_type{nullptr};
    llvm::Constant *llvm_init{nullptr};
};

inline std::string string(Type const &t) {
    return std::string(*t);
}

class SimpleType : public Type_ {
  public:
    explicit SimpleType(std::string n, TypeId id) : Type_(id), name(std::move(n)) {};
    ~SimpleType() override = default;

    explicit    operator std::string() override;
    std::string name;
};

class IntegerType : public SimpleType {
  public:
    explicit IntegerType() : SimpleType("INTEGER", TypeId::integer) {};
    ~IntegerType() override = default;

    [[nodiscard]] bool is_numeric() const override { return true; };

    [[nodiscard]] llvm::Constant *make_value(Int i) const;
    [[nodiscard]] size_t          get_size() const override {
        return llvm::dyn_cast<llvm::IntegerType>(get_llvm())->getBitWidth() / CHAR_BIT;
    }

    [[nodiscard]] llvm::Value *min() const override { return make_value(INT64_MIN); };
    [[nodiscard]] llvm::Value *max() const override { return make_value(INT64_MAX); };
};

class BooleanType : public SimpleType {
  public:
    explicit BooleanType() : SimpleType("BOOLEAN", TypeId::boolean) {};
    ~BooleanType() override = default;

    [[nodiscard]] llvm::Constant *make_value(Bool b) const;
    [[nodiscard]] size_t          get_size() const override {
        return 1; // llvm::dyn_cast<llvm::IntegerType>(get_llvm())->getBitWidth() / CHAR_BIT;
    }

    [[nodiscard]] llvm::Value *min() const override { return make_value(false); };
    [[nodiscard]] llvm::Value *max() const override { return make_value(true); };
};

class RealCType : public SimpleType {
  public:
    explicit RealCType() : SimpleType("REAL", TypeId::real) {};
    ~RealCType() override = default;

    [[nodiscard]] llvm::Constant *make_value(Real f) const {
        return llvm::ConstantFP::get(get_llvm(), f);
    }
    [[nodiscard]] size_t get_size() const override { return sizeof(Real); } // 64 bit floats;

    [[nodiscard]] llvm::Value *min() const override { return make_value(DBL_MIN); };
    [[nodiscard]] llvm::Value *max() const override { return make_value(DBL_MAX); };
};

class CharacterType : public SimpleType {
  public:
    explicit CharacterType() : SimpleType("CHAR", TypeId::chr) {};
    ~CharacterType() override = default;

    [[nodiscard]] llvm::Constant *make_value(Char c) const;
    [[nodiscard]] size_t          get_size() const override {
        return llvm::dyn_cast<llvm::IntegerType>(get_llvm())->getBitWidth() / CHAR_BIT;
    }

    [[nodiscard]] llvm::Value *min() const override { return make_value(WCHAR_MIN); };
    [[nodiscard]] llvm::Value *max() const override { return make_value(WCHAR_MAX); };
};

class ProcedureType : public Type_ {
  public:
    explicit ProcedureType() : Type_(TypeId::procedure) {};
    ProcedureType(Type returns, std::vector<std::pair<Type, Attr>> params)
        : Type_(TypeId::procedure), ret(std::move(returns)), params(std::move(params)) {};
    ~ProcedureType() override = default;

    explicit operator std::string() override;

    [[nodiscard]] bool is_assignable() const override { return false; };

    [[nodiscard]] llvm::Type *get_llvm() const override;

    [[nodiscard]] Type get_closure_struct() const;

    Type ret{nullptr};
    Type receiver{nullptr};
    Attr receiver_type{Attr::null};
    using ParamsList = std::vector<std::pair<Type, Attr>>;
    ParamsList params{};
    using FreeList = std::vector<std::pair<std::string, Type>>;
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

class ArrayType : public Type_ {
  public:
    explicit ArrayType(Type b) : Type_(TypeId::array), base_type(std::move(b)) {};
    ~ArrayType() override = default;

    explicit operator std::string() override;

    [[nodiscard]] bool is_assignable() const override { return false; };

    [[nodiscard]] llvm::Type     *get_llvm() const override;
    [[nodiscard]] llvm::Constant *get_init() const override;

    [[nodiscard]] size_t get_size() const override {
        return std::accumulate(begin(dimensions), end(dimensions), size_t(1),
                               std::multiplies<>()) *
               base_type->get_size();
    }

    Type                base_type;
    std::vector<size_t> dimensions;
};

class OpenArrayType : public ArrayType {
  public:
    explicit OpenArrayType(Type b) : ArrayType(std::move(b)) { id = TypeId::openarray; };
    ~OpenArrayType() override = default;

    explicit operator std::string() override;

    [[nodiscard]] bool is_assignable() const override { return true; };

    [[nodiscard]] llvm::Type     *get_llvm() const override;
    [[nodiscard]] llvm::Constant *get_init() const override;
};

class StringType : public SimpleType {
  public:
    StringType() : SimpleType("STRING", TypeId::string) {};
    ~StringType() override = default;

    static llvm::Constant *make_value(std::string const &s);
    static llvm::Type     *make_type(std::string const &s);
    static llvm::Type     *make_type_ptr();
};

class RecordType : public Type_ {
  public:
    RecordType() : Type_(TypeId::record) {};
    ~RecordType() override = default;

    explicit operator std::string() override;

    [[nodiscard]] bool is_assignable() const override { return false; };

    std::vector<llvm::Type *>     get_fieldTypes() const;
    std::vector<llvm::Constant *> get_fieldInit() const;

    llvm::Type     *get_llvm() const override;
    llvm::Constant *get_init() const override;

    void   insert(std::string const &field, Type type);
    bool   has_field(std::string const &field);
    size_t count() { return index.size(); };

    void                        set_baseType(std::shared_ptr<RecordType> const &b) { base = b; };
    std::shared_ptr<RecordType> baseType() { return base; };
    bool                        is_base(Type const &t) const;

    std::optional<Type> get_type(std::string const &field);
    int                 get_index(std::string const &field);

    size_t get_size() const override;

    void        set_identified(std::string const &s) { identified = s; };
    std::string get_identified() { return identified; };

    bool equiv(std::shared_ptr<RecordType> const &r);

  private:
    std::string                 identified{}; // identified records
    std::shared_ptr<RecordType> base{nullptr};
    llvm::StringMap<Type>       fields;
    std::vector<std::string>    index;

    mutable llvm::Type *cache{nullptr};
};

class TypeAlias : public Type_ {
  public:
    TypeAlias(std::string n, Type t)
        : Type_(TypeId::alias), name{std::move(n)}, alias{std::move(t)} {};
    ~TypeAlias() override = default;

    explicit             operator std::string() override { return name; };
    [[nodiscard]] size_t get_size() const override { return alias->get_size(); };

    Type get_alias() { return alias; }

  private:
    std::string name;
    Type        alias;
};

class PointerType : public Type_ {
  public:
    explicit PointerType(std::string r) : Type_(TypeId::pointer), ref_name{std::move(r)} {};
    explicit PointerType(Type const &r) : Type_(TypeId::pointer), reference{r} {
        ref_name = r->get_name();
    };

    ~PointerType() override = default;

    explicit             operator std::string() override { return '^' + ref_name; };
    [[nodiscard]] size_t get_size() const override { return PTR_SIZE; };

    [[nodiscard]] llvm::Type     *get_llvm() const override;
    [[nodiscard]] llvm::Constant *get_init() const override;

    Type get_reference() { return reference; }
    void set_reference(Type &r) { reference = r; }

    std::string &get_ref_name() { return ref_name; };

  private:
    constexpr static size_t PTR_SIZE = 8;
    Type                    reference = nullptr;
    std::string             ref_name;
};

inline bool is_ptr_to_record(Type const &t) {
    if (t->id == TypeId::pointer) {
        if (auto p = std::dynamic_pointer_cast<PointerType>(t);
            p->get_reference()->id == TypeId::record) {
            return true;
        }
    }
    return false;
}

class SetCType : public SimpleType {
  public:
    SetCType() : SimpleType("SET", TypeId::set) {};
    ~SetCType() override = default;

    [[nodiscard]] llvm::Type     *get_llvm() const override;
    [[nodiscard]] llvm::Constant *get_init() const override;

    [[nodiscard]] llvm::Value *min() const override;
    [[nodiscard]] llvm::Value *max() const override;
};

class ModuleType : public Type_ {
  public:
    explicit ModuleType(std::string n) : Type_(TypeId::module), name{std::move(n)} {};
    ~ModuleType() override = default;

    explicit operator std::string() override { return "MODULE: " + name; };

    [[nodiscard]] bool is_assignable() const override { return false; };

    std::string &module_name() { return name; };

  private:
    std::string name;
};

} // namespace ax
