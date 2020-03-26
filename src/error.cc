//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <algorithm>
#include <iostream>

#include <llvm/Support/FormatVariadic.h>

#include "error.hh"

namespace ax {

std::string AXException::error_msg() const {
    return std::string(llvm::formatv("{0}: {1}", std::string(location), msg));
}

void ErrorManager::print_errors(std::ostream &out) {
    std::for_each(begin(error_list), end(error_list),
                  [&](auto const &e) { out << e.error_msg() << std::endl; });
}

} // namespace ax