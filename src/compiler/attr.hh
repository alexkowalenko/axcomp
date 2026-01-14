//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <set>
#include <string>

namespace ax {

enum class Attr : std::uint8_t {
    null = 0,
    var,       // VAR parameters
    global,    // * symbol on global objects in modules
    read_only, // - symbol on read_only objects
    cnst,      // CONST variables
    ptr,       // ptr types and STRINGs
    used,      // function is called in the module

    global_function,  // global function defined in runtime
    compile_function, // compiler function
    closure,          // function needs has closure variables

    global_var, // global top level variable
    free_var,   // non local variable
    local_var,  // local variable
    modified,   // variable is modified - assignment, var argument
};

constexpr auto attr_star{"*"};
constexpr auto attr_dash{"-"};

class Attrs : public std::set<Attr> {
  public:
    Attrs() = default;
    ~Attrs() = default;

    void set(Attr const &t) { insert(t); };

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
