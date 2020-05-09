//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <memory>
#include <unordered_set>
#include <utility>

#include <llvm/IR/Value.h>

#include "type.hh"

namespace ax {

class Symbol {
  public:
    Symbol() = default;

    Symbol(Symbol const &) = default;
    Symbol &operator=(Symbol const &) = default;

    Symbol(Symbol &&) = default;
    Symbol &operator=(Symbol &&) = default;

    explicit Symbol(TypePtr t) : type{std::move(t)} {};
    Symbol(TypePtr t, Attr const &a) : type{std::move(t)} { attrs.insert(a); };
    Symbol(TypePtr t, Attr const &a, llvm::Value *v) : type{std::move(t)}, value(v) {
        attrs.insert(a);
    };

    void set(const Attr &a) { attrs.insert(a); };
    bool is(const Attr &a) { return attrs.find(a) != attrs.end(); }

    TypePtr                  type = nullptr;
    std::unordered_set<Attr> attrs;
    llvm::Value *            value = nullptr;
};

using SymbolPtr = std::shared_ptr<Symbol>;

template <typename... Ts> inline SymbolPtr mkSym(Ts... t) {
    return std::make_shared<Symbol>(t...);
};

} // namespace ax
