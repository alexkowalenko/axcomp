//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "builtin.hh"

#include <llvm/Support/Debug.h>
#include <memory>

#include "codegen.hh"
#include "type.hh"
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
    debug("builtin ABS");
    auto  args = codegen->do_arguments(ast);
    auto *arg = args[0];
    if (arg->getType()->isIntegerTy()) {
        debug("builtin abs int");
        return codegen->call_function("ABS", TypeTable::IntType->get_llvm(), {arg});
    }
    if (arg->getType()->isFloatingPointTy()) {
        debug("builtin abs fabs");
        std::vector<llvm::Type *> type_args{TypeTable::RealType->get_llvm()};
        auto *                    fun =
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
    debug("builtin SIZE {0}", name);
    auto type = codegen->get_types().find(name);
    if (type) {
        return TypeTable::IntType->make_value(type->get_size());
    }
    auto ty = ast->args[0]->get_type();
    return TypeTable::IntType->make_value(ty->get_size());
};

BIFunctor newfunct = [](CodeGenerator *codegen, ASTCallPtr ast) -> Value * {
    debug("builtin NEW");
    auto args = codegen->do_arguments(ast);
    if (ast->args.size() > 1 && ast->args[0]->get_type() == TypeTable::StrType) {
        debug("builtin NEW STRING");
        return codegen->call_function("NEW_String", TypeTable::IntType->get_llvm(),
                                      {args[0], args[1]});
    }
    if (ast->args.size() == 1 && ast->args[0]->get_type()->id == TypeId::pointer) {
        debug("builtin NEW POINTER");
        auto ptr_type = std::dynamic_pointer_cast<ax::PointerType>(ast->args[0]->get_type());
        auto size = ptr_type->get_reference()->get_size();
        return codegen->call_function("NEW_ptr", TypeTable::IntType->get_llvm(),
                                      {args[0], TypeTable::IntType->make_value(size)});
    }
    if (ast->args.size() > 1 && ast->args[0]->get_type()->id == TypeId::array) {
        debug("builtin NEW ARRAY");

        auto   array_type = std::dynamic_pointer_cast<ArrayType>(ast->args[0]->get_type());
        auto   base_size = array_type->base_type->get_size();
        Value *value = TypeTable::IntType->make_value(base_size);
        for (int i = 1; i < args.size(); i++) {
            value = codegen->get_builder().CreateMul(args[i], value);
        }
        return codegen->call_function("NEW_Array", TypeTable::IntType->get_llvm(),
                                      {args[0], value});
    }
    throw CodeGenException(llvm::formatv("Variable with type {0} passed to NEW",
                                         ast->args[0]->get_type()->get_name()),
                           ast->get_location());
};

BIFunctor min = [](CodeGenerator *codegen, ASTCallPtr ast) -> Value * {
    auto name = std::string(*ast->args[0]);
    auto type = codegen->get_types().resolve(name);
    assert(type);

    return (*type)->min();
};

BIFunctor max = [](CodeGenerator *codegen, ASTCallPtr ast) -> Value * {
    auto name = std::string(*ast->args[0]);
    auto type = codegen->get_types().resolve(name);
    assert(type);

    return (*type)->max();
};

std::tuple<Value *, Value *, Value *> do_incdec(CodeGenerator *codegen, ASTCallPtr ast,
                                                std::string const &name) {
    debug("builtin INC/DEC");
    auto  args = codegen->do_arguments(ast);
    auto *arg = args[0];
    if (arg->getType()->isPointerTy() && arg->getType()->getPointerElementType()->isIntegerTy()) {
        debug("builtin INC/DEC 2");
        Value *val = codegen->get_builder().CreateLoad(arg);
        Value *inc;
        if (args.size() == 1) {
            inc = TypeTable::IntType->make_value(1);
        } else {
            if (args[1]->getType()->isIntegerTy()) {
                inc = args[1];
            } else {
                throw CodeGenException(llvm::formatv("Type {0} passed to INC as increment",
                                                     ast->args[1]->get_type()->get_name()),
                                       ast->get_location());
            }
        }
        return {arg, val, inc};
    }
    throw CodeGenException(
        llvm::formatv("Type {0} passed to {1}", ast->args[0]->get_type()->get_name(), name),
        ast->get_location());
}

