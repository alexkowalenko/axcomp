//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "type_procedure.hh"

#include <format>
#include <ranges>

#include "../typetable.hh"

namespace ax {

using namespace llvm;

std::string ProcedureType::get_print(const bool forward) {
    std::string res{forward ? "^" : ""};
    if (receiver) {
        res += std::format("({0})", string(receiver));
    }
    res += '(';
    const auto *insert = "";
    for (const auto &[name, attr] : params) {
        res += insert;
        if (attr == Attr::var) {
            res += " VAR ";
        }
        res += string(name);
        insert = ", ";
    }
    res += ")";
    if (ret) {
        res += ":" + string(ret);
    }
    return res;
}

ProcedureType::operator std::string() {
    return get_print(false);
}

llvm::Type *ProcedureType::get_llvm() const {
    std::vector<llvm::Type *> proto;
    for (const auto &key : params | std::views::keys) {
        proto.push_back(key->get_llvm());
    }
    return FunctionType::get(ret->get_llvm(), proto, false);
}

Type ProcedureType::get_closure_struct() const {
    // this should be a struct
    auto cls_str = std::make_shared<ArrayType>(std::make_shared<PointerType>(TypeTable::IntType));
    cls_str->dimensions.push_back(free_vars.size());
    return cls_str;
}

ProcedureFwdType::operator std::string() {
    return get_print(true);
}

} // namespace ax
