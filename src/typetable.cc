//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko

#include "typetable.hh"
#include "type.hh"

#include <llvm/IR/Constants.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/Debug.h>
#include <memory>

namespace ax {

using namespace llvm;

#define DEBUG_TYPE "types"

template <typename... T> static void debug(const T &... msg) {
    LLVM_DEBUG(llvm::dbgs() << llvm::formatv(msg...) << '\n');
}

std::shared_ptr<IntegerType>   TypeTable::IntType;
std::shared_ptr<BooleanType>   TypeTable::BoolType;
std::shared_ptr<RealCType>     TypeTable::RealType;
std::shared_ptr<CharacterType> TypeTable::CharType;
std::shared_ptr<StringType>    TypeTable::StrType;
TypePtr                        TypeTable::VoidType;
TypePtr                        TypeTable::AnyType;

void TypeTable::initialise() {
    IntType = std::make_shared<IntegerType>();
    put(std::string(*IntType), IntType);

    BoolType = std::make_shared<BooleanType>();
    put(std::string(*BoolType), BoolType);

    RealType = std::make_shared<RealCType>();
    put(std::string(*RealType), RealType);

    CharType = std::make_shared<CharacterType>();
    put(std::string(*CharType), CharType);

    StrType = std::make_shared<StringType>();
    put(std::string(*StrType), StrType);

    VoidType = std::make_shared<SimpleType>("void", TypeId::null);
    put(std::string(*VoidType), VoidType);
    AnyType = std::make_shared<SimpleType>("any", TypeId::any);
    put(std::string(*AnyType), AnyType);

    // Type Rules

    // 1-Arity
    reg(TokenType::tilde, BoolType, BoolType);

    // 2-Arity
    reg(TokenType::plus, IntType, IntType, IntType);
    reg(TokenType::plus, IntType, RealType, RealType);
    reg(TokenType::plus, RealType, IntType, RealType);
    reg(TokenType::plus, RealType, RealType, RealType);

    reg(TokenType::dash, IntType, IntType, IntType);
    reg(TokenType::dash, IntType, RealType, RealType);
    reg(TokenType::dash, RealType, IntType, RealType);
    reg(TokenType::dash, RealType, RealType, RealType);

    reg(TokenType::asterisk, IntType, IntType, IntType);
    reg(TokenType::asterisk, IntType, RealType, RealType);
    reg(TokenType::asterisk, RealType, IntType, RealType);
    reg(TokenType::asterisk, RealType, RealType, RealType);

    reg(TokenType::slash, IntType, IntType, RealType);
    reg(TokenType::slash, IntType, RealType, RealType);
    reg(TokenType::slash, RealType, IntType, RealType);
    reg(TokenType::slash, RealType, RealType, RealType);

    reg(TokenType::div, IntType, IntType, IntType);
    reg(TokenType::mod, IntType, IntType, IntType);

    // logical operators
    reg(TokenType::equals, IntType, IntType, BoolType);
    reg(TokenType::hash, IntType, IntType, BoolType);
    reg(TokenType::less, IntType, IntType, BoolType);
    reg(TokenType::leq, IntType, IntType, BoolType);
    reg(TokenType::greater, IntType, IntType, BoolType);
    reg(TokenType::gteq, IntType, IntType, BoolType);

    reg(TokenType::equals, RealType, RealType, BoolType);
    reg(TokenType::hash, RealType, RealType, BoolType);
    reg(TokenType::less, RealType, RealType, BoolType);
    reg(TokenType::leq, RealType, RealType, BoolType);
    reg(TokenType::greater, RealType, RealType, BoolType);
    reg(TokenType::gteq, RealType, RealType, BoolType);

    reg(TokenType::ampersand, BoolType, BoolType, BoolType);
    reg(TokenType::or_k, BoolType, BoolType, BoolType);
    reg(TokenType::equals, BoolType, BoolType, BoolType);
    reg(TokenType::hash, BoolType, BoolType, BoolType);

    reg(TokenType::equals, CharType, CharType, BoolType);
    reg(TokenType::hash, CharType, CharType, BoolType);

    // assignment
    reg(TokenType::assign, IntType, IntType, VoidType);
    reg(TokenType::assign, BoolType, BoolType, VoidType);
    reg(TokenType::assign, RealType, RealType, VoidType);
    reg(TokenType::assign, CharType, CharType, VoidType);
    reg(TokenType::assign, StrType, StrType, VoidType);

    reg(TokenType::assign, IntType, AnyType, VoidType); // Any type can be assigned to anything
    reg(TokenType::assign, BoolType, AnyType, VoidType);
    reg(TokenType::assign, RealType, AnyType, VoidType);
    reg(TokenType::assign, CharType, AnyType, VoidType);
    reg(TokenType::assign, StrType, AnyType, VoidType);
} // namespace ax

void TypeTable::setTypes(llvm::LLVMContext &context) {
    debug("TypeTable::setTypes");

    IntType->set_llvm(llvm::Type::getInt64Ty(context));
    IntType->set_init(IntType->make_value(0));

    BoolType->set_llvm(llvm::Type::getInt1Ty(context));
    BoolType->set_init(BoolType->make_value(false));

    RealType->set_llvm(llvm::Type::getDoubleTy(context));
    RealType->set_init(RealType->make_value(0.0));

    CharType->set_llvm(llvm::Type::getInt32Ty(context));
    CharType->set_init(CharType->make_value(0));

    StrType->set_llvm(llvm::ArrayType::get(llvm::Type::getInt32Ty(context), 0)->getPointerTo());
    StrType->set_init(Constant::getNullValue(llvm::Type::getInt32PtrTy(context)));

    VoidType->set_llvm(llvm::Type::getVoidTy(context));
    VoidType->set_init(
        llvm::ConstantPointerNull::get(llvm::Type::getVoidTy(context)->getPointerTo()));
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

std::optional<TypePtr> TypeTable::check(TokenType op, TypePtr const &type) {
    auto range = rules1.equal_range(op);
    for (auto i = range.first; i != range.second; ++i) {
        if (i->second.value == type) {
            return i->second.result;
        }
    }
    return {};
}

std::optional<TypePtr> TypeTable::check(TokenType op, TypePtr const &L, TypePtr const &R) {
    auto range = rules2.equal_range(op);
    for (auto i = range.first; i != range.second; ++i) {
        if (i->second.L == L && i->second.R == R) {
            return i->second.result;
        }
    }
    // Deal with the assignment of NIL to pointers
    if (op == TokenType::assign && L->id == TypeId::pointer && R->id == TypeId::any) {
        return TypeTable::VoidType;
    }
    return {};
}

void TypeTable::reg(TokenType op, TypePtr const &type, TypePtr const &result) {
    rules1.insert({op, {type, result}});
}

void TypeTable::reg(TokenType op, TypePtr const &L, TypePtr const &R, TypePtr const &result) {
    rules2.insert({op, {L, R, result}});
}

} // namespace ax