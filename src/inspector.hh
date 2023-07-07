//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <iostream>

#include "ast.hh"
#include "astvisitor.hh"
#include "error.hh"
#include "importer.hh"
#include "symboltable.hh"
#include "type.hh"
#include "typetable.hh"

namespace ax {

class Inspector : ASTVisitor {

  public:
    explicit Inspector(SymbolFrameTable &s, TypeTable &t, ErrorManager &e, Importer &i);

    void check(ASTModule const &ast);

  private:
    void visit_ASTModule(ASTModule ast) override;
    void visit_ASTImport(ASTImport ast) override;
    void visit_ASTConst(ASTConst ast) override;
    void visit_ASTTypeDec(ASTTypeDec ast) override;
    void visit_ASTVar(ASTVar ast) override;

    void                                          do_receiver(RecVar &r);
    std::pair<TypePtr, ProcedureType::ParamsList> do_proc(ASTProc_ &ast);

    void visit_ASTProcedure(ASTProcedure ast) override;
    void visit_ASTProcedureForward(ASTProcedureForward ast) override;

    void visit_ASTAssignment(ASTAssignment ast) override;
    void visit_ASTReturn(ASTReturn ast) override;
    void visit_ASTCall(ASTCall ast) override;
    void visit_ASTIf(ASTIf ast) override;
    void visit_ASTCaseElement(ASTCaseElement ast) override;
    void visit_ASTCase(ASTCase ast) override;
    void visit_ASTFor(ASTFor ast) override;
    void visit_ASTWhile(ASTWhile ast) override;
    void visit_ASTRepeat(ASTRepeat ast) override;
    void visit_ASTLoop(ASTLoop ast) override;
    void visit_ASTBlock(ASTBlock ast) override;
    void visit_ASTSimpleExpr(ASTSimpleExpr ast) override;
    void visit_ASTExpr(ASTExpr ast) override;
    void visit_ASTTerm(ASTTerm ast) override;
    void visit_ASTFactor(ASTFactor ast) override;
    void visit_ASTDesignator(ASTDesignator ast) override;
    void visit_ASTType(ASTType ast) override;
    void visit_ASTArray(ASTArray ast) override;
    void visit_ASTRecord(ASTRecord ast) override;
    void visit_ASTPointerType(ASTPointerType ast) override;
    void visit_ASTQualident(ASTQualident ast) override;
    void visit_ASTIdentifier(ASTIdentifier ast) override;
    void visit_ASTSet(ASTSet ast) override;
    void visit_ASTInteger(ASTInteger ast) override;
    void visit_ASTReal(ASTReal ast) override;
    void visit_ASTChar(ASTCharPtr ast) override;
    void visit_ASTString(ASTString ast) override;
    void visit_ASTBool(ASTBool ast) override;
    void visit_ASTNil(ASTNil /*not used*/) override;

    std::string get_Qualident(ASTQualident const &ast);

    SymbolFrameTable &symboltable;
    TypeTable        &types;
    ErrorManager     &errors;
    Importer         &importer;

    bool         is_const{false};
    bool         is_lvalue{false};
    bool         is_qualid{false};
    bool         qualid_error{false};
    TypePtr      last_type{nullptr};
    ASTProcedure last_proc{nullptr};

    Attr variable_type{Attr::null};

    // Post process
    std::vector<std::shared_ptr<PointerType>> pointer_types;
};

} // namespace ax
