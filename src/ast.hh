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
    ASTBase() = default;
    virtual ~ASTBase() = default;

    ASTBase(ASTBase const &) = default;
    ASTBase &operator=(ASTBase const &) = default;

    ASTBase(ASTBase &&) = default;
    ASTBase &operator=(ASTBase &&) = default;

    virtual void accept(ASTVisitor *v) = 0;
};

} // namespace ax
