//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <cfloat>
#include <climits>
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

#include "../attr.hh"
#include "../ax.hh"

namespace ax {

enum class TypeId : std::uint8_t {
    VOID, // void
    ANY,  // any - used to for multi type return values like MIN, MAX
    INTEGER,
    REAL,
    BOOLEAN,
    ENUMERATION,
    CHAR,
    PROCEDURE,
    PROCEDURE_FWD,
    ARRAY,
    OPENARRAY,
    STRING,
    STR1, // One char STRING which can be a CHAR
    SET,
    RECORD,
    ALIAS,
    POINTER,
    MODULE
};

std::string const &string(TypeId t);

class Type_;
using Type = std::shared_ptr<Type_>;

class Type_ {
  public:
    explicit Type_(const TypeId i) : id(i) {};
    virtual ~Type_() = default;

    TypeId id = TypeId::VOID;

    [[nodiscard]] bool equiv(Type const &t);

    virtual explicit operator std::string() = 0;

    std::string get_name() { return std::string(*this); }

    [[nodiscard]] virtual bool   is_numeric() const { return false; };
    [[nodiscard]] constexpr bool is_array() const {
        return id == TypeId::ARRAY || id == TypeId::OPENARRAY;
    };
    [[nodiscard]] virtual bool is_assignable() const { return true; };
    [[nodiscard]] bool constexpr is_referencable() const {
        return !(id == TypeId::PROCEDURE || id == TypeId::ALIAS || id == TypeId::MODULE);
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

} // namespace ax
