//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <memory>
#include <optional>

#include "../astvisitor.hh"
#include "../location.hh"
#include "../token.hh"
#include "../types/type.hh"

namespace ax {

class ASTBase_ {
  public:
    ASTBase_() = default;
    virtual ~ASTBase_() = default;

    ASTBase_(ASTBase_ const &) = default;
    ASTBase_ &operator=(ASTBase_ const &) = default;

    ASTBase_(ASTBase_ &&) = default;
    ASTBase_ &operator=(ASTBase_ &&) = default;

    virtual void accept(ASTVisitor *v) = 0;

    void                          set_location(Location const &l) { location = l; };
    [[nodiscard]] Location const &get_location() const { return location; };

    [[nodiscard]] Type get_type() const { return type_info; };
    void               set_type(Type const &t) { type_info = t; }

    explicit operator std::string();

  private:
    Location location;
    Type     type_info{nullptr}; // store information about the type
};
using ASTBase = std::shared_ptr<ASTBase_>;

template <class T, typename... Rest> auto make(Rest... rest) {
    return std::make_shared<T>(rest...);
}

} // namespace ax
