//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <vector>

namespace ax {

class ASTBase {};

class ASTInteger : public ASTBase {
  public:
    long value;
};

class ASTExpr : public ASTBase {
  public:
    ASTInteger integer;
};

class ASTProg : public ASTBase {
  public:
    std::vector<ASTExpr> exprs;
};

} // namespace ax
