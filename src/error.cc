//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <llvm/Support/FormatVariadic.h>

#include "error.hh"

namespace ax {

std::string AXException::error_msg() {
    return std::string(llvm::formatv("{0}: {1}", std::string(location), msg));
}

} // namespace ax