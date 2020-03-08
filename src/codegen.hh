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
#include "options.hh"
#include "symboltable.hh"
#include "typetable.hh"

using namespace llvm;

namespace ax {

class CodeGenerator : ASTVisitor {
  public:
    explicit CodeGenerator(Options &o, TypeTable &t);

    void generate(std::shared_ptr<ASTModule> const &ast) {
        visit_ASTModule(ast.get());
    };

    void visit_ASTModule(ASTModule *ast) override;

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
    void visit_ASTCall(ASTCall *ast) override;
    void visit_ASTExpr(ASTExpr *ast) override;
    void visit_ASTSimpleExpr(ASTSimpleExpr *ast) override;
    void visit_ASTTerm(ASTTerm *ast) override;
    void visit_ASTFactor(ASTFactor *ast) override;
    void visit_ASTIdentifier(ASTIdentifier *ast) override;
    void visit_ASTInteger(ASTInteger *ast) override;
    void visit_ASTBool(ASTBool *ast) override;

  private:
    void init(std::string const &module_name);
    void generate_objectcode();
    void print_code();

    AllocaInst *createEntryBlockAlloca(Function *         TheFunction,
                                       std::string const &name,
                                       std::string const &type);

    llvm::Type *getType(std::string const &t);

    Options &                             options;
    TypeTable &                           types;
    std::shared_ptr<SymbolTable<Value *>> top_symboltable;
    std::shared_ptr<SymbolTable<Value *>> current_symboltable;

    std::string             filename;
    LLVMContext             context;
    IRBuilder<>             builder;
    std::unique_ptr<Module> module;

    bool   top_level = true; // am I in the top level of the module?
    Value *last_value;       // holds last value of compilation
};

} // namespace ax