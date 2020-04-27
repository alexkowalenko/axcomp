//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <llvm/ADT/StringMap.h>

#include "ast.hh"
#include "error.hh"
#include "symboltable.hh"
#include "typetable.hh"

namespace ax {

class Importer {
  public:
    explicit Importer(ErrorManager &e) : errors{e} {};
    virtual ~Importer() = default;

    void set_search_path(std::string const &path);

    virtual bool find_module(std::string const &name, SymbolFrameTable &symbols, TypeTable &types);

  private:
    std::optional<SymbolFrameTable> read_module(std::string const &name, TypeTable &types);

    ErrorManager &                    errors;
    llvm::StringMap<SymbolFrameTable> cache;
    std::vector<std::string>          paths;
};
} // namespace ax