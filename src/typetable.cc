//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko

#include "typetable.hh"

#include <llvm/IR/Constants.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/Debug.h>

namespace ax {

using namespace llvm;

#define DEBUG_TYPE "types"

template <typename... T> static void debug(const T &... msg) {
    LLVM_DEBUG(llvm::dbgs() << llvm::formatv(msg...) << '\n');
}

std::shared_ptr<IntegerType>   TypeTable::IntType;
std::shared_ptr<BooleanType>   TypeTable::BoolType;
std::shared_ptr<CharacterType> TypeTable::CharType;
std::shared_ptr<StringType>    TypeTable::StrType;
TypePtr                        TypeTable::VoidType;
TypePtr                        TypeTable::AnyType;

void TypeTable::initialise() {
    IntType = std::make_shared<IntegerType>();
    put(std::string(*IntType), IntType);

    BoolType = std::make_shared<BooleanType>();
    put(std::string(*BoolType), BoolType);

    CharType = std::make_shared<CharacterType>();
    put(std::string(*CharType), CharType);

    StrType = std::make_shared<StringType>();
    put(std::string(*StrType), StrType);

    VoidType = std::make_shared<SimpleType>("void", TypeId::null);
    put(std::string(*VoidType), VoidType);
    AnyType = std::make_shared<SimpleType>("any", TypeId::any);
    put(std::string(*AnyType), AnyType);
}

void TypeTable::setTypes(llvm::LLVMContext &context) {
    debug("TypeTable::setTypes");

    IntType->set_llvm(llvm::Type::getInt64Ty(context));
    IntType->set_init(IntType->make_value(0));

    BoolType->set_llvm(llvm::Type::getInt1Ty(context));
    BoolType->set_init(BoolType->make_value(false));

    CharType->set_llvm(llvm::Type::getInt32Ty(context));
    CharType->set_init(CharType->make_value(0));

    StrType->set_llvm(llvm::ArrayType::get(llvm::Type::getInt32Ty(context), 0)->getPointerTo());
    StrType->set_init(Constant::getNullValue(llvm::Type::getInt32PtrTy(context)));

    VoidType->set_llvm(llvm::Type::getVoidTy(context));
}

std::optional<TypePtr> TypeTable::resolve(std::string const &n) {
    auto name = n;
    while (true) {
        auto res = find(name);
        if (!res) {
            // not found
            return res;
        }
        auto alias = std::dynamic_pointer_cast<TypeAlias>(res);
        if (!alias) {
            // normal type
            return res;
        }
        name = alias->get_alias()->get_name();
    }
}

} // namespace ax