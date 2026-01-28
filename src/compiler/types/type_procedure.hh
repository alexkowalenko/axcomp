//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include "type.hh"

namespace ax {

class ProcedureType : public Type_ {
  public:
    explicit ProcedureType() : Type_(TypeId::PROCEDURE) {};
    ProcedureType(Type returns, std::vector<std::pair<Type, Attr>> params)
        : Type_(TypeId::PROCEDURE), ret(std::move(returns)), params(std::move(params)) {};
    ~ProcedureType() override = default;

    explicit operator std::string() override;

    [[nodiscard]] bool is_assignable() const override { return false; };

    [[nodiscard]] llvm::Type *get_llvm() const override;

    [[nodiscard]] Type get_closure_struct() const;

    Type ret{nullptr};
    Type receiver{nullptr};
    Attr receiver_type{Attr::null};
    using ParamsList = std::vector<std::pair<Type, Attr>>;
    ParamsList params;
    using FreeList = std::vector<std::pair<std::string, Type>>;
    FreeList free_vars;

  protected:
    std::string get_print(bool forward);
};

class ProcedureFwdType : public ProcedureType {
  public:
    explicit ProcedureFwdType() { id = TypeId::PROCEDURE_FWD; };
    ~ProcedureFwdType() override = default;

    explicit operator std::string() override;
};

} // namespace ax
