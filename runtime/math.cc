//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <cmath>

#include "ax.hh"

namespace ax {

extern "C" Int ABS(Int x) {
    return std::abs(x);
}

extern "C" Int ASH(Int x, Int n) {
    return x << n;
}

extern "C" Bool ODD(Int x) {
    return x % 2 == 1;
}

} // namespace ax