//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "type_pointer.hh"

namespace ax {

llvm::Type *PointerType::get_llvm() const {
    return llvm::PointerType::get(reference->get_llvm()->getContext(), 0);
}

llvm::Constant *PointerType::get_init() const {
    return llvm::Constant::getNullValue(get_llvm());
}

} // namespace ax
