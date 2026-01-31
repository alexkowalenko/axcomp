//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko

#include "typetable.hh"
#include "types/all.hh"

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
    Str1Type->id = TypeId::STR1;
    Str1Type->name = "STRING1";
    put(std::string(*Str1Type), Str1Type);

    SetType = std::make_shared<SetCType>();
    put(std::string(*SetType), SetType);

    VoidType = std::make_shared<SimpleType>("void", TypeId::VOID);
    put(std::string(*VoidType), VoidType);
    AnyType = std::make_shared<SimpleType>("any", TypeId::ANY);
    put(std::string(*AnyType), AnyType);

    // Type aliases for compatibility
    set_type_alias("SHORTINT", TypeTable::IntType);
    set_type_alias("LONGINT", TypeTable::IntType);
    set_type_alias("HUGEINT", TypeTable::IntType);

    set_type_alias("LONGREAL", TypeTable::RealType);

    // Type Rules

    // 1-Arity
    reg(TokenType::TILDE, BoolType, BoolType);

    // 2-Arity
    reg(TokenType::PLUS, IntType, IntType, IntType);
    reg(TokenType::PLUS, IntType, RealType, RealType);
    reg(TokenType::PLUS, RealType, IntType, RealType);
    reg(TokenType::PLUS, RealType, RealType, RealType);

    reg(TokenType::PLUS, StrType, StrType, StrType);
    reg(TokenType::PLUS, StrType, CharType, StrType);
    reg(TokenType::PLUS, CharType, StrType, StrType);

    reg(TokenType::DASH, IntType, IntType, IntType);
    reg(TokenType::DASH, IntType, RealType, RealType);
    reg(TokenType::DASH, RealType, IntType, RealType);
    reg(TokenType::DASH, RealType, RealType, RealType);

    reg(TokenType::ASTÉRIX, IntType, IntType, IntType);
    reg(TokenType::ASTÉRIX, IntType, RealType, RealType);
    reg(TokenType::ASTÉRIX, RealType, IntType, RealType);
    reg(TokenType::ASTÉRIX, RealType, RealType, RealType);

    reg(TokenType::SLASH, IntType, IntType, RealType);
    reg(TokenType::SLASH, IntType, RealType, RealType);
    reg(TokenType::SLASH, RealType, IntType, RealType);
    reg(TokenType::SLASH, RealType, RealType, RealType);

    reg(TokenType::DIV, IntType, IntType, IntType);
    reg(TokenType::MOD, IntType, IntType, IntType);

    // logical operators
    reg(TokenType::EQUALS, IntType, IntType, BoolType);
    reg(TokenType::EQUALS, RealType, IntType, BoolType);
    reg(TokenType::EQUALS, IntType, RealType, BoolType);
    reg(TokenType::EQUALS, RealType, RealType, BoolType);

    reg(TokenType::HASH, IntType, IntType, BoolType);
    reg(TokenType::HASH, RealType, IntType, BoolType);
    reg(TokenType::HASH, IntType, RealType, BoolType);
    reg(TokenType::HASH, RealType, RealType, BoolType);

    reg(TokenType::LESS, IntType, IntType, BoolType);
    reg(TokenType::LESS, RealType, IntType, BoolType);
    reg(TokenType::LESS, IntType, RealType, BoolType);
    reg(TokenType::LESS, RealType, RealType, BoolType);

    reg(TokenType::LEQ, IntType, IntType, BoolType);
    reg(TokenType::LEQ, RealType, IntType, BoolType);
    reg(TokenType::LEQ, IntType, RealType, BoolType);
    reg(TokenType::LEQ, RealType, RealType, BoolType);

    reg(TokenType::GREATER, IntType, IntType, BoolType);
    reg(TokenType::GREATER, RealType, IntType, BoolType);
    reg(TokenType::GREATER, IntType, RealType, BoolType);
    reg(TokenType::GREATER, RealType, RealType, BoolType);

    reg(TokenType::GTEQ, IntType, IntType, BoolType);
    reg(TokenType::GTEQ, RealType, IntType, BoolType);
    reg(TokenType::GTEQ, IntType, RealType, BoolType);
    reg(TokenType::GTEQ, RealType, RealType, BoolType);

    // String comparison operators
    reg(TokenType::EQUALS, StrType, StrType, BoolType);
    reg(TokenType::HASH, StrType, StrType, BoolType);
    reg(TokenType::LESS, StrType, StrType, BoolType);
    reg(TokenType::LEQ, StrType, StrType, BoolType);
    reg(TokenType::GREATER, StrType, StrType, BoolType);
    reg(TokenType::GTEQ, StrType, StrType, BoolType);

    reg(TokenType::AMPERSAND, BoolType, BoolType, BoolType);
    reg(TokenType::OR, BoolType, BoolType, BoolType);
    reg(TokenType::EQUALS, BoolType, BoolType, BoolType);
    reg(TokenType::HASH, BoolType, BoolType, BoolType);

    reg(TokenType::EQUALS, CharType, CharType, BoolType);
    reg(TokenType::HASH, CharType, CharType, BoolType);
    reg(TokenType::LESS, CharType, CharType, BoolType);
    reg(TokenType::LEQ, CharType, CharType, BoolType);
    reg(TokenType::GREATER, CharType, CharType, BoolType);
    reg(TokenType::GTEQ, CharType, CharType, BoolType);

    // Set operations
    reg(TokenType::EQUALS, SetType, SetType, BoolType);
    reg(TokenType::HASH, SetType, SetType, BoolType);
    reg(TokenType::IN, IntType, SetType, BoolType);

    reg(TokenType::PLUS, SetType, SetType, SetType);
    reg(TokenType::DASH, SetType, SetType, SetType);
    reg(TokenType::ASTÉRIX, SetType, SetType, SetType);
    reg(TokenType::SLASH, SetType, SetType, SetType);

    // assignment
    reg(TokenType::ASSIGN, IntType, IntType, VoidType);
    reg(TokenType::ASSIGN, BoolType, BoolType, VoidType);
    reg(TokenType::ASSIGN, RealType, RealType, VoidType);
    reg(TokenType::ASSIGN, CharType, CharType, VoidType);
    reg(TokenType::ASSIGN, StrType, StrType, VoidType);
    reg(TokenType::ASSIGN, SetType, SetType, VoidType);

    reg(TokenType::ASSIGN, IntType, AnyType, VoidType); // Any type can be assigned to anything
    reg(TokenType::ASSIGN, BoolType, AnyType, VoidType);
    reg(TokenType::ASSIGN, RealType, AnyType, VoidType);
    reg(TokenType::ASSIGN, CharType, AnyType, VoidType);
    reg(TokenType::ASSIGN, StrType, AnyType, VoidType);
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

    auto *string_ptr = llvm::PointerType::get(context, 0);
    StrType->set_llvm(string_ptr);
    StrType->set_init(llvm::ConstantPointerNull::get(string_ptr));

    VoidType->set_llvm(llvm::Type::getVoidTy(context));
    VoidType->set_init(llvm::ConstantPointerNull::get(llvm::PointerType::get(context, 0)));

    // Set enumeration types to i32 if needed.
    if (auto *table = TypeTable::sgl()) {
        for (auto const &[name, type] : *table) {
            auto resolved = type;
            if (const auto alias = std::dynamic_pointer_cast<TypeAlias>(type)) {
                resolved = alias->get_alias();
            }
            if (resolved && resolved->id == TypeId::ENUMERATION && !resolved->get_llvm()) {
                resolved->set_llvm(llvm::Type::getInt32Ty(context));
                if (const auto enum_type = std::dynamic_pointer_cast<EnumType>(resolved)) {
                    resolved->set_init(enum_type->make_value(0));
                } else {
                    resolved->set_init(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0));
                }
            }
        }
    }
}

