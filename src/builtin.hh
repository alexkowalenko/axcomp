//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <utility>
#include <vector>

#include <llvm/ADT/StringMap.h>

#include "codegen.hh"
#include "symboltable.hh"
#include "type.hh"

namespace ax {

using BIFunctor = std::function<Value *(CodeGenerator *, ASTCall const &ast)>;

class Builtin {
  public:
    static void initialise(SymbolFrameTable &symbols);

    static std::vector<std::pair<std::string, Symbol>> global_functions;
    static llvm::StringMap<BIFunctor>                  compile_functions;
};

} // namespace ax