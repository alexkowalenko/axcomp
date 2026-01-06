//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "codegen.hh"
#include "symboltable.hh"
#include "type.hh"

namespace ax {

using BIFunctor = std::function<Value *(CodeGenerator *, ASTCall const &ast)>;

class Builtin {
  public:
    static void initialise(SymbolFrameTable &symbols);

    inline static std::vector<std::pair<std::string, Symbol>> global_functions;
    inline static std::map<std::string, BIFunctor>            compile_functions;
};

} // namespace ax