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

    void check(ASTModulePtr const &ast);

  private:
    void visit_ASTModule(ASTModulePtr ast) override;
    void visit_ASTImport(ASTImportPtr ast) override;
    void visit_ASTConst(ASTConstPtr ast) override;
    void visit_ASTTypeDec(ASTTypeDecPtr ast) override;
    void visit_ASTVar(ASTVarPtr ast) override;

    void                                          do_receiver(RecVar &r);
    std::pair<TypePtr, ProcedureType::ParamsList> do_proc(ASTProc &ast);

    void visit_ASTProcedure(ASTProcedurePtr ast) override;
    void visit_ASTProcedureForward(ASTProcedureForwardPtr ast) override;

    void visit_ASTAssignment(ASTAssignmentPtr ast) override;
    void visit_ASTReturn(ASTReturnPtr ast) override;
    void visit_ASTCall(ASTCallPtr ast) override;
    void visit_ASTIf(ASTIfPtr ast) override;
    void visit_ASTCaseElement(ASTCaseElementPtr ast) override;
    void visit_ASTCase(ASTCasePtr ast) override;
    void visit_ASTFor(ASTForPtr ast) override;
    void visit_ASTWhile(ASTWhilePtr ast) override;
    void visit_ASTRepeat(ASTRepeatPtr ast) override;
    void visit_ASTLoop(ASTLoopPtr ast) override;
    void visit_ASTBlock(ASTBlockPtr ast) override;
    void visit_ASTSimpleExpr(ASTSimpleExprPtr ast) override;
    void visit_ASTExpr(ASTExprPtr ast) override;
    void visit_ASTTerm(ASTTermPtr ast) override;
    void visit_ASTFactor(ASTFactorPtr ast) override;
    void visit_ASTDesignator(ASTDesignatorPtr ast) override;
    void visit_ASTType(ASTTypePtr ast) override;
    void visit_ASTArray(ASTArrayPtr ast) override;
    void visit_ASTRecord(ASTRecordPtr ast) override;
    void visit_ASTPointerType(ASTPointerTypePtr ast) override;
    void visit_ASTQualident(ASTQualidentPtr ast) override;
    void visit_ASTIdentifier(ASTIdentifierPtr ast) override;
    void visit_ASTSet(ASTSetPtr ast) override;
    void visit_ASTInteger(ASTIntegerPtr ast) override;
    void visit_ASTReal(ASTRealPtr ast) override;
    void visit_ASTChar(ASTCharPtr ast) override;
    void visit_ASTString(ASTStringPtr ast) override;
    void visit_ASTBool(ASTBoolPtr ast) override;
    void visit_ASTNil(ASTNilPtr /*not used*/) override;

    std::string get_Qualident(ASTQualidentPtr const &ast);

    SymbolFrameTable &symboltable;
    TypeTable &       types;
    ErrorManager &    errors;
    Importer &        importer;

    bool            is_const{false};
    bool            is_lvalue{false};
    bool            is_qualid{false};
    bool            qualid_error{false};
    TypePtr         last_type{nullptr};
    ASTProcedurePtr last_proc{nullptr};

    Attr variable_type{Attr::null};

    // Post process
    std::vector<std::shared_ptr<PointerType>> pointer_types;
};

} // namespace ax
