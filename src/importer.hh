//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include "ast.hh"
#include "error.hh"
#include "symboltable.hh"
#include "typetable.hh"

namespace ax {

class Importer {
  public:
    Importer(ErrorManager &e) : errors{e} {};

    bool find_module(std::string const &                    name,
                     std::shared_ptr<SymbolTable<TypePtr>> &symbols,
                     TypeTable &                            types);

  private:
    ErrorManager &errors;
};
} // namespace ax