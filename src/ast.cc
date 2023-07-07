//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "ast.hh"

#include <strstream>

#include "printer.hh"

namespace ax {

ASTBase_::operator std::string() {
    std::ostrstream s_out;
    ax::ASTPrinter  printer(s_out);
    this->accept(&printer);
    s_out.put(0); // needs a zero to terminate the string.
    return s_out.str();
}

ASTIdentifierPtr ASTDesignator::first_field() const {
    if (selectors.empty()) {
        return nullptr;
    }
    if (const auto *val = std::get_if<FieldRef>(&selectors[0]); val) {
        return val->first;
    }
    return nullptr;
}

} // namespace ax