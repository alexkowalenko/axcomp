//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include "ast.hh"

namespace ax {

class ASTModule;
class ASTExpr;
class ASTTerm;
class ASTFactor;
class ASTInteger;

class ASTVisitor {

  public:
    virtual void visit_ASTModule(ASTModule *) = 0;
    virtual void visit_ASTExpr(ASTExpr *) = 0;
    virtual void visit_ASTTerm(ASTTerm *) = 0;
    virtual void visit_ASTFactor(ASTFactor *) = 0;
    virtual void visit_ASTInteger(ASTInteger *) = 0;
};

} // namespace ax