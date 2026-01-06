//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <memory>
#include <string>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshadow"
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wunused-parameter"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#pragma clang diagnostic pop

#include "ast.hh"
#include "astvisitor.hh"
#include "importer.hh"
#include "options.hh"
#include "symboltable.hh"
#include "type.hh"
#include "typetable.hh"

using namespace llvm;

namespace ax {

class CodeGenerator : ASTVisitor {
  public:
    CodeGenerator(Options &o, SymbolFrameTable &s, TypeTable &t, Importer &i);

    void generate(ASTModule const &ast) { ast->accept(this); };

    void setup_builtins();

    void generate_objectcode();
    void generate_llcode();

    void optimize();

    TypeTable &get_types() { return types; };

    std::vector<Value *> do_arguments(ASTCall const &ast);
    Value               *call_function(std::string const &name, llvm::Type *ret,
                                       std::vector<Value *> const &args);

    IRBuilder<>             &get_builder() { return builder; };
    std::unique_ptr<Module> &get_module() { return module; };
    LLVMContext             &get_context() { return context; };

  private:
    void visit_ASTModule(ASTModule ast) override;
    void visit_ASTImport(ASTImport ast) override;
    void doTopDecs(ASTDeclaration const &ast);
    void doTopVars(ASTVar const &ast);
    void doTopConsts(ASTConst const &ast);

    void visit_ASTDeclaration(ASTDeclaration ast) override;
    void visit_ASTConst(ASTConst ast) override;
    void visit_ASTVar(ASTVar ast) override;

    void doProcedures(std::vector<ASTProc> const &procs);

    void visit_ASTProcedure(ASTProcedure ast) override;
    void visit_ASTProcedureForward(ASTProcedureForward ast) override;

    void visit_ASTAssignment(ASTAssignment ast) override;
    void visit_ASTReturn(ASTReturn ast) override;
    void visit_ASTExit(ASTExit ast) override;

    std::tuple<std::shared_ptr<ProcedureType>, std::string, bool> do_find_proc(ASTCall const &ast);

    void visit_ASTCall(ASTCall ast) override;
    void visit_ASTIf(ASTIf ast) override;
    void visit_ASTCase(ASTCase ast) override;
    void visit_ASTFor(ASTFor ast) override;
    void visit_ASTWhile(ASTWhile ast) override;
    void visit_ASTRepeat(ASTRepeat ast) override;
    void visit_ASTLoop(ASTLoop ast) override;
    void visit_ASTBlock(ASTBlock ast) override;
    void visit_ASTExpr(ASTExpr ast) override;
    void visit_ASTSimpleExpr(ASTSimpleExpr ast) override;
    void visit_ASTTerm(ASTTerm ast) override;
    void visit_ASTFactor(ASTFactor ast) override;
    void visit_ASTRange_value(ASTRange const &ast, Value *case_value);
    void get_index(ASTDesignator const &ast);
    void visit_ASTDesignatorPtr(ASTDesignator const &ast, bool ptr);
    void visit_ASTQualidentPtr(ASTQualident const &ast, bool ptr);
    void visit_ASTIdentifierPtr(ASTIdentifier const &ast, bool ptr);
    void visit_ASTSet(ASTSet ast) override;
    void visit_ASTInteger(ASTInteger ast) override;
    void visit_ASTReal(ASTReal ast) override;
    void visit_ASTString(ASTString ast) override;
    void visit_ASTChar(ASTCharPtr ast) override;
    void visit_ASTBool(ASTBool ast) override;
    void visit_ASTNil(ASTNil /*not used*/) override;

    void init();

    AllocaInst *createEntryBlockAlloca(Function *TheFunction, std::string const &name,
                                       ASTType type, bool var = false);

    static AllocaInst *createEntryBlockAlloca(Function *function, std::string const &name,
                                              llvm::Type *type);

    Type        resolve_type(ASTType const &t);
    llvm::Type *getType(ASTType const &type);
    Constant   *getType_init(ASTType const &type);

    void ejectBranch(std::vector<ASTStatement> const &stats, BasicBlock *block, BasicBlock *where);

    [[nodiscard]] std::string gen_module_id(std::string const &id) const;
    std::string               get_nested_name();

    GlobalVariable *generate_global(std::string const &name, llvm::Type *t);
    FunctionCallee  generate_function(std::string const &name, llvm::Type *t,
                                      llvm::ArrayRef<llvm::Type *> const &params);

    Options                 &options;
    SymbolFrameTable        &symboltable;
    TypeTable               &types;
    Importer                &importer;
    std::vector<std::string> nested_procs{};

    std::string             module_name;
    std::string             filename;
    LLVMContext             context;
    IRBuilder<>             builder;
    std::unique_ptr<Module> module;

    bool        top_level{true};        // am I in the top level of the module?
    Value      *last_value{nullptr};    // holds last value of compilation
    BasicBlock *last_end{nullptr};      // last end block in loop, used for EXIT
    bool        do_strchar_conv{false}; // Convert STRING1 to CHAR

    int                               string_const{0}; // const_strings counter
    llvm::StringMap<GlobalVariable *> global_strings;

    bool is_var{false}; // Do VAR change in IndentifierPtr
};

} // namespace ax