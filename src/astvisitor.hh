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
class ASTExpr;
class ASTTerm;
class ASTFactor;
class ASTInteger;
class ASTIdentifier;

class ASTVisitor {

  public:
    virtual ~ASTVisitor(){};
    virtual void visit_ASTModule(ASTModule *) = 0;
    virtual void visit_ASTDeclaration(ASTDeclaration *) = 0;
    virtual void visit_ASTConst(ASTConst *) = 0;
    virtual void visit_ASTVar(ASTVar *) = 0;
    virtual void visit_ASTProcedure(ASTProcedure *) = 0;
    virtual void visit_ASTAssignment(ASTAssignment *) = 0;
    virtual void visit_ASTReturn(ASTReturn *) = 0;
    virtual void visit_ASTExpr(ASTExpr *) = 0;
    virtual void visit_ASTTerm(ASTTerm *) = 0;
    virtual void visit_ASTFactor(ASTFactor *) = 0;
    virtual void visit_ASTInteger(ASTInteger *) = 0;
    virtual void visit_ASTIdentifier(ASTIdentifier *) = 0;
};

} // namespace ax