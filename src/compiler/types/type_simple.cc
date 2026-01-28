//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "type_simple.hh"

#include "../typetable.hh"

#include "utf8.h"

namespace ax {

using namespace llvm;

SimpleType::operator std::string() {
    return name;
}

llvm::Constant *IntegerType::make_value(const long i) const {
    return ConstantInt::get(get_llvm(), i);
}

llvm::Constant *BooleanType::make_value(const bool b) const {
    return ConstantInt::get(get_llvm(), static_cast<uint64_t>(b));
}

llvm::Constant *CharacterType::make_value(const Char c) const {
    return ConstantInt::get(get_llvm(), c);
}

llvm::Constant *StringType::make_value(std::string const &s) {

    std::vector<Constant *> array;
    auto                    it = s.cbegin();
    while (it != s.cend()) {
        const auto c = utf8::next(it, s.end());
        array.push_back(TypeTable::CharType->make_value(c));
    }
    array.push_back(TypeTable::CharType->make_value(0)); // null terminate
    return ConstantArray::get(llvm::dyn_cast<llvm::ArrayType>(make_type(s)), array);
}

llvm::Type *StringType::make_type(std::string const &s) {
    // Type dependent on string size

    return llvm::ArrayType::get(TypeTable::CharType->get_llvm(),
                                utf8::distance(s.begin(), s.end()) + 1);
}

llvm::Type *StringType::make_type_ptr() {
    return TypeTable::StrType->get_llvm();
}

} // namespace ax
