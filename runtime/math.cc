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

// No .o file is produced so need these constants to be repeated here
extern "C" Real Math_pi = 3.14159265358979323846;
extern "C" Real Math_e = 2.7182818284590452354;
extern "C" Real Math_ln2 = 0.693147180559945309417232121458;
extern "C" Real Math_eps = 2.2e-16;

extern "C" Real Math_sin(Real x) {
    return sin(x);
}

extern "C" Real Math_cos(Real x) {
    return cos(x);
}

extern "C" Real Math_arctan(Real x) {
    return atan(x);
}

extern "C" Real Math_sqrt(Real x) {
    return sqrt(x);
}

extern "C" Real Math_ln(Real x) {
    return log(x);
}

extern "C" Real Math_exp(Real x) {
    return exp(x);
}

} // namespace ax