//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "type.hh"

#include <fmt/core.h>

namespace ax {

bool Type::equiv(std::shared_ptr<Type> t) {
    return std::string(*this) == std::string(*t);
}

SimpleType::operator std::string() {
    return name;
}

ProcedureType::operator std::string() {
    std::string res{"("};
    for (auto &t : params) {
        res += std::string(*t.get());
        if (t != *(params.end() - 1)) {
            res += ",";
        }
    }
    res += "):";
    res += std::string(*ret);
    return res;
}

} // namespace ax
