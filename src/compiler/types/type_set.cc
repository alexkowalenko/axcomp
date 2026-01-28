//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "type_set.hh"

#include "../typetable.hh"

namespace ax {

llvm::Type *SetCType::get_llvm() const {
    return TypeTable::IntType->get_llvm(); // 64-bit set
};

llvm::Constant *SetCType::get_init() const {
    return TypeTable::IntType->get_init(); // 64-bit set
};

llvm::Value *SetCType::min() const {
    return TypeTable::IntType->get_init();
};

llvm::Value *SetCType::max() const {
    return TypeTable::IntType->make_value(SET_MAX);
};

} // namespace ax
