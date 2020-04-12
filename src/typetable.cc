//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko

#include "typetable.hh"

#include <llvm/IR/Constants.h>
#include <llvm/IR/LLVMContext.h>

namespace ax {

using namespace llvm;

std::shared_ptr<IntegerType> TypeTable::IntType;
std::shared_ptr<BooleanType> TypeTable::BoolType;
TypePtr                      TypeTable::VoidType;

void TypeTable::initialise() {
    IntType = std::make_shared<IntegerType>();
    table.put(std::string(*IntType), IntType);

    BoolType = std::make_shared<BooleanType>();
    table.put(std::string(*BoolType), BoolType);

    VoidType = std::make_shared<SimpleType>("void");
    table.put(std::string(*VoidType), VoidType);
}

void TypeTable::setTypes(llvm::LLVMContext &context) {
    IntType->set_llvm(llvm::Type::getInt64Ty(context));
    IntType->set_init(IntType->make_value(0));

    BoolType->set_llvm(llvm::Type::getInt1Ty(context));
    BoolType->set_init(BoolType->make_value(false));

    VoidType->set_llvm(llvm::Type::getVoidTy(context));
}

std::optional<TypePtr> TypeTable::find(std::string const &name) {
    return table.find(name);
}

std::optional<TypePtr> TypeTable::resolve(std::string const &n) {
    auto name = n;
    while (true) {
        auto res = table.find(name);
        if (!res) {
            // not found
            return res;
        }
        auto alias = std::dynamic_pointer_cast<TypeAlias>(*res);
        if (!alias) {
            // normal type
            return res;
        }
        name = alias->get_alias()->get_name();
    }
}

void TypeTable::put(std::string const &name, TypePtr const &t) {
    table.put(name, t);
}

} // namespace ax