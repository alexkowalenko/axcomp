//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

namespace ax {

class ASTVisitor;

class ASTBase {
  public:
    virtual ~ASTBase(){};

    virtual void accept(ASTVisitor *v) = 0;
};

} // namespace ax
