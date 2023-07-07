//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "ax.hh"

#include <cctype>
#include <cstddef>
#include <cstring>
#include <cwchar>

#include <gc.h>

#pragma clang diagnostic ignored "-Wold-style-cast"

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
    return static_cast<Int>(std::wcslen(x));
}

static bool init_gc{false};

void *my_malloc(size_t size) {
    if (!init_gc) {
        GC_init();
        init_gc = true;
    }
    return GC_malloc(size);
}

extern "C" void NEW_String(String **ptr, Int x) {
    auto size = static_cast<size_t>(x);
    *ptr = static_cast<String *>(my_malloc(size));
    memset(*ptr, 0, size);
}

extern "C" void NEW_ptr(void **ptr, Int x) {
    auto size = static_cast<size_t>(x);
    *ptr = my_malloc(size);
    memset(*ptr, 0, size);
}

extern "C" void COPY(String x, String **v) {
    auto len = std::wcslen(x) * sizeof(wchar_t);
    *v = static_cast<String *>(my_malloc(len));
    // Opposite to Oberon
    std::wcscpy(*(String *)v, x);
}

extern "C" void *NEW_Array(Int x) {
    auto size = static_cast<size_t>(x);
    auto ptr = static_cast<void *>(my_malloc(size));
    memset(ptr, 0, size);
    return ptr;
}

extern "C" String Strings_Concat(String s1, String s2) {
    // printf("Strings_Concat :%S(%ld) :%S(%ld)\n", s1, std::wcslen(s1), s2, std::wcslen(s2));
    auto  len = std::wcslen(s1) + std::wcslen(s2) + 1u;
    auto *res = static_cast<String>(my_malloc(len * sizeof(String &)));
    std::wcscpy(res, s1);
    std::wcscat(res, s2);
    return res;
}

extern "C" String Strings_ConcatChar(String s, Char c) {
    auto  len = std::wcslen(s);
    auto *res = static_cast<String>(my_malloc((len + 2) * sizeof(String &)));
    std::wcscpy(res, s);
    res[len] = c;
    res[len + 1] = 0;
    return res;
}

extern "C" String Strings_AppendChar(Char c, String s) {
    auto  len = std::wcslen(s);
    auto *res = static_cast<String>(my_malloc((len + 2) * sizeof(String &)));
    res[0] = c;
    std::wcscpy(res + 1, s);
    res[len + 1] = 0;
    return res;
}

extern "C" Int Strings_Compare(String s1, String s2) {
    return std::wcscmp(s1, s2);
}

} // namespace ax