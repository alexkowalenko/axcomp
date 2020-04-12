//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "importer.hh"

#include <fstream>

#include <dirent.h>
#include <sys/types.h>

#include "llvm/Support/FormatVariadic.h"

#include "ast.hh"
#include "defparser.hh"
#include "inspector.hh"
#include "lexer.hh"

namespace ax {

const std::string suffix{".def"};

bool ends_with(std::string const &s) {
    if (s.length() < suffix.length()) {
        return false;
    }
    return s.substr(s.length() - suffix.length(), s.length()) == suffix;
}

bool Importer::find_module(std::string const &                    name,
                           std::shared_ptr<SymbolTable<TypePtr>> &symbols,
                           TypeTable &                            types) {
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

            fname =
                std::string(in_file->d_name).substr(0, fname.find_last_of('.'));

            if (fname == name) {
                std::cout << "module: " << fname << '\n';
                auto module_symbols =
                    std::make_shared<SymbolTable<TypePtr>>(nullptr);
                std::ifstream is(in_file->d_name);
                Lexer         lex(is, errors);
                DefParser     parser(lex, module_symbols, types, errors);
                auto          ast = parser.parse();
                Inspector     inpect(module_symbols, types, errors);
                inpect.check(ast);

                // Import the symbols into the top level symbol table
                std::for_each(module_symbols->cbegin(), module_symbols->cend(),
                              [this, name, symbols](auto const &s) {
                                  auto n = ASTQualident::make_coded_id(
                                      name, s.first); // name + "_" + s.first;
                                  symbols->put(n, s.second);
                              });

                return true;
            }
        }
    }
    return false;
}

} // namespace ax