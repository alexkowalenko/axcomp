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
    explicit Importer(ErrorManager &e) : errors{e} {};

    bool find_module(std::string const &name, Symbols &symbols,
                     TypeTable &types);

  private:
    Symbols read_module(std::string const &name, TypeTable &types);

    void transfer_symbols(Symbols const &from, Symbols &to,
                          std::string const &module_name);

    ErrorManager &                 errors;
    std::map<std::string, Symbols> cache;
};
} // namespace ax