//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "location.hh"

#include <format>

namespace ax {

Location::operator std::string() const {
    if (line_no == 0 && char_pos == 0) {
        return std::string{"[X]"};
    }
    return std::format("[{},{}]", line_no, char_pos);
}

}; // namespace ax