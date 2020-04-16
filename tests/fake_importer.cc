//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "fake_importer.hh"
#include "type.hh"
#include "typetable.hh"
#include <memory>

namespace ax {

bool FakeImporter::find_module(std::string const &name, SymbolFrameTable &symbols, TypeTable &) {

    if (name != "beta" && name != "B") {
        return false;
    }

    // CONST a : INTEGER
    auto s = ASTQualident::make_coded_id(name, "a");
    symbols.put(s, {TypeTable::IntType, Attr::cnst});

    // b- : INTEGER
    s = ASTQualident::make_coded_id(name, "b");
    symbols.put(s, {TypeTable::IntType, Attr::read_only});

    // c* : INTEGER
    s = ASTQualident::make_coded_id(name, "c");
    symbols.put(s, {TypeTable::IntType, Attr::global});

    // d : BOOLEAN
    s = ASTQualident::make_coded_id(name, "d");
    symbols.put(s, {TypeTable::BoolType, Attr::global});

    // f : (INTEGER): INTEGER
    auto type = std::make_shared<ProcedureType>();
    type->ret = TypeTable::IntType;
    type->params = ProcedureType::ParamsList{std::pair{TypeTable::IntType, Attr::null}};
    s = ASTQualident::make_coded_id(name, "f");
    symbols.put(s, {type, Attr::global});
    return true;
}
} // namespace ax