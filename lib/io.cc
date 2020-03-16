//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <iostream>

using INTEGER = long;

extern "C" void WriteLn(void) {
    std::cout << std::endl;
}

extern "C" void WriteInt(INTEGER x) {
    std::cout << x;
}