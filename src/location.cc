//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "location.hh"

#include <llvm/Support/FormatVariadic.h>

namespace ax {

Location::operator std::string() {
    return llvm::formatv("{0},{1}", lineno, charpos);
}

}; // namespace ax