BIFunctor inc = [](CodeGenerator *codegen, ASTCallPtr ast) -> Value * {
    auto [arg, val, inc] = do_incdec(codegen, ast, "INC");
    val = codegen->get_builder().CreateAdd(val, inc, "inc");
    return codegen->get_builder().CreateStore(val, arg);
};

BIFunctor dec = [](CodeGenerator *codegen, ASTCallPtr ast) -> Value * {
    auto [arg, val, inc] = do_incdec(codegen, ast, "DEC");
    val = codegen->get_builder().CreateSub(val, inc, "dec");
    return codegen->get_builder().CreateStore(val, arg);
};

BIFunctor floor = [](CodeGenerator *codegen, ASTCallPtr ast) -> Value * {
    debug("builtin FLOOR");
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

BIFunctor assert = [](CodeGenerator *codegen, ASTCallPtr ast) -> Value * {
    debug("builtin ASSERT");
    auto  args = codegen->do_arguments(ast);
    auto *arg = args[0];

    auto *      funct = codegen->get_builder().GetInsertBlock()->getParent();
    BasicBlock *assert_block = BasicBlock::Create(codegen->get_context(), "assert", funct);
    BasicBlock *merge_block = BasicBlock::Create(codegen->get_context(), "merge");

    auto val = codegen->get_builder().CreateNot(arg);
    codegen->get_builder().CreateCondBr(val, assert_block, merge_block);

    codegen->get_builder().SetInsertPoint(assert_block);
    Value *ret = TypeTable::IntType->make_value(1);
    if (ast->args.size() > 1) {
        ret = args[1];
    }
    codegen->call_function("HALT", TypeTable::IntType->get_llvm(), {ret});
    codegen->get_builder().CreateBr(merge_block);

    funct->getBasicBlockList().push_back(merge_block);
    codegen->get_builder().SetInsertPoint(merge_block);
    return val;
};

BIFunctor long_func = [](CodeGenerator *codegen, ASTCallPtr ast) -> Value * {
    auto  args = codegen->do_arguments(ast);
    auto *arg = args[0];
    if (arg->getType()->isIntegerTy() || arg->getType()->isFloatingPointTy()) {
        return args[0];
    }
    throw CodeGenException(
        llvm::formatv("Type {0} passed to LONG", ast->args[0]->get_type()->get_name()),
        ast->get_location());
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
        {"ENTIER", Symbol{std::make_shared<ProcedureType>(TypeTable::IntType,
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
                           ProcedureType::ParamsList{{TypeTable::AnyType, Attr::var}}),
                       Attr::compile_function}},
        {"DEC", Symbol{std::make_shared<ProcedureType>(
                           TypeTable::VoidType,
                           ProcedureType::ParamsList{{TypeTable::AnyType, Attr::var}}),
                       Attr::compile_function}},

        {"LONG", Symbol{std::make_shared<ProcedureType>(
                            TypeTable::AnyType,
                            ProcedureType::ParamsList{{TypeTable::VoidType, Attr::null}}),
                        Attr::compile_function}},

        {"SHORT", Symbol{std::make_shared<ProcedureType>(
                             TypeTable::AnyType,
                             ProcedureType::ParamsList{{TypeTable::VoidType, Attr::null}}),
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

        {"ASSERT", Symbol{std::make_shared<ProcedureType>(
                              TypeTable::VoidType,
                              ProcedureType::ParamsList{{TypeTable::AnyType, Attr::null}}),
                          Attr::compile_function}},

        {"NEW", Symbol{std::make_shared<ProcedureType>(TypeTable::VoidType,
                                                       ProcedureType::ParamsList{
                                                           {TypeTable::AnyType, Attr::var},
                                                       }),
                       Attr::compile_function}},

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
    compile_functions.try_emplace("ENTIER", floor);
    compile_functions.try_emplace("FLT", flt);
    compile_functions.try_emplace("NEW", newfunct);
    compile_functions.try_emplace("ASSERT", assert);
    compile_functions.try_emplace("LONG", long_func);
    compile_functions.try_emplace("SHORT", long_func);
}

} // namespace ax