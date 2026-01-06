//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <memory>
namespace ax {

class ASTModule_;
using ASTModule = std::shared_ptr<ASTModule_>;
class ASTImport_;
using ASTImport = std::shared_ptr<ASTImport_>;
class ASTDeclaration_;
using ASTDeclaration = std::shared_ptr<ASTDeclaration_>;
class ASTTypeDec_;
using ASTTypeDec = std::shared_ptr<ASTTypeDec_>;
class ASTConst_;
using ASTConst = std::shared_ptr<ASTConst_>;
class ASTVar_;
using ASTVar = std::shared_ptr<ASTVar_>;
class ASTProcedure_;
using ASTProcedure = std::shared_ptr<ASTProcedure_>;
class ASTProcedureForward_;
using ASTProcedureForward = std::shared_ptr<ASTProcedureForward_>;
class ASTAssignment_;
using ASTAssignment = std::shared_ptr<ASTAssignment_>;
class ASTReturn_;
using ASTReturn = std::shared_ptr<ASTReturn_>;
class ASTExit_;
using ASTExit = std::shared_ptr<ASTExit_>;
class ASTCall_;
using ASTCall = std::shared_ptr<ASTCall_>;
class ASTIf_;
using ASTIf = std::shared_ptr<ASTIf_>;
class ASTCaseElement_;
using ASTCaseElement = std::shared_ptr<ASTCaseElement_>;
class ASTCase_;
using ASTCase = std::shared_ptr<ASTCase_>;
class ASTFor_;
using ASTFor = std::shared_ptr<ASTFor_>;
class ASTWhile_;
using ASTWhile = std::shared_ptr<ASTWhile_>;
class ASTRepeat_;
using ASTRepeat = std::shared_ptr<ASTRepeat_>;
class ASTLoop_;
using ASTLoop = std::shared_ptr<ASTLoop_>;
class ASTBlock_;
using ASTBlock = std::shared_ptr<ASTBlock_>;
class ASTExpr_;
using ASTExpr = std::shared_ptr<ASTExpr_>;
class ASTSimpleExpr_;
using ASTSimpleExpr = std::shared_ptr<ASTSimpleExpr_>;
class ASTRange_;
using ASTRange = std::shared_ptr<ASTRange_>;
class ASTTerm_;
using ASTTerm = std::shared_ptr<ASTTerm_>;
class ASTFactor_;
using ASTFactor = std::shared_ptr<ASTFactor_>;
class ASTDesignator_;
using ASTDesignator = std::shared_ptr<ASTDesignator_>;
class ASTType_;
using ASTType = std::shared_ptr<ASTType_>;
class ASTArray_;
using ASTArray = std::shared_ptr<ASTArray_>;
class ASTRecord_;
using ASTRecord = std::shared_ptr<ASTRecord_>;
class ASTPointerType_;
using ASTPointerType = std::shared_ptr<ASTPointerType_>;
class ASTQualident_;
using ASTQualident = std::shared_ptr<ASTQualident_>;
class ASTIdentifier_;
using ASTIdentifier = std::shared_ptr<ASTIdentifier_>;
class ASTInteger_;
using ASTInteger = std::shared_ptr<ASTInteger_>;
class ASTReal_;
using ASTReal = std::shared_ptr<ASTReal_>;
class ASTChar_;
using ASTCharPtr = std::shared_ptr<ASTChar_>;
class ASTString_;
using ASTString = std::shared_ptr<ASTString_>;
class ASTSet_;
using ASTSet = std::shared_ptr<ASTSet_>;
class ASTBool_;
using ASTBool = std::shared_ptr<ASTBool_>;
class ASTNil_;
using ASTNil = std::shared_ptr<ASTNil_>;

class ASTVisitor {

  public:
    virtual ~ASTVisitor() = default;

    virtual void visit_ASTModule(ASTModule ast);
    virtual void visit_ASTImport(ASTImport ast);
    virtual void visit_ASTDeclaration(ASTDeclaration ast);
    virtual void visit_ASTConst(ASTConst ast);
    virtual void visit_ASTTypeDec(ASTTypeDec ast);
    virtual void visit_ASTVar(ASTVar ast);
    virtual void visit_ASTProcedure(ASTProcedure ast);
    virtual void visit_ASTProcedureForward(ASTProcedureForward ast);

    virtual void visit_ASTAssignment(ASTAssignment ast);
    virtual void visit_ASTReturn(ASTReturn ast);
    virtual void visit_ASTExit(ASTExit ast);
    virtual void visit_ASTCall(ASTCall ast);
    virtual void visit_ASTIf(ASTIf ast);
    virtual void visit_ASTCaseElement(ASTCaseElement ast);
    virtual void visit_ASTCase(ASTCase ast);
    virtual void visit_ASTFor(ASTFor ast);
    virtual void visit_ASTWhile(ASTWhile ast);
    virtual void visit_ASTRepeat(ASTRepeat ast);
    virtual void visit_ASTLoop(ASTLoop ast);
    virtual void visit_ASTBlock(ASTBlock ast);

    virtual void visit_ASTExpr(ASTExpr ast);
    virtual void visit_ASTRange(ASTRange ast);
    virtual void visit_ASTSimpleExpr(ASTSimpleExpr ast);
    virtual void visit_ASTTerm(ASTTerm ast);
    virtual void visit_ASTFactor(ASTFactor ast);
    virtual void visit_ASTDesignator(ASTDesignator ast);

    virtual void visit_ASTType(ASTType ast);
    virtual void visit_ASTArray(ASTArray ast);
    virtual void visit_ASTPointerType(ASTPointerType ast);
    virtual void visit_ASTRecord(ASTRecord ast);
    virtual void visit_ASTQualident(ASTQualident /*not used*/){};   // NOLINT
    virtual void visit_ASTIdentifier(ASTIdentifier /*not used*/){}; // NOLINT
    virtual void visit_ASTSet(ASTSet /*not used*/){};               // NOLINT
    virtual void visit_ASTInteger(ASTInteger /*not used*/){};       // NOLINT
    virtual void visit_ASTReal(ASTReal /*not used*/){};             // NOLINT
    virtual void visit_ASTChar(ASTCharPtr /*not used*/){};          // NOLINT
    virtual void visit_ASTString(ASTString /*not used*/){};         // NOLINT
    virtual void visit_ASTBool(ASTBool /*not used*/){};             // NOLINT
    virtual void visit_ASTNil(ASTNil /*not used*/){};               // NOLINT
};

} // namespace ax