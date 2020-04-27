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
#include <ostream>
#include <stack>
#include <string>

#include <llvm/ADT/StringMap.h>

namespace ax {

template <typename T> class TableInterface {
  public:
    virtual ~TableInterface() = default;

    virtual void            put(const std::string &name, T const &val) = 0;
    [[nodiscard]] virtual T find(const std::string &name) const = 0;
    virtual bool            set(const std::string &name, T const &val) = 0;
    virtual void            remove(const std::string &name) = 0;

    [[nodiscard]] virtual typename llvm::StringMap<T>::const_iterator begin() const = 0;
    [[nodiscard]] virtual typename llvm::StringMap<T>::const_iterator end() const = 0;
};

template <typename T> class SymbolTable : public TableInterface<T> {
  public:
    explicit SymbolTable(std::shared_ptr<SymbolTable> s) : next(std::move(s)){};
    ~SymbolTable() override = default;

    SymbolTable(const SymbolTable &) = delete; // stop copying

    void put(const std::string &name, T const &val) override { table[name] = val; };

    [[nodiscard]] T find(const std::string &name) const override;
    bool            set(const std::string &name, T const &val) override;
    void            remove(const std::string &name) override;

    [[nodiscard]] typename llvm::StringMap<T>::const_iterator begin() const override {
        return table.begin();
    }
    [[nodiscard]] typename llvm::StringMap<T>::const_iterator end() const override {
        return table.end();
    }

    void dump(std::ostream &os) const;

  private:
    llvm::StringMap<T>           table;
    std::shared_ptr<SymbolTable> next = nullptr;
};

template <typename T> T SymbolTable<T>::find(const std::string &name) const {
    if (auto const &x = table.find(name); x != table.end()) {
        return x->second;
    }
    if (next) {
        return next->find(name);
    }
    return nullptr;
}

template <typename T> bool SymbolTable<T>::set(const std::string &name, T const &val) {
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
        os << x.first << " -> " << x.second.first->get_name() << std::endl;
    }
    if (next) {
        next->dump(os);
    }
}

template <typename T> class FrameTable : public TableInterface<T> {
  public:
    FrameTable() { push_frame("."); };

    void put(const std::string &name, T const &val) override { current_table->put(name, val); };
    [[nodiscard]] T find(const std::string &name) const override {
        return current_table->find(name);
    };
    bool set(const std::string &name, T const &val) override {
        return current_table->set(name, val);
    };
    void remove(const std::string &name) override { current_table->remove(name); };

    [[nodiscard]] typename llvm::StringMap<T>::const_iterator begin() const override {
        return current_table->begin();
    };
    [[nodiscard]] typename llvm::StringMap<T>::const_iterator end() const override {
        return current_table->end();
    };

    void push_frame(std::string const &frame_name);
    void pop_frame();

    void dump(std::ostream &os);

  private:
    using SymbolTablePtr = std::shared_ptr<SymbolTable<T>>;
    std::map<std::string, SymbolTablePtr> frame_map;
    std::stack<SymbolTablePtr>            frame_stack;

    SymbolTablePtr current_table = nullptr;
};

template <typename T> void FrameTable<T>::push_frame(std::string const &name) {
    if (frame_map.find(name) != frame_map.end()) {
        current_table = frame_map[name];
        frame_stack.push(current_table);
        return;
    }
    // Create new frame
    auto new_table = std::make_shared<SymbolTable<T>>(current_table);
    frame_map[name] = new_table;
    frame_stack.push(new_table);
    current_table = new_table;
}

template <typename T> void FrameTable<T>::pop_frame() {
    frame_stack.pop();
    current_table = frame_stack.top();
}

template <typename T> void FrameTable<T>::dump(std::ostream &os) {
    std::for_each(std::begin(frame_map), std::end(frame_map), [&os](auto &f) {
        os << f.first << "  ------------------------------\n";
        std::for_each(f.second->begin(), f.second->end(), [&os](auto &s) {
            os << std::string(s.first()) << " : " << s.second->type->get_name() << '\n';
        });
    });
}

} // namespace ax

#include "symbol.hh"
#include "llvm/Support/FormatVariadic.h"
namespace ax {

class SymbolFrameTable : public FrameTable<SymbolPtr> {

  public:
    void set_value(std::string const &name, llvm::Value *v) {
        auto s = find(name);
        s->value = v;
    };

    void set_value(std::string const &name, llvm::Value *v, Attr a) {
        auto s = find(name);
        s->value = v;
        s->set(a);
    };
};

} // namespace ax