//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include "importer.hh"

namespace ax {

class FakeImporter : public Importer {
  public:
    explicit FakeImporter(ErrorManager &e) : Importer{e} {};

    bool find_module(std::string const &name, Symbols &symbols, TypeTable &types) override;

  private:
};
} // namespace ax
