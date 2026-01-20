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
    Location(const int l, const int c) : line_no(l), char_pos(c) {};
    ~Location() = default;

    Location(Location const &) = default;
    Location &operator=(Location const &) = default;

    explicit operator std::string() const;

    int line_no{0};
    int char_pos{0};
};

}; // namespace ax
