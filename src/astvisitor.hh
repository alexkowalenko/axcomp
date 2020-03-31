//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

namespace ax {

class ASTModule;
class ASTDeclaration;
class ASTTypeDec;
class ASTConst;
class ASTVar;
class ASTProcedure;
class ASTStatement;
class ASTAssignment;
class ASTReturn;
class ASTExit;
class ASTCall;
class ASTIf;
class ASTFor;
class ASTWhile;
class ASTRepeat;
class ASTLoop;
class ASTBlock;
class ASTExpr;
class ASTSimpleExpr;
class ASTTerm;
class ASTFactor;
class ASTDesignator;
class ASTInteger;
class ASTBool;
class ASTType;
class ASTArray;
class ASTRecord;
class ASTIdentifier;

class ASTVisitor {

  public:
    virtual ~ASTVisitor() = default;

    virtual void visit_ASTModule(ASTModule *ast);
    virtual void visit_ASTDeclaration(ASTDeclaration *ast);
    virtual void visit_ASTConst(ASTConst *ast);
    virtual void visit_ASTTypeDec(ASTTypeDec *);
    virtual void visit_ASTVar(ASTVar *ast);
    virtual void visit_ASTProcedure(ASTProcedure *ast);
    virtual void visit_ASTAssignment(ASTAssignment *ast);
    virtual void visit_ASTReturn(ASTReturn *ast);
    virtual void visit_ASTExit(ASTExit *ast);
    virtual void visit_ASTCall(ASTCall *ast);
    virtual void visit_ASTIf(ASTIf *ast);
    virtual void visit_ASTFor(ASTFor *ast);
    virtual void visit_ASTWhile(ASTWhile *ast);
    virtual void visit_ASTRepeat(ASTRepeat *ast);
    virtual void visit_ASTLoop(ASTLoop *ast);
    virtual void visit_ASTBlock(ASTBlock *ast);
    virtual void visit_ASTExpr(ASTExpr *ast);
    virtual void visit_ASTSimpleExpr(ASTSimpleExpr *ast);
    virtual void visit_ASTTerm(ASTTerm *ast);
    virtual void visit_ASTFactor(ASTFactor *ast);
    virtual void visit_ASTDesignator(ASTDesignator *ast);
    virtual void visit_ASTInteger(ASTInteger *ast);
    virtual void visit_ASTType(ASTType *ast);
    virtual void visit_ASTArray(ASTArray *ast);
    virtual void visit_ASTRecord(ASTRecord *ast);
    virtual void visit_ASTIdentifier(ASTIdentifier *ast);
    virtual void visit_ASTBool(ASTBool *ast);
};

} // namespace ax