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

llvm::Constant *IntegerType::make_value(long i) {
    return ConstantInt::get(get_llvm(), i);
};

llvm::Constant *BooleanType::make_value(bool b) {
    return ConstantInt::get(get_llvm(), static_cast<uint64_t>(b));
};

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
    std::string str{"{"};
    std::for_each(begin(index), end(index), [&str](auto const &name) {
        str += name;
        str += ",";
    });
    str += "}";
    return str;
}

llvm::Type *RecordType::get_llvm() {
    std::vector<llvm::Type *> fs;
    std::for_each(begin(index), end(index), [&fs, this](auto const &name) {
        fs.push_back(fields[name]->get_llvm());
    });
    return StructType::create(fs);
};

llvm::Constant *RecordType::get_init() {
    std::vector<llvm::Constant *> fs;
    std::for_each(begin(index), end(index), [&fs, this](auto const &name) {
        fs.push_back(fields[name]->get_init());
    });
    return ConstantStruct::get(dyn_cast<llvm::StructType>(get_llvm()), fs);
};

void RecordType::insert(std::string const &field, TypePtr type) {
    fields[field] = type;
    index.push_back(field);
}

bool RecordType::has_field(std::string const &field) {
    return fields.find(field) != fields.end();
}

std::optional<TypePtr> RecordType::get_type(std::string const &field) {
    auto res = fields.find(field);
    if (res != fields.end()) {
        return std::make_optional<TypePtr>(res->second);
    }
    return std::nullopt;
}

int RecordType::get_index(std::string const &field) {
    auto it = std::find(begin(index), end(index), field);
    return std::distance(begin(index), it);
}

} // namespace ax
