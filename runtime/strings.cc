//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "ax.hh"

#include <cctype>
#include <cstring>
#include <cwchar>

#include <gc.h>

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

static bool init_gc{false};

void gc_init() {
    GC_init();
    init_gc = true;
}

extern "C" void NEW_String(String **ptr, Int x) {
    if (!init_gc) {
        gc_init();
    }
    *ptr = static_cast<String *>(GC_malloc(x));
    memset(*ptr, 0, x);
}

extern "C" void COPY(String x, String **v) {
    if (!init_gc) {
        gc_init();
    }
    auto len = std::wcslen(x) * sizeof(wchar_t);
    *v = static_cast<String *>(GC_malloc(len));
    // Opposite to Oberon
    std::wcscpy(*(String *)v, x);
}

extern "C" void NEW_Array(void **ptr, Int size) {
    if (!init_gc) {
        gc_init();
    }
    *ptr = static_cast<void *>(GC_malloc(size));
    memset(*ptr, 0, size);
}

} // namespace ax