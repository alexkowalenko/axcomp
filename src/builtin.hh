//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <map>
#include <utility>
#include <vector>

#include "codegen.hh"
#include "symboltable.hh"
#include "type.hh"

namespace ax {

using BIFunctor = std::function<Value *(CodeGenerator *, ASTCallPtr ast)>;

class Builtin {
  public:
    static void initialise(SymbolFrameTable &symbols);

    static std::vector<std::pair<std::string, Symbol>> global_functions;
    static std::map<std::string, BIFunctor>            compile_functions;
};

} // namespace ax