//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "fake_importer.hh"
#include "typetable.hh"

namespace ax {

bool FakeImporter::find_module(std::string const &name, Symbols &symbols, TypeTable &) {

    if (name != "beta") {
        return false;
    }

    // CONST a : INTEGER
    auto s = ASTQualident::make_coded_id(name, "a");
    symbols->put(s, {TypeTable::IntType, Attr::cnst});

    // b- : INTEGER
    s = ASTQualident::make_coded_id(name, "b");
    symbols->put(s, {TypeTable::IntType, Attr::read_only});

    // c* : INTEGER
    s = ASTQualident::make_coded_id(name, "c");
    symbols->put(s, {TypeTable::IntType, Attr::global});

    // d : BOOLEAN
    s = ASTQualident::make_coded_id(name, "d");
    symbols->put(s, {TypeTable::BoolType, Attr::global});

    return true;
}
} // namespace ax