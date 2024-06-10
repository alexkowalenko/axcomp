//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko

#include "typetable.hh"
#include "type.hh"

#include <format>

#include <llvm/IR/Constants.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/Debug.h>
#include <memory>

namespace ax {

using namespace llvm;

#define DEBUG_TYPE "types"

template <typename S, typename... Args> static void debug(const S &format, const Args &...msg) {
    LLVM_DEBUG(llvm::dbgs() << DEBUG_TYPE << std::vformat(format, std::make_format_args(msg...))
                            << '\n'); // NOLINT
}

void TypeTable::set_type_alias(char const *name, Type const &t) {
    auto type = std::make_shared<TypeAlias>(name, t);
    put(name, type);
}

void TypeTable::initialise() {

    set_singleton(this);
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

    Str1Type = std::make_shared<StringType>();
    Str1Type->id = TypeId::str1;
    Str1Type->name = "STRING1";
    put(std::string(*Str1Type), Str1Type);

    SetType = std::make_shared<SetCType>();
    put(std::string(*SetType), SetType);

    VoidType = std::make_shared<SimpleType>("void", TypeId::null);
    put(std::string(*VoidType), VoidType);
    AnyType = std::make_shared<SimpleType>("any", TypeId::any);
    put(std::string(*AnyType), AnyType);

    // Type aliases for compatiblity
    set_type_alias("SHORTINT", TypeTable::IntType);
    set_type_alias("LONGINT", TypeTable::IntType);
    set_type_alias("HUGEINT", TypeTable::IntType);

    set_type_alias("LONGREAL", TypeTable::RealType);

    // Type Rules

    // 1-Arity
    reg(TokenType::tilde, BoolType, BoolType);

    // 2-Arity
    reg(TokenType::plus, IntType, IntType, IntType);
    reg(TokenType::plus, IntType, RealType, RealType);
    reg(TokenType::plus, RealType, IntType, RealType);
    reg(TokenType::plus, RealType, RealType, RealType);

    reg(TokenType::plus, StrType, StrType, StrType);
    reg(TokenType::plus, StrType, CharType, StrType);
    reg(TokenType::plus, CharType, StrType, StrType);

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
    reg(TokenType::equals, RealType, IntType, BoolType);
    reg(TokenType::equals, IntType, RealType, BoolType);
    reg(TokenType::equals, RealType, RealType, BoolType);

    reg(TokenType::hash, IntType, IntType, BoolType);
    reg(TokenType::hash, RealType, IntType, BoolType);
    reg(TokenType::hash, IntType, RealType, BoolType);
    reg(TokenType::hash, RealType, RealType, BoolType);

    reg(TokenType::less, IntType, IntType, BoolType);
    reg(TokenType::less, RealType, IntType, BoolType);
    reg(TokenType::less, IntType, RealType, BoolType);
    reg(TokenType::less, RealType, RealType, BoolType);

    reg(TokenType::leq, IntType, IntType, BoolType);
    reg(TokenType::leq, RealType, IntType, BoolType);
    reg(TokenType::leq, IntType, RealType, BoolType);
    reg(TokenType::leq, RealType, RealType, BoolType);

    reg(TokenType::greater, IntType, IntType, BoolType);
    reg(TokenType::greater, RealType, IntType, BoolType);
    reg(TokenType::greater, IntType, RealType, BoolType);
    reg(TokenType::greater, RealType, RealType, BoolType);

    reg(TokenType::gteq, IntType, IntType, BoolType);
    reg(TokenType::gteq, RealType, IntType, BoolType);
    reg(TokenType::gteq, IntType, RealType, BoolType);
    reg(TokenType::gteq, RealType, RealType, BoolType);

    // String comparsion operators
    reg(TokenType::equals, StrType, StrType, BoolType);
    reg(TokenType::hash, StrType, StrType, BoolType);
    reg(TokenType::less, StrType, StrType, BoolType);
    reg(TokenType::leq, StrType, StrType, BoolType);
    reg(TokenType::greater, StrType, StrType, BoolType);
    reg(TokenType::gteq, StrType, StrType, BoolType);

    reg(TokenType::ampersand, BoolType, BoolType, BoolType);
    reg(TokenType::or_k, BoolType, BoolType, BoolType);
    reg(TokenType::equals, BoolType, BoolType, BoolType);
    reg(TokenType::hash, BoolType, BoolType, BoolType);

    reg(TokenType::equals, CharType, CharType, BoolType);
    reg(TokenType::hash, CharType, CharType, BoolType);
    reg(TokenType::less, CharType, CharType, BoolType);
    reg(TokenType::leq, CharType, CharType, BoolType);
    reg(TokenType::greater, CharType, CharType, BoolType);
    reg(TokenType::gteq, CharType, CharType, BoolType);

    // Set operations
    reg(TokenType::equals, SetType, SetType, BoolType);
    reg(TokenType::hash, SetType, SetType, BoolType);
    reg(TokenType::in, IntType, SetType, BoolType);

    reg(TokenType::plus, SetType, SetType, SetType);
    reg(TokenType::dash, SetType, SetType, SetType);
    reg(TokenType::asterisk, SetType, SetType, SetType);
    reg(TokenType::slash, SetType, SetType, SetType);

    // assignment
    reg(TokenType::assign, IntType, IntType, VoidType);
    reg(TokenType::assign, BoolType, BoolType, VoidType);
    reg(TokenType::assign, RealType, RealType, VoidType);
    reg(TokenType::assign, CharType, CharType, VoidType);
    reg(TokenType::assign, StrType, StrType, VoidType);
    reg(TokenType::assign, SetType, SetType, VoidType);

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
    VoidType->set_init(llvm::ConstantPointerNull::get(llvm::Type::getInt8PtrTy(context)));
}

Type TypeTable::resolve(std::string const &n) {

    // All ARRAY OF CHAR are STRING types
    if (n == "CHAR[]") {
        return StrType;
    }

    auto name = n;
    while (true) {
        // debug("TypeTable::resolve {0}", name);
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

Type TypeTable::check(TokenType op, Type const &type) {
    auto range = rules1.equal_range(op);
    for (auto i = range.first; i != range.second; ++i) {
        if (i->second.value == type) {
            return i->second.result;
        }
    }
    return nullptr;
}

Type TypeTable::check(TokenType op, Type const &Lt, Type const &Rt) {
    auto range = rules2.equal_range(op);
    auto L = resolve(Lt->get_name());
    auto R = resolve(Rt->get_name());
    for (auto i = range.first; i != range.second; ++i) {
        if (i->second.L == L && i->second.R == R) {
            return i->second.result;
        }
    }
    // Deal with the assignment and comparison of NIL to pointers
    // debug("TypeTable::check {0} {1} {2}", string(op), L->get_name(), R->get_name());
    if (op == TokenType::assign && L->id == TypeId::pointer && R->id == TypeId::null) {
        return TypeTable::VoidType;
    }
    if ((op == TokenType::equals || op == TokenType::hash) && L->id == TypeId::pointer &&
        R->id == TypeId::null) {
        return TypeTable::BoolType;
    }
    return nullptr;
}

void TypeTable::reg(TokenType op, Type const &type, Type const &result) {
    rules1.insert({op, {type, result}});
}

void TypeTable::reg(TokenType op, Type const &L, Type const &R, Type const &result) {
    rules2.insert({op, {L, R, result}});
}

} // namespace ax