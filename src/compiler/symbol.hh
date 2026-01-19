//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <memory>
#include <set>
#include <utility>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wunused-parameter"
#include <llvm/IR/Value.h>
#pragma clang diagnostic pop

#include "type.hh"

namespace ax {

class Symbol {
  public:
    Symbol() = default;

    Symbol(Symbol const &) = default;
    Symbol &operator=(Symbol const &) = default;

    Symbol(Symbol &&) = default;
    Symbol &operator=(Symbol &&) = default;

    explicit Symbol(Type t) : type{std::move(t)} {};
    Symbol(Type t, Attr const &a) : type{std::move(t)} { attrs.insert(a); };
    Symbol(Type t, Attr const &a, llvm::Value *v) : type{std::move(t)}, value(v) {
        attrs.insert(a);
    };

    void               set(const Attr &a) { attrs.insert(a); };
    [[nodiscard]] bool is(const Attr &a) const { return attrs.contains(a); }

    Type           type = nullptr;
    std::set<Attr> attrs;
    llvm::Value   *value = nullptr;
};

using SymbolPtr = std::shared_ptr<Symbol>;

template <typename... Ts> inline SymbolPtr mkSym(Ts... t) {
    return std::make_shared<Symbol>(t...);
};

} // namespace ax
