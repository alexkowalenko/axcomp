//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <codecvt>
#include <cstdio>
#include <locale>
#include <sstream>

#include "ax.hh"

inline std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;

namespace ax {

extern "C" void Out_Open(void) {}

extern "C" void Out_Flush(void) {
    std::fflush(stdout);
}

extern "C" void Out_Int(Int x, Int n) {
    std::printf("%*li", int(n), x);
}

extern "C" void Out_Hex(Int x, Int n) {
    std::printf("%*lx", int(n), x);
}

extern "C" void Out_Bool(Bool x) {
    std::printf(x ? "1" : "0");
}

extern "C" void Out_Real(Real x, Int n) {
    if (n == 0) {
        std::printf("%G", x);
    } else {
        std::printf("%.*G", int(n), x);
    }
}

extern "C" void Out_LongReal(Real x, Int n) {
    Out_Real(x, n);
}

extern "C" void Out_Set(Set x) {
    std::stringstream result;
    result << '{';
    for (auto i = 0; i < SET_MAX; i++) {
        if (x & (1UL << i)) {
            result << i << ',';
        }
    }
    auto str = result.str();
    if (str.size() > 1) {
        str.erase(str.end() - 1);
    }
    str += '}';
    std::printf("%s", str.c_str());
}

extern "C" void Out_Char(Char x) {
    std::fputs(converterX.to_bytes(std::wstring(1, x)).c_str(), stdout);
}

extern "C" void Out_String(String x) {
    std::fputs(converterX.to_bytes(x).c_str(), stdout);
}

extern "C" void Out_Ln(void) {
    std::putc('\n', stdout);
}

extern "C" void WriteLn(void) {
    Out_Ln();
}

extern "C" void WriteInt(Int x) {
    Out_Int(x, 0);
}

extern "C" void WriteBoolean(Bool x) {
    Out_Bool(x);
}

extern "C" void HALT(Int x) {
    exit(static_cast<int>(x));
}

} // namespace ax
