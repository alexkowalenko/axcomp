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

    virtual void visit(ASTModule const &ast);
    virtual void visit(ASTImport const &ast);
    virtual void visit(ASTDeclaration const &ast);
    virtual void visit(ASTConst const &ast);
    virtual void visit(ASTTypeDec const &ast);
    virtual void visit(ASTVar const &ast);
    virtual void visit(ASTProcedure const &ast);
    virtual void visit(ASTProcedureForward const &ast);

    virtual void visit(ASTAssignment const &ast);
    virtual void visit(ASTReturn const &ast);
    virtual void visit(ASTExit const &ast);
    virtual void visit(ASTCall const &ast);
    virtual void visit(ASTIf const &ast);
    virtual void visit(ASTCaseElement const &ast);
    virtual void visit(ASTCase const &ast);
    virtual void visit(ASTFor const &ast);
    virtual void visit(ASTWhile const &ast);
    virtual void visit(ASTRepeat const &ast);
    virtual void visit(ASTLoop const &ast);
    virtual void visit(ASTBlock const &ast);

    virtual void visit(ASTExpr const &ast);
    virtual void visit(ASTRange const &ast);
    virtual void visit(ASTSimpleExpr const &ast);
    virtual void visit(ASTTerm const &ast);
    virtual void visit(ASTFactor const &ast);
    virtual void visit(ASTDesignator const &ast);

    virtual void visit(ASTType const &ast);
    virtual void visit(ASTArray const &ast);
    virtual void visit(ASTPointerType const &ast);
    virtual void visit(ASTRecord const &ast);
    virtual void visit(ASTQualident const & /*not used*/){};   // NOLINT
    virtual void visit(ASTIdentifier const & /*not used*/){}; // NOLINT
    virtual void visit(ASTSet const & /*not used*/){};               // NOLINT
    virtual void visit(ASTInteger const & /*not used*/){};       // NOLINT
    virtual void visit(ASTReal const & /*not used*/){};             // NOLINT
    virtual void visit(ASTCharPtr const & /*not used*/){};          // NOLINT
    virtual void visit(ASTString const & /*not used*/){};         // NOLINT
    virtual void visit(ASTBool const & /*not used*/){};             // NOLINT
    virtual void visit(ASTNil const & /*not used*/){};               // NOLINT
};

} // namespace ax
