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

namespace ax {

template <typename T> class SymbolTable {
  public:
    explicit SymbolTable(std::shared_ptr<SymbolTable> s) : next(std::move(s)){};
    ~SymbolTable() = default;

    SymbolTable(const SymbolTable &) = delete; // stop copying

    inline void put(const std::string &name, T const &val) {
        table[name] = val;
    };

    std::optional<T> find(const std::string &name) const;
    bool             set(const std::string &name, T const &val);
    void             remove(const std::string &name);

    void dump(std::ostream &os) const;

  private:
    std::map<std::string, T>     table;
    std::shared_ptr<SymbolTable> next;
};

template <typename T>
std::optional<T> SymbolTable<T>::find(const std::string &name) const {
    if (auto const &x = table.find(name); x != table.end()) {
        return x->second;
    }
    if (next) {
        return next->find(name);
    }
    return {};
}

template <typename T>
bool SymbolTable<T>::set(const std::string &name, T const &val) {
    if (auto const &x = table.find(name); x != table.end()) {
        put(name, val);
        return true;
    }
    // not found, check above
    if (next) {
        return next->set(name, val);
    }
    return false;
}

template <typename T> void SymbolTable<T>::remove(const std::string &name) {
    if (auto const &x = table.find(name); x != table.end()) {
        table.erase(name);
        return;
    }
    if (next) {
        next->remove(name);
    }
}

template <typename T> void SymbolTable<T>::dump(std::ostream &os) const {
    os << "Dump symbol table: \n";
    for (auto const &x : table) {
        os << x.first << " -> " << std::string(x.second) << std::endl;
    }
    if (next) {
        next->dump(os);
    }
}

} // namespace ax