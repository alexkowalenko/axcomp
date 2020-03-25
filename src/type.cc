//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "type.hh"

#include <llvm/Support/FormatVariadic.h>

namespace ax {

using namespace llvm;

bool Type::equiv(TypePtr const &t) {
    return std::string(*this) == std::string(*t);
}

SimpleType::operator std::string() {
    return name;
}

ProcedureType::operator std::string() {
    std::string res{"("};
    for (auto &t : params) {
        res += std::string(*t);
        if (t != *(params.end() - 1)) {
            res += ",";
        }
    }
    res += "):";
    res += std::string(*ret);
    return res;
}

ArrayType::operator std::string() {
    return llvm::formatv("{0}[{1}]", std::string(*base_type), size);
}

llvm::Type *ArrayType::get_llvm() {
    return llvm::ArrayType::get(base_type->get_llvm(), size);
};

llvm::Constant *ArrayType::get_init() {
    auto const_array = std::vector<Constant *>(size, base_type->get_init());
    return ConstantArray::get(dyn_cast<llvm::ArrayType>(get_llvm()),
                              const_array);
};

} // namespace ax
