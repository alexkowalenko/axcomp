//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <vector>

#include "ast.hh"
#include "astvisitor.hh"

namespace ax {

class ASTInteger : public ASTBase {
  public:
    ~ASTInteger(){};

    void accept(ASTVisitor *v) { v->visit_ASTInteger(this); };

    long value;
};

class ASTExpr : public ASTBase {
  public:
    ~ASTExpr(){};

    void accept(ASTVisitor *v) { v->visit_ASTExpr(this); };

    std::shared_ptr<ASTInteger> integer;
};

class ASTModule : public ASTBase {
  public:
    ~ASTModule(){};
    void accept(ASTVisitor *v) { v->visit_ASTModule(this); };

    std::vector<std::shared_ptr<ASTExpr>> exprs;
};
} // namespace ax