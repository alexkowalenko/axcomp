//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "ast/all.hh"

#include <sstream>

#include "printer.hh"

namespace ax {

ASTBase_::operator std::string() {
    std::ostringstream s_out;
    ax::ASTPrinter     printer(s_out);
    this->accept(&printer);
    return s_out.str();
}

ASTIdentifier ASTDesignator_::first_field() const {
    if (selectors.empty()) {
        return nullptr;
    }
    if (const auto *val = std::get_if<FieldRef>(selectors.data()); val) {
        return val->first;
    }
    return nullptr;
}

} // namespace ax