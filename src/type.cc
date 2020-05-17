//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "type.hh"
#include "typetable.hh"

#include <llvm/Support/Debug.h>
#include <llvm/Support/FormatVariadic.h>

#include "utf8.h"

namespace ax {

using namespace llvm;

bool Type::equiv(TypePtr const &t) const {
    return id == t->id;
}

llvm::Value *Type::min() {
    return llvm::ConstantPointerNull::get(
        dyn_cast<llvm::PointerType>(TypeTable::VoidType->get_llvm()));
}

llvm::Value *Type::max() {
    return llvm::ConstantPointerNull::get(
        dyn_cast<llvm::PointerType>(TypeTable::VoidType->get_llvm()));
}

SimpleType::operator std::string() {
    return name;
}

llvm::Constant *IntegerType::make_value(long i) {
    return ConstantInt::get(get_llvm(), i);
}

llvm::Constant *BooleanType::make_value(bool b) {
    return ConstantInt::get(get_llvm(), static_cast<uint64_t>(b));
}

llvm::Constant *CharacterType::make_value(Char c) {
    return ConstantInt::get(get_llvm(), static_cast<Char>(c));
}

llvm::Constant *StringType::make_value(std::string const &s) {

    std::vector<Constant *> array;
    auto                    it = s.begin();
    while (it != s.end()) {
        auto c = utf8::next(it, s.end());
        array.push_back(TypeTable::CharType->make_value(c));
    }
    array.push_back(TypeTable::CharType->make_value(0)); // null terminate
    return ConstantArray::get(llvm::dyn_cast<llvm::ArrayType>(make_type(s)), array);
}

llvm::Type *StringType::make_type(std::string const & /* unused */) {
    // Type dependant on string size

    return llvm::ArrayType::get(TypeTable::CharType->get_llvm(),
                                // utf8::distance(s.begin(), s.end()) + 1
                                0); // All strings are undetermined length
}

ProcedureType::operator std::string() {
    std::string res{"("};
    for (auto &t : params) {
        if (t.second == Attr::var) {
            res += " VAR ";
        }
        res += std::string(*t.first);
        if (t != *(params.end() - 1)) {
            res += ",";
        }
    }
    res += "):";
    res += std::string(*ret);
    return res;
}

llvm::Type *ProcedureType::get_llvm() {
    std::vector<llvm::Type *> proto;
    std::for_each(begin(params), end(params),
                  [this, &proto](auto const &t) { proto.push_back(t.first->get_llvm()); });

    return FunctionType::get(ret->get_llvm(), proto, false);
}

ArrayType::operator std::string() {
    std::string result = llvm::formatv("{0}[", std::string(*base_type));
    for (auto iter = dimensions.begin(); iter != dimensions.end(); iter++) {
        result += llvm::formatv("{0}", *iter);
        if ((iter + 1) != dimensions.end()) {
            result += ',';
        }
    };
    result += ']';
    return result;
}

llvm::Type *ArrayType::get_llvm() {
    if (dimensions.size() == 0) {
        return llvm::ArrayType::get(base_type->get_llvm(), 0);
    } else {
        llvm::Type *array_type = base_type->get_llvm();
        for (auto d : dimensions) {
            array_type = llvm::ArrayType::get(array_type, d);
        }
        return array_type;
    }
}

llvm::Constant *ArrayType::get_init() {
    if (dimensions.size() == 0) {
        auto const_array = std::vector<Constant *>(0, base_type->get_init());
        return ConstantArray::get(dyn_cast<llvm::ArrayType>(get_llvm()), const_array);
    } else {
        std::vector<Constant *> const_array =
            std::vector<Constant *>(dimensions[0], base_type->get_init());
        return ConstantArray::get(dyn_cast<llvm::ArrayType>(get_llvm()), const_array);
    }
}

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
    std::for_each(begin(index), end(index),
                  [&fs, this](auto const &name) { fs.push_back(fields[name]->get_llvm()); });
    return StructType::create(fs);
}

llvm::Constant *RecordType::get_init() {
    std::vector<llvm::Constant *> fs;
    std::for_each(begin(index), end(index),
                  [&fs, this](auto const &name) { fs.push_back(fields[name]->get_init()); });
    return ConstantStruct::get(dyn_cast<llvm::StructType>(get_llvm()), fs);
}

void RecordType::insert(std::string const &field, TypePtr type) {
    fields[field] = std::move(type);
    index.push_back(field);
}

bool RecordType::has_field(std::string const &field) {
    return fields.find(field) != fields.end();
}

unsigned int RecordType::get_size() {
    return std::accumulate(begin(fields), end(fields), begin(fields)->second->get_size(),
                           [](unsigned int x, auto &y) { return x + y.second->get_size(); });
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

llvm::Type *PointerType::get_llvm() {
    return reference->get_llvm()->getPointerTo();
}

llvm::Constant *PointerType::get_init() {
    return Constant::getNullValue(get_llvm());
}

} // namespace ax
