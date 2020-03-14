//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include "location.hh"

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

    void            set_location(Location const &l) { location = l; };
    Location const &get_location() { return location; };

  private:
    Location location;
};

} // namespace ax
