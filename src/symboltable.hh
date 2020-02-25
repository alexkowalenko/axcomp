//
// AX compiler
//
// Copyright © Alex Kowalenko 2020.
//

#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <string>

#include <llvm/IR/Value.h>

namespace ax {

using namespace llvm;

class SymbolTable {
  public:
    SymbolTable(std::shared_ptr<SymbolTable> s) : next(s){};

    SymbolTable(const SymbolTable &) = delete; // stop copying

    inline void put(const std::string &name, Value *const val) {
        table[name] = val;
    };

    std::optional<Value *> find(const std::string &name) const;
    bool                   set(const std::string &name, Value *const val);
    void                   remove(const std::string &name);

    void dump(std::ostream &os) const;

  private:
    std::map<std::string, Value *> table;
    std::shared_ptr<SymbolTable>   next;
};

} // namespace ax