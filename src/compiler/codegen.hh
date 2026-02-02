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
    Value               *eval_expr(ASTExpr const &expr);

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

    AllocaInst *createEntryBlockAlloca(Function *function, std::string const &name,
                                       const ASTType &type, bool var = false);

    static AllocaInst *createEntryBlockAlloca(Function *function, std::string const &name,
                                              llvm::Type *type);

    Type        resolve_type(ASTType const &t) const;
    llvm::Type *getType(ASTType const &type) const;
    Constant   *getType_init(ASTType const &type) const;
    llvm::Type *llvm_type(Type const &type) const;
    Constant   *llvm_init(Type const &type) const;

    Value *load_value(Value *addr, llvm::Type *type, const char *name = nullptr);
    void   store_value(Value *addr, Value *value);
    Value *load_var_param(SymbolPtr const &sym, bool want_address, std::string const &name);
    Value *emit_designator_address(ASTDesignator const &ast, bool var_param = false);
    Value *emit_designator_value(ASTDesignator const &ast);
    Value *emit_qualident_address(ASTQualident const &ast);
    Value *emit_identifier_address(ASTIdentifier const &ast);

    void      build_procedure_proto(ASTProcedure const &ast, SymbolPtr const &sym,
                                    std::vector<llvm::Type *> &proto,
                                    std::vector<std::pair<int, llvm::Attribute::AttrKind>> &argAttr,
                                    llvm::Type *&closure_type, llvm::Type *&returnType);
    Function *create_procedure_function(ASTProcedure const &ast,
                                        std::vector<llvm::Type *> const &proto,
                                        std::vector<std::pair<int, llvm::Attribute::AttrKind>> const
                                            &argAttr,
                                        llvm::Type *returnType);
    void      setup_procedure_params(ASTProcedure const &ast, SymbolPtr const &sym, Function *f,
                                     llvm::Type *closure_type);

    BasicBlock *make_block(std::string const &name, Function *funct = nullptr,
                           bool attach = true);
    void        insert_block(BasicBlock *block, Function *funct = nullptr);
    void        set_block(BasicBlock *block);
    void        emit_br(BasicBlock *target);
    void        emit_cond_br(Value *cond, BasicBlock *then_block, BasicBlock *else_block);
    BasicBlock *emit_short_circuit_step(Value *cond, BasicBlock *end_block,
                                        const char *next_name, bool is_or);
    void        finalize_short_circuit(BasicBlock *end_block,
                                       std::vector<std::pair<BasicBlock *, Value *>> &incoming,
                                       const char *phi_name);
    bool        try_short_circuit(TokenType op, TokenType target, BasicBlock *end_block,
                                  std::vector<std::pair<BasicBlock *, Value *>> &incoming,
                                  const char *next_name, bool is_or,
                                  std::function<void()> eval_right, Value *&L);
    Value      *emit_index_i32(ASTSimpleExpr const &expr);
    std::vector<Value *> build_index_list(ArrayRef const &indices, bool with_leading_zero);

    void ejectBranch(std::vector<ASTStatement> const &stats, BasicBlock *block, BasicBlock *where);

    [[nodiscard]] std::string gen_module_id(std::string const &id) const;
    std::string               get_nested_name() const;

    GlobalVariable *generate_global(std::string const &name, llvm::Type *t) const;
    FunctionCallee  generate_function(std::string const &name, llvm::Type *return_type,
                                      llvm::ArrayRef<llvm::Type *> const &params) const;
    GlobalVariable *emit_global(const std::string &name, llvm::Type *type,
                                llvm::Constant *init, bool is_const,
                                GlobalValue::LinkageTypes linkage) const;

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
