//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <climits>
#include <cwctype>

// Common defintions to be shared with the runtime.

namespace ax {

constexpr int MAX_STR_LITTERAL = 65536;

// Types used in implementation

using Int = long;
using Real = double;
using Bool = bool;
using Char = wchar_t;
using String = wchar_t *;

using Set = unsigned long;
constexpr int SET_MAX = sizeof(Set) * CHAR_BIT - 1;

} // namespace ax