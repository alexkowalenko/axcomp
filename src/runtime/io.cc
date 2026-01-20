//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <cstdio>
#include <locale>
#include <sstream>

#include <unicode/unistr.h>

#include "ax.hh"

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
        std::printf("%.*G", static_cast<int>(n), x);
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
    const auto  ustr = icu::UnicodeString::fromUTF32(reinterpret_cast<const UChar32 *>(&x), 1);
    std::string utf8;
    ustr.toUTF8String(utf8);
    std::fputs(utf8.c_str(), stdout);
}

extern "C" void Out_String(String x) {
    if (x == nullptr) {
        return;
    }
    const auto len = std::wcslen(x);
    const auto ustr = icu::UnicodeString::fromUTF32(reinterpret_cast<const UChar32 *>(x),
                                                    static_cast<int32_t>(len));

    std::string utf8;
    ustr.toUTF8String(utf8);
    std::fputs(utf8.c_str(), stdout);
}

extern "C" void Out_Ln(void) {
    std::fputc('\n', stdout);
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
