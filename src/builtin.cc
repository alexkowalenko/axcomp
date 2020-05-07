//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "builtin.hh"

#include <llvm/Support/Debug.h>

#include "typetable.hh"

namespace ax {

// builtin procedures
std::vector<std::pair<std::string, Symbol>> Builtin::global_functions;
llvm::StringMap<BIFunctor>                  Builtin::compile_functions;

#define DEBUG_TYPE "builtin"

template <typename... T> static void debug(const T &... msg) {
    LLVM_DEBUG(llvm::dbgs() << formatv(msg...) << '\n');
}

BIFunctor abs = [](CodeGenerator *codegen, ASTCallPtr ast) -> Value * {
    debug("builtin abs");
    auto  args = codegen->do_arguments(ast);
    auto *arg = args[0];
    if (arg->getType()->isIntegerTy()) {
        debug("builtin abs int");
        return codegen->call_function("ABS", TypeTable::IntType->get_llvm(), {arg});
    }
    if (arg->getType()->isFloatingPointTy()) {
        debug("builtin abs fabs");
        std::vector<llvm::Type *> type_args{TypeTable::RealType->get_llvm()};
        auto                      fun =
            Intrinsic::getDeclaration(codegen->get_module().get(), Intrinsic::fabs, type_args);
        return codegen->get_builder().CreateCall(fun, args);
    }
    return TypeTable::IntType->make_value(1);
};

BIFunctor len = [](CodeGenerator *codegen, ASTCallPtr ast) -> Value * {
    auto  args = codegen->do_arguments(ast);
    auto *arg = args[0];
    if (arg->getType()->isArrayTy()) {
        auto *array = dyn_cast<llvm::ArrayType>(arg->getType());
        return TypeTable::IntType->make_value(array->getArrayNumElements());
    }
    if (arg->getType()->isPointerTy()) {
        return codegen->call_function("Strings_Length", TypeTable::IntType->get_llvm(), {arg});
    }
    return TypeTable::IntType->make_value(1);
};

BIFunctor size = [](CodeGenerator *codegen, ASTCallPtr ast) -> Value * {
    auto name = std::string(*ast->args[0]);
    auto type = codegen->get_types().find(name);
    assert(type);

    return TypeTable::IntType->make_value(type->get_size());
};

BIFunctor min = [](CodeGenerator *codegen, ASTCallPtr ast) -> Value * {
    auto name = std::string(*ast->args[0]);
    auto type = codegen->get_types().find(name);
    assert(type);

    return type->min();
};

BIFunctor max = [](CodeGenerator *codegen, ASTCallPtr ast) -> Value * {
    auto name = std::string(*ast->args[0]);
    auto type = codegen->get_types().find(name);
    assert(type);

    return type->max();
};

BIFunctor inc = [](CodeGenerator *codegen, ASTCallPtr ast) -> Value * {
    auto   args = codegen->do_arguments(ast);
    auto * arg = args[0];
    Value *val = codegen->get_builder().CreateLoad(arg);
    val = codegen->get_builder().CreateAdd(val, TypeTable::IntType->make_value(1), "inc");
    return codegen->get_builder().CreateStore(val, arg);
};

BIFunctor dec = [](CodeGenerator *codegen, ASTCallPtr ast) -> Value * {
    auto   args = codegen->do_arguments(ast);
    auto * arg = args[0];
    Value *val = codegen->get_builder().CreateLoad(arg);
    val = codegen->get_builder().CreateSub(val, TypeTable::IntType->make_value(1), "dec");
    return codegen->get_builder().CreateStore(val, arg);
};

BIFunctor floor = [](CodeGenerator *codegen, ASTCallPtr ast) -> Value * {
    debug("builtin floor");
    auto                      args = codegen->do_arguments(ast);
    std::vector<llvm::Type *> type_args{TypeTable::RealType->get_llvm()};
    auto fun = Intrinsic::getDeclaration(codegen->get_module().get(), Intrinsic::floor, type_args);
    auto value = codegen->get_builder().CreateCall(fun, args);
    return codegen->get_builder().CreateFPToSI(value, TypeTable::IntType->get_llvm());
};

BIFunctor flt = [](CodeGenerator *codegen, ASTCallPtr ast) -> Value * {
    auto args = codegen->do_arguments(ast);
    return codegen->get_builder().CreateSIToFP(args[0], TypeTable::RealType->get_llvm());
};

void Builtin::initialise(SymbolFrameTable &symbols) {

    global_functions = {

        // Maths
        {"ABS", Symbol{std::make_shared<ProcedureType>(
                           TypeTable::AnyType,
                           ProcedureType::ParamsList{{TypeTable::VoidType, Attr::null}}),
                       Attr::compile_function}},

        {"ASH", Symbol{std::make_shared<ProcedureType>(
                           TypeTable::IntType,
                           ProcedureType::ParamsList{{TypeTable::IntType, Attr::null},
                                                     {TypeTable::IntType, Attr::null}}),
                       Attr::global_function}},
        {"ODD", Symbol{std::make_shared<ProcedureType>(TypeTable::BoolType,
                                                       ProcedureType::ParamsList{
                                                           {TypeTable::IntType, Attr::null},
                                                       }),
                       Attr::global_function}},
        {"FLOOR", Symbol{std::make_shared<ProcedureType>(TypeTable::IntType,
                                                         ProcedureType::ParamsList{
                                                             {TypeTable::RealType, Attr::null},
                                                         }),
                         Attr::compile_function}},
        {"FLT", Symbol{std::make_shared<ProcedureType>(TypeTable::RealType,
                                                       ProcedureType::ParamsList{
                                                           {TypeTable::IntType, Attr::null},
                                                       }),
                       Attr::compile_function}},

        {"INC", Symbol{std::make_shared<ProcedureType>(
                           TypeTable::VoidType,
                           ProcedureType::ParamsList{{TypeTable::IntType, Attr::var}}),
                       Attr::compile_function}},
        {"DEC", Symbol{std::make_shared<ProcedureType>(
                           TypeTable::VoidType,
                           ProcedureType::ParamsList{{TypeTable::IntType, Attr::var}}),
                       Attr::compile_function}},

        // CHARs
        {"CAP", Symbol{std::make_shared<ProcedureType>(
                           TypeTable::CharType,
                           ProcedureType::ParamsList{{TypeTable::CharType, Attr::null}}),
                       Attr::global_function}},
        {"CHR", Symbol{std::make_shared<ProcedureType>(
                           TypeTable::CharType,
                           ProcedureType::ParamsList{{TypeTable::IntType, Attr::null}}),
                       Attr::global_function}},
        {"ORD", Symbol{std::make_shared<ProcedureType>(
                           TypeTable::IntType,
                           ProcedureType::ParamsList{{TypeTable::CharType, Attr::null}}),
                       Attr::global_function}},

        // I/O
        {"WriteInt", Symbol{std::make_shared<ProcedureType>(
                                TypeTable::VoidType,
                                ProcedureType::ParamsList{{TypeTable::IntType, Attr::null}}),
                            Attr::global_function}},

        {"WriteBoolean", Symbol{std::make_shared<ProcedureType>(
                                    TypeTable::VoidType,
                                    ProcedureType::ParamsList{{TypeTable::BoolType, Attr::null}}),
                                Attr::global_function}},

        {"WriteLn",
         Symbol{std::make_shared<ProcedureType>(TypeTable::VoidType, ProcedureType::ParamsList{}),
                Attr::global_function}},

        // System
        {"HALT", Symbol{std::make_shared<ProcedureType>(
                            TypeTable::VoidType,
                            ProcedureType::ParamsList{{TypeTable::IntType, Attr::null}}),
                        Attr::global_function}},

        {"NEW", Symbol{std::make_shared<ProcedureType>(TypeTable::VoidType,
                                                       ProcedureType::ParamsList{
                                                           {TypeTable::StrType, Attr::var},
                                                           {TypeTable::IntType, Attr::null},
                                                       }),
                       Attr::global_function}},

        {"COPY", Symbol{std::make_shared<ProcedureType>(TypeTable::VoidType,
                                                        ProcedureType::ParamsList{
                                                            {TypeTable::StrType, Attr::null},
                                                            {TypeTable::StrType, Attr::var},
                                                        }),
                        Attr::global_function}},

        // Compile time Functions
        {"LEN", Symbol{std::make_shared<ProcedureType>(
                           TypeTable::IntType,
                           ProcedureType::ParamsList{{TypeTable::VoidType, Attr::null}}),
                       Attr::compile_function}},

        {"SIZE", Symbol{std::make_shared<ProcedureType>(
                            TypeTable::IntType,
                            ProcedureType::ParamsList{{TypeTable::VoidType, Attr::null}}),
                        Attr::compile_function}},

        {"MIN", Symbol{std::make_shared<ProcedureType>(
                           TypeTable::AnyType,
                           ProcedureType::ParamsList{{TypeTable::VoidType, Attr::null}}),
                       Attr::compile_function}},

        {"MAX", Symbol{std::make_shared<ProcedureType>(
                           TypeTable::AnyType,
                           ProcedureType::ParamsList{{TypeTable::VoidType, Attr::null}}),
                       Attr::compile_function}},

    };

    std::for_each(begin(global_functions), end(global_functions),
                  [&symbols](auto &f) { symbols.put(f.first, mkSym(f.second)); });

    compile_functions.try_emplace("LEN", len);
    compile_functions.try_emplace("SIZE", size);
    compile_functions.try_emplace("MIN", min);
    compile_functions.try_emplace("MAX", max);
    compile_functions.try_emplace("INC", inc);
    compile_functions.try_emplace("DEC", dec);
    compile_functions.try_emplace("ABS", abs);
    compile_functions.try_emplace("FLOOR", floor);
    compile_functions.try_emplace("FLT", flt);
}

} // namespace ax