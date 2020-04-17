//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <iostream>

#include "ax.hh"

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

} // namespace ax
