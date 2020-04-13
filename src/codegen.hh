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
    explicit CodeGenerator(Options &o, TypeTable &t, Importer &i);

    void generate(std::shared_ptr<ASTModule> const &ast) {
        visit_ASTModule(ast.get());
    };

    void setup_builtins();

    void visit_ASTModule(ASTModule *ast) override;
    void visit_ASTImport(ASTImport *ast) override;
    void doTopDecs(ASTDeclaration *ast);
    void doTopVars(ASTVar *ast);
    void doTopConsts(ASTConst *ast);

    void visit_ASTDeclaration(ASTDeclaration *ast) override;
    void visit_ASTConst(ASTConst *ast) override;
    void visit_ASTVar(ASTVar *ast) override;

    void doProcedures(std::vector<std::shared_ptr<ASTProcedure>> const &procs);

    void visit_ASTProcedure(ASTProcedure *ast) override;
    void visit_ASTAssignment(ASTAssignment *ast) override;
    void visit_ASTReturn(ASTReturn *ast) override;
    void visit_ASTExit(ASTExit *ast) override;
    void visit_ASTCall(ASTCall *ast) override;
    void visit_ASTIf(ASTIf *ast) override;
    void visit_ASTFor(ASTFor *ast) override;
    void visit_ASTWhile(ASTWhile *ast) override;
    void visit_ASTRepeat(ASTRepeat *ast) override;
    void visit_ASTLoop(ASTLoop *ast) override;
    void visit_ASTBlock(ASTBlock *ast) override;
    void visit_ASTExpr(ASTExpr *ast) override;
    void visit_ASTSimpleExpr(ASTSimpleExpr *ast) override;
    void visit_ASTTerm(ASTTerm *ast) override;
    void visit_ASTFactor(ASTFactor *ast) override;
    void get_index(ASTDesignator *ast);
    void visit_ASTDesignator(ASTDesignator *ast) override;
    void visit_ASTDesignatorPtr(ASTDesignator *ast);
    void visit_ASTQualident(ASTQualident *ast) override;
    void visit_ASTIdentifier(ASTIdentifier *ast) override;
    void visit_ASTIdentifierPtr(ASTIdentifier *ast);
    void visit_ASTInteger(ASTInteger *ast) override;
    void visit_ASTBool(ASTBool *ast) override;

    void generate_objectcode();
    void generate_llcode();

  private:
    void init();

    AllocaInst *createEntryBlockAlloca(Function *               TheFunction,
                                       std::string const &      name,
                                       std::shared_ptr<ASTType> type,
                                       bool                     var = false);

    AllocaInst *createEntryBlockAlloca(Function *         function,
                                       std::string const &name,
                                       llvm::Type *       type);

    TypePtr     resolve_type(std::shared_ptr<ASTType> const &t);
    llvm::Type *getType(std::shared_ptr<ASTType> const &type);
    Constant *  getType_init(std::shared_ptr<ASTType> const &type);

    bool        find_var_Identifier(ASTDesignator *ast);
    std::string gen_module_id(std::string const &id) const;

    Options &  options;
    TypeTable &types;
    Importer & importer;

    using ValueSymbolTable =
        std::shared_ptr<SymbolTable<std::pair<Value *, Attr>>>;

    ValueSymbolTable top_symboltable;
    ValueSymbolTable current_symboltable;

    std::string             module_name;
    std::string             filename;
    LLVMContext             context;
    IRBuilder<>             builder;
    std::unique_ptr<Module> module;

    bool        top_level{true};   // am I in the top level of the module?
    Value *     last_value;        // holds last value of compilation
    BasicBlock *last_end{nullptr}; // last end block in loop, used for EXIT
    bool        has_return{false};

    bool is_var{false}; // Do VAR change in IndentifierPtr
};

} // namespace ax