//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <memory>
namespace ax {

class ASTModule;
using ASTModulePtr = std::shared_ptr<ASTModule>;
class ASTImport;
using ASTImportPtr = std::shared_ptr<ASTImport>;
class ASTDeclaration;
using ASTDeclarationPtr = std::shared_ptr<ASTDeclaration>;
class ASTTypeDec;
using ASTTypeDecPtr = std::shared_ptr<ASTTypeDec>;
class ASTConst;
using ASTConstPtr = std::shared_ptr<ASTConst>;
class ASTVar;
using ASTVarPtr = std::shared_ptr<ASTVar>;
class ASTProcedure;
using ASTProcedurePtr = std::shared_ptr<ASTProcedure>;
class ASTAssignment;
using ASTAssignmentPtr = std::shared_ptr<ASTAssignment>;
class ASTReturn;
using ASTReturnPtr = std::shared_ptr<ASTReturn>;
class ASTExit;
using ASTExitPtr = std::shared_ptr<ASTExit>;
class ASTCall;
using ASTCallPtr = std::shared_ptr<ASTCall>;
class ASTIf;
using ASTIfPtr = std::shared_ptr<ASTIf>;
class ASTFor;
using ASTForPtr = std::shared_ptr<ASTFor>;
class ASTWhile;
using ASTWhilePtr = std::shared_ptr<ASTWhile>;
class ASTRepeat;
using ASTRepeatPtr = std::shared_ptr<ASTRepeat>;
class ASTLoop;
using ASTLoopPtr = std::shared_ptr<ASTLoop>;
class ASTBlock;
using ASTBlockPtr = std::shared_ptr<ASTBlock>;
class ASTExpr;
using ASTExprPtr = std::shared_ptr<ASTExpr>;
class ASTSimpleExpr;
using ASTSimpleExprPtr = std::shared_ptr<ASTSimpleExpr>;
class ASTTerm;
using ASTTermPtr = std::shared_ptr<ASTTerm>;
class ASTFactor;
using ASTFactorPtr = std::shared_ptr<ASTFactor>;
class ASTDesignator;
using ASTDesignatorPtr = std::shared_ptr<ASTDesignator>;
class ASTType;
using ASTTypePtr = std::shared_ptr<ASTType>;
class ASTArray;
using ASTArrayPtr = std::shared_ptr<ASTArray>;
class ASTRecord;
using ASTRecordPtr = std::shared_ptr<ASTRecord>;
class ASTQualident;
using ASTQualidentPtr = std::shared_ptr<ASTQualident>;
class ASTIdentifier;
using ASTIdentifierPtr = std::shared_ptr<ASTIdentifier>;
class ASTInteger;
using ASTIntegerPtr = std::shared_ptr<ASTInteger>;
class ASTBool;
using ASTBoolPtr = std::shared_ptr<ASTBool>;

class ASTVisitor {

  public:
    virtual ~ASTVisitor() = default;

    virtual void visit_ASTModule(ASTModulePtr ast);
    virtual void visit_ASTImport(ASTImportPtr ast);
    virtual void visit_ASTDeclaration(ASTDeclarationPtr ast);
    virtual void visit_ASTConst(ASTConstPtr ast);
    virtual void visit_ASTTypeDec(ASTTypeDecPtr ast);
    virtual void visit_ASTVar(ASTVarPtr ast);
    virtual void visit_ASTProcedure(ASTProcedurePtr ast);

    virtual void visit_ASTAssignment(ASTAssignmentPtr ast);
    virtual void visit_ASTReturn(ASTReturnPtr ast);
    virtual void visit_ASTExit(ASTExitPtr ast);
    virtual void visit_ASTCall(ASTCallPtr ast);
    virtual void visit_ASTIf(ASTIfPtr ast);
    virtual void visit_ASTFor(ASTForPtr ast);
    virtual void visit_ASTWhile(ASTWhilePtr ast);
    virtual void visit_ASTRepeat(ASTRepeatPtr ast);
    virtual void visit_ASTLoop(ASTLoopPtr ast);
    virtual void visit_ASTBlock(ASTBlockPtr ast);

    virtual void visit_ASTExpr(ASTExprPtr ast);
    virtual void visit_ASTSimpleExpr(ASTSimpleExprPtr ast);
    virtual void visit_ASTTerm(ASTTermPtr ast);
    virtual void visit_ASTFactor(ASTFactorPtr ast);
    virtual void visit_ASTDesignator(ASTDesignatorPtr ast);

    virtual void visit_ASTType(ASTTypePtr ast);
    virtual void visit_ASTArray(ASTArrayPtr ast);
    virtual void visit_ASTRecord(ASTRecordPtr ast);
    virtual void visit_ASTQualident(ASTQualidentPtr /*not used*/);
    virtual void visit_ASTIdentifier(ASTIdentifierPtr /*not used*/);
    virtual void visit_ASTInteger(ASTIntegerPtr /*not used*/);
    virtual void visit_ASTBool(ASTBoolPtr /*not used*/);
};

} // namespace ax