//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <string>
#include <unordered_set>

namespace ax {

enum class Attr {
    null = 0,
    var,       // VAR parameters
    global,    // * symbol on global objects in modules
    read_only, // - symbol on read_only objects
    cnst,      // CONST variables
    used,      // function is called in the module

    global_function,  // global function defined in runtime
    compile_function, // compiler function
};

constexpr auto attr_star{"*"};
constexpr auto attr_dash{"-"};

class Attrs : std::unordered_set<Attr> {
  public:
    Attrs() = default;
    ~Attrs() = default;

    void               set(Attr const &t) { insert(t); };
    [[nodiscard]] bool contains(Attr const &t) const { return (this->find(t) != end()); }

    explicit operator std::string() const {
        if (contains(Attr::global)) {
            return attr_star;
        }
        if (contains(Attr::read_only)) {
            return attr_dash;
        }
        return std::string{};
    };
};

} // namespace ax
