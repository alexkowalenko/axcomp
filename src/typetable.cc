//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko

#include "typetable.hh"

#include <llvm/IR/LLVMContext.h>

namespace ax {

TypePtr TypeTable::IntType;
TypePtr TypeTable::BoolType;
TypePtr TypeTable::ModuleType;
TypePtr TypeTable::VoidType;

void TypeTable::initialise() {
    llvm::LLVMContext context;
    IntType = std::make_shared<SimpleType>("INTEGER");
    table.put(std::string(*IntType), IntType);

    BoolType = std::make_shared<SimpleType>("BOOLEAN");
    table.put(std::string(*BoolType), BoolType);

    ModuleType = std::make_shared<SimpleType>("MODULE");
    table.put(std::string(*ModuleType), ModuleType);

    VoidType = std::make_shared<SimpleType>("void");
    table.put(std::string(*VoidType), VoidType);
}

void TypeTable::setTypes(llvm::LLVMContext &context) {
    IntType->set_llvm(llvm::Type::getInt64Ty(context));
    BoolType->set_llvm(llvm::Type::getInt1Ty(context));
    ModuleType->set_llvm(llvm::Type::getVoidTy(context));
    VoidType->set_llvm(llvm::Type::getVoidTy(context));
}

bool TypeTable::isNumericType(TypePtr const &t) {
    return t == IntType;
}

std::optional<TypePtr> TypeTable::find(std::string const &name) {
    return table.find(name);
}

void TypeTable::put(std::string const &name, TypePtr const &t) {
    table.put(name, t);
}

} // namespace ax