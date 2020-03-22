//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko

#include "typetable.hh"

#include <llvm/IR/Constants.h>
#include <llvm/IR/LLVMContext.h>

namespace ax {

using namespace llvm;

TypePtr TypeTable::IntType;
TypePtr TypeTable::BoolType;
TypePtr TypeTable::ModuleType;
TypePtr TypeTable::VoidType;

void TypeTable::initialise() {
    IntType = std::make_shared<IntegerType>();
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
    IntType->set_init(ConstantInt::get(context, APInt(64, 0, true)));

    BoolType->set_llvm(llvm::Type::getInt1Ty(context));
    BoolType->set_init(ConstantInt::get(context, APInt(1, 0, true)));

    ModuleType->set_llvm(llvm::Type::getVoidTy(context));
    VoidType->set_llvm(llvm::Type::getVoidTy(context));
}

std::optional<TypePtr> TypeTable::find(std::string name) {
    return table.find(name);
}

void TypeTable::put(std::string const &name, TypePtr const &t) {
    table.put(name, t);
}

} // namespace ax