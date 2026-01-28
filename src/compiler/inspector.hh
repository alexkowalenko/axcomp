//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include "ast/all.hh"
#include "astvisitor.hh"
#include "error.hh"
#include "importer.hh"
#include "symboltable.hh"
#include "types/all.hh"
#include "typetable.hh"

namespace ax {

class Inspector : ASTVisitor {

  public:
    explicit Inspector(SymbolFrameTable &s, TypeTable &t, ErrorManager &e, Importer &i);

    void check(ASTModule const &ast);

  private:
    void visit(ASTModule const &ast) override;
    void visit(ASTDeclaration const &ast) override;
    void visit(ASTImport const &ast) override;
    void visit(ASTConst const &ast) override;
    void visit(ASTTypeDec const &ast) override;
    void visit(ASTVar const &ast) override;

    void                                       do_receiver(const RecVar &r) const;
    std::pair<Type, ProcedureType::ParamsList> do_proc(const ASTProc_ &ast);

    void visit(ASTProcedure const &ast) override;
    void visit(ASTProcedureForward const &ast) override;

    void visit(ASTAssignment const &ast) override;
    void visit(ASTReturn const &ast) override;
    void visit(ASTCall const &ast) override;
    void visit(ASTIf const &ast) override;
    void visit(ASTCaseElement const &ast) override;
    void visit(ASTCase const &ast) override;
    void visit(ASTFor const &ast) override;
    void visit(ASTWhile const &ast) override;
    void visit(ASTRepeat const &ast) override;
    void visit(ASTLoop const &ast) override;
    void visit(ASTBlock const &ast) override;
    void visit(ASTSimpleExpr const &ast) override;
    void visit(ASTExpr const &ast) override;
    void visit(ASTTerm const &ast) override;
    void visit(ASTFactor const &ast) override;
    void visit(ASTDesignator const &ast) override;
    void visit(ASTType const &ast) override;
    void visit(ASTArray const &ast) override;
    void visit(ASTRecord const &ast) override;
    void visit(ASTPointerType const &ast) override;
    void visit(ASTQualident const &ast) override;
    void visit(ASTIdentifier const &ast) override;
    void visit(ASTSet const &ast) override;
    void visit(ASTInteger const &ast) override;
    void visit(ASTReal const &ast) override;
    void visit(ASTCharPtr const &ast) override;
    void visit(ASTString const &ast) override;
    void visit(ASTBool const &ast) override;
    void visit(ASTNil const & /*not used*/) override;

    std::string get_Qualident(ASTQualident const &ast) const;

    SymbolFrameTable &symboltable;
    TypeTable        &types;
    ErrorManager     &errors;
    Importer         &importer;

    bool         is_const{false};
    bool         is_lvalue{false};
    bool         is_qualid{false};
    bool         qualid_error{false};
    Type         last_type{nullptr};
    ASTProcedure last_proc{nullptr};

    Attr variable_type{Attr::null};

    // Post process
    std::vector<std::shared_ptr<PointerType>> pointer_types;
};

} // namespace ax
