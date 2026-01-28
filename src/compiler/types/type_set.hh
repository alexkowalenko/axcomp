//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include "type_simple.hh"

namespace ax {

class SetCType : public SimpleType {
  public:
    SetCType() : SimpleType("SET", TypeId::SET) {};
    ~SetCType() override = default;

    [[nodiscard]] llvm::Type     *get_llvm() const override;
    [[nodiscard]] llvm::Constant *get_init() const override;

    [[nodiscard]] llvm::Value *min() const override;
    [[nodiscard]] llvm::Value *max() const override;
};

} // namespace ax
