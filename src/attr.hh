//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <unordered_set>

namespace ax {

enum class Attr {
    null = 0,
    var,       // VAR parameters
    global,    // * symbol on global objects in modules
    read_only, // - symbol on read_only objects
};

class Attrs : std::unordered_set<Attr> {
  public:
    Attrs() = default;
    ~Attrs() = default;

    void set(Attr const &t) { insert(t); };
    bool contains(Attr const &t) const { return (this->find(t) != end()); }
};

} // namespace ax
