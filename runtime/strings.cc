//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "ax.hh"
#include <cctype>

namespace ax {

extern "C" Char CAP(Char x) {
    return std::toupper(x);
}

extern "C" Char CHR(Int x) {
    return Char(x);
}

extern "C" Int ORD(Char x) {
    return Int(x);
}

extern "C" Int Strings_Length(String x) {
    Int len = 0;
    while (*x++) {
        len++;
    }
    return len;
}

} // namespace ax