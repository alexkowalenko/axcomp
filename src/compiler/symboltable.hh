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
#include <set>
#include <stack>
#include <string>

#include <llvm/Support/Debug.h>

namespace ax {

template <typename T> class TableInterface {
  public:
    virtual ~TableInterface() = default;

    [[nodiscard]] virtual T find(const std::string &name) const = 0;

    /**
     * @brief puts the value into the table at the top current map. See set for changeing an
     * existing value.
     *
     * @param name
     * @param val
     */
    virtual void put(const std::string &name, T const &val) = 0;

    /**
     * @brief sets a value in the symbol table, wherever it could be in the symbol table.
     *
     * @param name
     * @param val
     * @return true - name found, and value changed changed.
     * @return false
     */
    virtual bool set(const std::string &name, T const &val) = 0;
    virtual void remove(const std::string &name) = 0;

    [[nodiscard]] virtual typename std::map<std::string, T>::const_iterator begin() const = 0;
    [[nodiscard]] virtual typename std::map<std::string, T>::const_iterator end() const = 0;

    virtual void                                 reset_free_variables() = 0;
    [[nodiscard]] virtual std::set<std::string> &get_free_variables() const = 0;
};

template <typename T> class SymbolTable : public TableInterface<T> {
  public:
    explicit SymbolTable(std::shared_ptr<SymbolTable> s) : next(std::move(s)) {};
    ~SymbolTable() override = default;

    SymbolTable(const SymbolTable &) = delete; // stop copying

    [[nodiscard]] T find(const std::string &name) const override;

    void put(const std::string &name, T const &val) override { table[name] = val; };
    bool set(const std::string &name, T const &val) override;
    void remove(const std::string &name) override;

    [[nodiscard]] typename std::map<std::string, T>::const_iterator begin() const override {
        return table.begin();
    }
    [[nodiscard]] typename std::map<std::string, T>::const_iterator end() const override {
        return table.end();
    }

    // Free variables

    void reset_free_variables() override {
        free_variables.clear();
        if (next) {
            next->reset_free_variables();
        }
    }

    [[nodiscard]] std::set<std::string> &get_free_variables() const override {
        return free_variables;
    }

    void dump(std::ostream &os) const;

  private:
    std::map<std::string, T>     table{};
    std::shared_ptr<SymbolTable> next = nullptr;

    mutable std::set<std::string> free_variables{};
};

template <typename T> T SymbolTable<T>::find(const std::string &name) const {
    if (auto const &x = table.find(name); x != table.end()) {
        return x->second;
    }
    if (next) {
        auto res = next->find(name);
        if (res) {
            // add free variable
            free_variables.insert(name);
        }
        return res;
    }
    return nullptr;
}

template <typename T> bool SymbolTable<T>::set(const std::string &name, T const &val) {
    if (table.contains(name)) {
        put(name, val);
        return true;
    }
    // not found, check above
    if (next) {
        auto res = next->set(name, val);
        if (res) {
            // add free variable
            free_variables.insert(name);
        }
        return res;
    }
    return false;
}

template <typename T> void SymbolTable<T>::remove(const std::string &name) {
    if (table.contains(name)) {
        table.erase(name);
        return;
    }
    if (next) {
        next->remove(name);
    }
}

template <typename T> void SymbolTable<T>::dump(std::ostream &os) const {
    os << "Dump symbol table: \n";
    for (auto const &[name, val] : table) {
        os << name << " -> " << val.first->get_name() << std::endl;
    }
    if (next) {
        next->dump(os);
    }
}

template <typename T> class FrameTable : public TableInterface<T> {
  public:
    FrameTable() { push_frame("."); };

    [[nodiscard]] T find(const std::string &name) const override {
        return current_table->find(name);
    };

    void put(const std::string &name, T const &val) override { current_table->put(name, val); };
    bool set(const std::string &name, T const &val) override {
        return current_table->set(name, val);
    };

    void remove(const std::string &name) override { current_table->remove(name); };

    [[nodiscard]] typename std::map<std::string, T>::const_iterator begin() const override {
        return current_table->begin();
    };
    [[nodiscard]] typename std::map<std::string, T>::const_iterator end() const override {
        return current_table->end();
    };

    // Free variables

    void reset_free_variables() override { current_table->reset_free_variables(); };
    [[nodiscard]] std::set<std::string> &get_free_variables() const override {
        return current_table->get_free_variables();
    };

    // Frames

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
    if (frame_map.contains(name)) {
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
    for (auto &frame : frame_map) {
        os << frame.first << "  ------------------------------\n";
        for (auto &symbol : *frame.second) {
            os << std::string(symbol.first) << " : " << symbol.second->type->get_name() << '\n';
        }
    }
}

} // namespace ax

#include "symbol.hh"
namespace ax {

class SymbolFrameTable : public FrameTable<SymbolPtr> {

  public:
    /**
     * @brief Set the value object - wherever it is in the symbol table.
     *
     * @param name
     * @param v
     */
    void set_value(std::string const &name, llvm::Value *v) const {
        const auto s = find(name);
        s->value = v;
    };

    /**
     * @brief Set the value object - wherever it is in the symbol table.
     *
     * @param name
     * @param v
     * @param a
     */
    void set_value(std::string const &name, llvm::Value *v, const Attr a) const {
        const auto s = find(name);
        s->value = v;
        s->set(a);
    };
};

} // namespace ax
