//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <codecvt>
#include <iostream>

#include "ax.hh"

inline std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;

namespace ax {

extern "C" void Out_Open(void) {}

extern "C" void Out_Flush(void) {
    std::cout << std::flush;
}

extern "C" void Out_Int(Int x) {
    std::cout << x;
}

extern "C" void Out_Hex(Int x) {
    std::cout << std::hex << x << std::dec;
}

extern "C" void Out_Bool(Bool x) {
    std::cout << x;
}

extern "C" void Out_Char(Char x) {
    std::cout << converterX.to_bytes(std::wstring(1, x));
}

extern "C" void Out_String(String x) {
    std::cout << converterX.to_bytes(x);
}

extern "C" void Out_Ln(void) {
    std::cout << std::endl;
}

extern "C" void WriteLn(void) {
    Out_Ln();
}

extern "C" void WriteInt(Int x) {
    Out_Int(x);
}

extern "C" void WriteBoolean(Bool x) {
    Out_Bool(x);
}

extern "C" void HALT(Int x) {
    exit(x);
}

} // namespace ax
