//
// AX compiler
//
// Copyright © Alex Kowalenko 2020.
//

#include "symboltable.hh"

namespace ax {

std::optional<Value *> SymbolTable::find(const std::string &name) const {
    if (auto x = table.find(name); x != table.end()) {
        return x->second;
    }
    if (next) {
        return next->find(name);
    }
    return {};
}

bool SymbolTable::set(const std::string &name, Value *const val) {
    if (auto x = table.find(name); x != table.end()) {
        put(name, val);
        return true;
    } else {
        // not found, check above
        if (next) {
            return next->set(name, val);
        }
        return false;
    }
};

void SymbolTable::remove(const std::string &name) {
    if (auto x = table.find(name); x != table.end()) {
        table.erase(name);
        return;
    } else if (next) {
        next->remove(name);
    }
}

void SymbolTable::dump(std::ostream &os) const {
    os << "Dump symbol table: \n";
    for (auto x : table) {
        // os << x.first << " -> " << std::to_string(x.second) << std::endl;
    }
    if (next) {
        next->dump(os);
    }
}

} // namespace ax
