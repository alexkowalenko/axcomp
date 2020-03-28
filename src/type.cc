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

RecordType::operator std::string() {
    std::string str{"{ "};
    for (auto &t : fields) {
        str += std::string(*t);
        if (t != *(fields.end() - 1)) {
            str += ",";
        }
    }
    str += "}";
    return str;
}

llvm::Type *RecordType::get_llvm() {
    std::vector<llvm::Type *> fs;
    std::for_each(begin(fields), end(fields),
                  [&fs](auto const &f) { fs.push_back(f->get_llvm()); });
    return StructType::create(fs);
};

llvm::Constant *RecordType::get_init() {
    std::vector<llvm::Constant *> fs;
    std::for_each(begin(fields), end(fields),
                  [&fs](auto const &f) { fs.push_back(f->get_init()); });
    return ConstantStruct::get(dyn_cast<llvm::StructType>(get_llvm()), fs);
};

} // namespace ax
