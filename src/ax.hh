//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

// Common defintions to be shared with the runtime.

namespace ax {

constexpr int MAX_STR_LITTERAL = 65536;

// Types used in implementation

using Int = long;
using Bool = bool;
using Char = wchar_t;

} // namespace ax