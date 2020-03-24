//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "ast.hh"

#include <strstream>

#include "printer.hh"

namespace ax {

ASTBase::operator std::string() {
    std::ostrstream s_out;
    ax::ASTPrinter  printer(s_out);
    this->accept(&printer);
    s_out.put(0); // needs a zero to terminate the string.
    return s_out.str();
}

} // namespace ax