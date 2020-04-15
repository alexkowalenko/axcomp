//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "importer.hh"

#include <cstddef>
#include <fstream>

#include <dirent.h>
#include <sys/types.h>

#include "llvm/Support/FormatVariadic.h"

#include "ast.hh"
#include "defparser.hh"
#include "inspector.hh"
#include "lexer.hh"
#include "type.hh"

namespace ax {

constexpr auto suffix{".def"};

bool ends_with(std::string const &s) {
    if (s.length() < std::strlen(suffix)) {
        return false;
    }
    return s.substr(s.length() - std::strlen(suffix), s.length()) == suffix;
}

Symbols Importer::read_module(std::string const &name, TypeTable &types) {

    Symbols     module_symbols = nullptr;
    std::string path = ".";

    auto *dir = opendir(path.c_str());
    if (dir == nullptr) {
        throw CodeGenException(llvm::formatv("Can't open {0}", path));
    }

    struct dirent *in_file = nullptr;
    while ((in_file = readdir(dir))) {
        if (!strcmp(in_file->d_name, ".")) {
            continue;
        }
        if (!strcmp(in_file->d_name, "..")) {
            continue;
        }
        if (ends_with(in_file->d_name)) {
            std::string fname(in_file->d_name);

            fname = std::string(in_file->d_name).substr(0, fname.find_last_of('.'));

            if (fname == name) {
                module_symbols = make_Symbols(nullptr);
                std::ifstream is(in_file->d_name);
                Lexer         lex(is, errors);
                DefParser     parser(lex, module_symbols, types, errors);
                auto          ast = parser.parse();
                Inspector     inpect(module_symbols, types, errors, *this);
                inpect.check(ast);
                return module_symbols;
            }
        }
    }
    return module_symbols;
}

void Importer::transfer_symbols(Symbols const &from, Symbols &to, std::string const &module_name) {
    std::for_each(from->cbegin(), from->cend(), [this, module_name, to](auto const &s) {
        auto n = ASTQualident::make_coded_id(module_name, s.first);
        to->put(n, s.second);
    });
}

bool Importer::find_module(std::string const &name, Symbols &symbols, TypeTable &types) {
    // Look at cache

    if (auto res = cache.find(name); res != cache.end()) {
        transfer_symbols(res->second, symbols, name);
        return true;
    }

    auto mod_symbols = read_module(name, types);
    if (mod_symbols) {
        transfer_symbols(mod_symbols, symbols, name);
        return true;
    }
    return false;
}

} // namespace ax