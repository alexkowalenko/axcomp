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

    bool find_module(std::string const &                    name,
                     std::shared_ptr<SymbolTable<TypePtr>> &symbols,
                     TypeTable &                            types);

  private:
    std::shared_ptr<SymbolTable<TypePtr>> read_module(std::string const &name,
                                                      TypeTable &        types);

    void transfer_symbols(std::shared_ptr<SymbolTable<TypePtr>> const &from,
                          std::shared_ptr<SymbolTable<TypePtr>> &      to,
                          std::string const &module_name);

    ErrorManager &                                               errors;
    std::map<std::string, std::shared_ptr<SymbolTable<TypePtr>>> cache;
};
} // namespace ax