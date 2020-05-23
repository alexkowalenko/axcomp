//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <codecvt>
#include <cstdio>
#include <locale>

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

extern "C" void Out_Real(Real x) {
    std::printf("%G", x);
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
    exit(x);
}

} // namespace ax