Type TypeTable::resolve(std::string const &n) const {

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
        const auto alias = std::dynamic_pointer_cast<TypeAlias>(res);
        if (!alias) {
            // normal type
            return res;
        }
        const auto next = alias->get_alias()->get_name();
        if (next == name) {
            return alias->get_alias();
        }
        name = next;
    }
}

Type TypeTable::check(const TokenType op, Type const &type) {
    auto [fst, snd] = rules1.equal_range(op);
    for (auto i = fst; i != snd; ++i) {
        if (i->second.value == type) {
            return i->second.result;
        }
    }
    return nullptr;
}

Type TypeTable::check(const TokenType op, Type const &Lt, Type const &Rt) {
    auto [fst, snd] = rules2.equal_range(op);
    const auto L = resolve(Lt->get_name());
    const auto R = resolve(Rt->get_name());
    for (auto i = fst; i != snd; ++i) {
        if (i->second.L == L && i->second.R == R) {
            return i->second.result;
        }
    }
    // Deal with the assignment and comparison of NIL to pointers
    // debug("TypeTable::check {0} {1} {2}", string(op), L->get_name(), R->get_name());
    if (op == TokenType::ASSIGN && L->id == TypeId::POINTER && R->id == TypeId::VOID) {
        return TypeTable::VoidType;
    }
    if ((op == TokenType::EQUALS || op == TokenType::HASH) && L->id == TypeId::POINTER &&
        R->id == TypeId::VOID) {
        return TypeTable::BoolType;
    }
    if ((op == TokenType::EQUALS || op == TokenType::HASH) && L->id == TypeId::ENUMERATION &&
        R->id == TypeId::ENUMERATION && L->equiv(R)) {
        return TypeTable::BoolType;
    }
    return nullptr;
}

void TypeTable::reg(TokenType op, Type const &type, Type const &result) {
    rules1.insert({op, {.value = type, .result = result}});
}

void TypeTable::reg(TokenType op, Type const &L, Type const &R, Type const &result) {
    rules2.insert({op, {.L = L, .R = R, .result = result}});
}

} // namespace ax
