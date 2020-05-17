//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <memory>
#include <string>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include "astvisitor.hh"
#include "importer.hh"
#include "options.hh"
#include "symboltable.hh"
#include "typetable.hh"

using namespace llvm;

namespace ax {

class CodeGenerator : ASTVisitor {
  public:
    explicit CodeGenerator(Options &o, SymbolFrameTable &s, TypeTable &t, Importer &i);

    void generate(ASTModulePtr const &ast) { ast->accept(this); };

    void setup_builtins();

    void generate_objectcode();
    void generate_llcode();

    void visit_ASTModule(ASTModulePtr ast) override;
    void visit_ASTImport(ASTImportPtr ast) override;
    void doTopDecs(ASTDeclarationPtr const &ast);
    void doTopVars(ASTVarPtr const &ast);
    void doTopConsts(ASTConstPtr const &ast);

    void visit_ASTDeclaration(ASTDeclarationPtr ast) override;
    void visit_ASTConst(ASTConstPtr ast) override;
    void visit_ASTVar(ASTVarPtr ast) override;

    void doProcedures(std::vector<ASTProcedurePtr> const &procs);

    void visit_ASTProcedure(ASTProcedurePtr ast) override;
    void visit_ASTAssignment(ASTAssignmentPtr ast) override;
    void visit_ASTReturn(ASTReturnPtr ast) override;
    void visit_ASTExit(ASTExitPtr ast) override;

    std::vector<Value *> do_arguments(ASTCallPtr ast);

    void visit_ASTCall(ASTCallPtr ast) override;
    void visit_ASTIf(ASTIfPtr ast) override;
    void visit_ASTCase(ASTCasePtr ast) override;
    void visit_ASTFor(ASTForPtr ast) override;
    void visit_ASTWhile(ASTWhilePtr ast) override;
    void visit_ASTRepeat(ASTRepeatPtr ast) override;
    void visit_ASTLoop(ASTLoopPtr ast) override;
    void visit_ASTBlock(ASTBlockPtr ast) override;
    void visit_ASTExpr(ASTExprPtr ast) override;
    void visit_ASTSimpleExpr(ASTSimpleExprPtr ast) override;
    void visit_ASTTerm(ASTTermPtr ast) override;
    void visit_ASTFactor(ASTFactorPtr ast) override;
    void visit_ASTRange_value(ASTRangePtr ast, Value *case_value);
    void get_index(ASTDesignatorPtr const &ast);
    void visit_ASTDesignator(ASTDesignatorPtr ast) override;
    void visit_ASTDesignatorPtr(ASTDesignatorPtr const &ast);
    void visit_ASTQualident(ASTQualidentPtr ast) override;
    void visit_ASTQualidentPtr(ASTQualidentPtr const &ast);
    void visit_ASTIdentifier(ASTIdentifierPtr ast) override;
    void visit_ASTIdentifierPtr(ASTIdentifierPtr const &ast);
    void visit_ASTInteger(ASTIntegerPtr ast) override;
    void visit_ASTReal(ASTRealPtr ast) override;
    void visit_ASTString(ASTStringPtr ast) override;
    void visit_ASTChar(ASTCharPtr ast) override;
    void visit_ASTBool(ASTBoolPtr ast) override;
    void visit_ASTNil(ASTNilPtr /*not used*/) override;

    TypeTable &get_types() { return types; };

    Value *call_function(std::string const &name, llvm::Type *ret,
                         std::vector<Value *> const &args);

    IRBuilder<> &            get_builder() { return builder; };
    std::unique_ptr<Module> &get_module() { return module; };

  private:
    void init();

    AllocaInst *createEntryBlockAlloca(Function *TheFunction, std::string const &name,
                                       ASTTypePtr type, bool var = false);

    static AllocaInst *createEntryBlockAlloca(Function *function, std::string const &name,
                                              llvm::Type *type);

    TypePtr     resolve_type(ASTTypePtr const &t);
    llvm::Type *getType(ASTTypePtr const &type);
    Constant *  getType_init(ASTTypePtr const &type);

    [[nodiscard]] std::string gen_module_id(std::string const &id) const;

    Options &         options;
    SymbolFrameTable &symboltable;
    TypeTable &       types;
    Importer &        importer;

    using ValueSymbolTable = std::shared_ptr<SymbolTable<std::pair<Value *, Attr>>>;

    std::string             module_name;
    std::string             filename;
    LLVMContext             context;
    IRBuilder<>             builder;
    std::unique_ptr<Module> module;

    bool        top_level{true};     // am I in the top level of the module?
    Value *     last_value{nullptr}; // holds last value of compilation
    BasicBlock *last_end{nullptr};   // last end block in loop, used for EXIT
    bool        has_return{false};
    int         string_const{0}; // const_strings counter

    bool is_var{false}; // Do VAR change in IndentifierPtr
};

} // namespace ax