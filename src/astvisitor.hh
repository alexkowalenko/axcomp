//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include "ast.hh"

namespace ax {

class ASTModule;
class ASTDeclaration;
class ASTConst;
class ASTVar;
class ASTProcedure;
class ASTStatement;
class ASTAssignment;
class ASTReturn;
class ASTCall;
class ASTExpr;
class ASTSimpleExpr;
class ASTTerm;
class ASTFactor;
class ASTInteger;
class ASTBool;
class ASTIdentifier;

class ASTVisitor {

  public:
    virtual ~ASTVisitor() = default;

    virtual void visit_ASTModule(ASTModule *ast);
    virtual void visit_ASTDeclaration(ASTDeclaration *ast);
    virtual void visit_ASTConst(ASTConst *ast);
    virtual void visit_ASTVar(ASTVar *ast);
    virtual void visit_ASTProcedure(ASTProcedure *ast);
    virtual void visit_ASTAssignment(ASTAssignment *ast);
    virtual void visit_ASTReturn(ASTReturn *ast);
    virtual void visit_ASTCall(ASTCall *ast);
    virtual void visit_ASTExpr(ASTExpr *ast);
    virtual void visit_ASTSimpleExpr(ASTSimpleExpr *ast);
    virtual void visit_ASTTerm(ASTTerm *ast);
    virtual void visit_ASTFactor(ASTFactor *ast);
    virtual void visit_ASTInteger(ASTInteger *ast);
    virtual void visit_ASTIdentifier(ASTIdentifier *ast);
    virtual void visit_ASTBool(ASTBool *ast);
};

} // namespace ax