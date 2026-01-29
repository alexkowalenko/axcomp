//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <memory>
#include <stack>
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

#include "ast/all.hh"
#include "astvisitor.hh"
#include "importer.hh"
#include "options.hh"
#include "symboltable.hh"
#include "types/all.hh"
#include "typetable.hh"

using namespace llvm;

namespace ax {

class CodeGenerator : ASTVisitor {
  public:
    CodeGenerator(Options &o, SymbolFrameTable &s, TypeTable &t, Importer &i);

    void generate(ASTModule const &ast) { ast->accept(this); };

    void setup_builtins() const;

    void generate_objectcode() const;
    void generate_llcode() const;

    void optimize() const;

    TypeTable &get_types() const { return types; };

    std::vector<Value *> do_arguments(ASTCall const &ast);
    Value               *call_function(std::string const &name, llvm::Type *ret,
                                       std::vector<Value *> const &args);

    IRBuilder<>             &get_builder() { return builder; };
    std::unique_ptr<Module> &get_module() { return module; };
    LLVMContext             &get_context() { return context; };

  private:
    void visit(ASTModule const &ast) override;
    void visit(ASTImport const &ast) override;
    void doTopDecs(ASTDeclaration const &ast);
    void doTopVars(ASTVar const &ast) const;
    void doTopConsts(ASTConst const &ast);

    void visit(ASTDeclaration const &ast) override;
    void visit(ASTConst const &ast) override;
    void visit(ASTVar const &ast) override;

    void doProcedures(std::vector<ASTProc> const &procs);

    void visit(ASTProcedure const &ast) override;
    void visit(ASTProcedureForward const &ast) override;

    void visit(ASTAssignment const &ast) override;
    void visit(ASTReturn const &ast) override;
    void visit(ASTExit const &ast) override;

    std::tuple<std::shared_ptr<ProcedureType>, std::string, bool>
    do_find_proc(ASTCall const &ast) const;

    void visit(ASTCall const &ast) override;
    void visit(ASTIf const &ast) override;
    void visit(ASTCase const &ast) override;
    void visit(ASTFor const &ast) override;
    void visit(ASTWhile const &ast) override;
    void visit(ASTRepeat const &ast) override;
    void visit(ASTLoop const &ast) override;
    void visit(ASTBlock const &ast) override;
    void visit(ASTExpr const &ast) override;
    void visit(ASTSimpleExpr const &ast) override;
    void visit(ASTTerm const &ast) override;
    void visit(ASTFactor const &ast) override;
    void visit_value(ASTRange const &ast, Value *case_value);
    void get_index(ASTDesignator const &ast);
    void visitPtr(ASTDesignator const &ast, bool ptr);
    void visitPtr(ASTQualident const &ast, bool ptr);
    void visitPtr(ASTIdentifier const &ast, bool ptr);
    void visit(ASTSet const &ast) override;
    void visit(ASTInteger const &ast) override;
    void visit(ASTReal const &ast) override;
    void visit(ASTString const &ast) override;
    void visit(ASTCharPtr const &ast) override;
    void visit(ASTBool const &ast) override;
    void visit(ASTNil const & /*not used*/) override;

    void init();

    AllocaInst *createEntryBlockAlloca(Function *TheFunction, std::string const &name,
                                       const ASTType &type, bool var = false);

    static AllocaInst *createEntryBlockAlloca(Function *function, std::string const &name,
                                              llvm::Type *type);

    Type        resolve_type(ASTType const &t) const;
    llvm::Type *getType(ASTType const &type) const;
    Constant   *getType_init(ASTType const &type) const;

    void ejectBranch(std::vector<ASTStatement> const &stats, BasicBlock *block, BasicBlock *where);

    [[nodiscard]] std::string gen_module_id(std::string const &id) const;
    std::string               get_nested_name() const;

    GlobalVariable *generate_global(std::string const &name, llvm::Type *t) const;
    FunctionCallee  generate_function(std::string const &name, llvm::Type *return_type,
                                      llvm::ArrayRef<llvm::Type *> const &params) const;

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

    bool                     top_level{true};        // am I in the top level of the module?
    Value                   *last_value{nullptr};    // holds last value of compilation
    std::stack<BasicBlock *> last_end;               // last end block in loop, used for EXIT
    bool                     do_strchar_conv{false}; // Convert STRING1 to CHAR

    int                               string_const{0}; // const_strings counter
    llvm::StringMap<GlobalVariable *> global_strings;

    bool is_var{false}; // Do VAR change in IndentifierPtr
};

} // namespace ax
