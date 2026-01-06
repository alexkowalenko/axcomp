//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <string>

namespace ax {

class Location {

  public:
    Location() = default;
    Location(int l, int c) : lineno(l), charpos(c){};
    ~Location() = default;

    Location(Location const &) = default;
    Location &operator=(Location const &) = default;

    explicit operator std::string() const;

    int lineno{0};
    int charpos{0};
};

}; // namespace ax
