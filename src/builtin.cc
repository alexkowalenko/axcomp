//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "builtin.hh"

#include "typetable.hh"

namespace ax {

// builtin procedures
std::vector<std::pair<std::string, Symbol>> Builtin::global_functions;
llvm::StringMap<BIFunctor>                  Builtin::compile_functions;

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

void Builtin::initialise(SymbolFrameTable &symbols) {

    global_functions = {

        // Maths
        {"ABS", Symbol{std::make_shared<ProcedureType>(
                           TypeTable::IntType,
                           ProcedureType::ParamsList{{TypeTable::IntType, Attr::null}}),
                       Attr::global_function}},

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
}

} // namespace ax