//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "type_array.hh"

#include <format>

namespace ax {

ArrayType::operator std::string() {
    std::string result = std::format("{0}[", string(base_type));
    for (auto iter = dimensions.cbegin(); iter != dimensions.cend(); iter++) {
        result += std::format("{0}", *iter);
        if ((iter + 1) != dimensions.end()) {
            result += ',';
        }
    };
    result += ']';
    return result;
}

llvm::Type *ArrayType::get_llvm() const {
    if (dimensions.empty()) {
        return llvm::ArrayType::get(base_type->get_llvm(), 0);
    }

    llvm::Type *array_type = base_type->get_llvm();
    for (const auto d : dimensions) {
        array_type = llvm::ArrayType::get(array_type, d);
    }
    return array_type;
}

llvm::Constant *ArrayType::get_init() const {
    if (dimensions.empty()) {
        const auto const_array = std::vector<llvm::Constant *>(0, base_type->get_init());
        return llvm::ConstantArray::get(llvm::dyn_cast<llvm::ArrayType>(get_llvm()), const_array);
    }

    auto const const_array = std::vector<llvm::Constant *>(dimensions[0], base_type->get_init());
    return llvm::ConstantArray::get(llvm::dyn_cast<llvm::ArrayType>(get_llvm()), const_array);
}

OpenArrayType::operator std::string() {
    return std::format("{0}[]", string(base_type));
}

llvm::Type *OpenArrayType::get_llvm() const {
    return llvm::PointerType::get(ArrayType::get_llvm()->getContext(), 0);
}

llvm::Constant *OpenArrayType::get_init() const {
    return llvm::Constant::getNullValue(get_llvm());
}

} // namespace ax
