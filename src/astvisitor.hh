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
class ASTInteger;

class ASTVisitor {

  public:
    virtual void visit_ASTModule(ASTModule *ast) = 0;
    virtual void visit_ASTExpr(ASTExpr *ast) = 0;
    virtual void visit_ASTTerm(ASTTerm *ast) = 0;
    virtual void visit_ASTInteger(ASTInteger *ast) = 0;
};

} // namespace ax