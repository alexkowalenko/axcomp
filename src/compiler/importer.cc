//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "importer.hh"

#include <fstream>
#include <memory>
#include <optional>
#include <sstream>

#include <dirent.h>
#include <format>

#include <llvm/ADT/Statistic.h>
#include <llvm/Support/Debug.h>

#include "ast.hh"
#include "defparser.hh"
#include "error.hh"
#include "inspector.hh"
#include "lexer.hh"
#include "type.hh"

namespace ax {

constexpr auto DEBUG_TYPE{"importer "};

template <typename S, typename... Args> static void debug(const S &format, const Args &...msg) {
    LLVM_DEBUG(llvm::dbgs() << DEBUG_TYPE << std::vformat(format, std::make_format_args(msg...))
                            << '\n'); // NOLINT
}

STATISTIC(st_imports, "Number of imports");

constexpr auto suffix{".def"};

void Importer::set_search_path(std::string const &path) {
    std::stringstream ss(path);
    std::string       item;
    while (getline(ss, item, ':')) {
        if (!item.empty()) {
            paths.push_back(item);
        }
    }
}

std::optional<SymbolFrameTable> Importer::read_module(std::string const &name, TypeTable &types) {
    debug("Importer::read_module {0}", name);

    struct DirCloser {
        void operator()(DIR *dp) const { closedir(dp); }
    };

    for (const auto &path : paths) {

        auto *dp = opendir(path.c_str());
        if (dp == nullptr) {
            throw CodeGenException("Can't open {0}", path);
        }
        std::unique_ptr<DIR, DirCloser> const dir(dp);

        struct dirent *in_file = nullptr;
        while ((in_file = readdir(dir.get()))) {
            std::string const fname(in_file->d_name);
            if (fname == "." || fname == "..") {
                continue;
            }
            if (fname.ends_with(suffix)) {
                auto dname = fname.substr(0, fname.find_last_of('.'));
                if (dname == name) {
                    auto full_path = path + '/';
                    full_path += fname;
                    SymbolFrameTable module_symbols;
                    std::ifstream    is(full_path);
                    try {
                        Lexer     lex(is, errors);
                        DefParser parser(lex, module_symbols, types, errors);
                        auto      ast = parser.parse();
                        Inspector inspect(module_symbols, types, errors, *this);
                        inspect.check(ast);
                    } catch (AXException const &e) {
                        throw CodeGenException("Importer MODULE {0} error: {1} at: {2}", name,
                                               e.error_msg(), full_path);
                    }
                    ++st_imports;
                    return module_symbols;
                }
            }
        }
    }
    return std::nullopt;
}

void transfer_symbols(const SymbolFrameTable &from, SymbolFrameTable &to,
                      std::string const &module_name) {
    for (const auto &iter : from) {
        std::string const n = ASTQualident_::make_coded_id(module_name, std::string(iter.first));
        iter.second->set(Attr::global_var);
        to.put(n, iter.second);
    }
}

bool Importer::find_module(std::string const &name, SymbolFrameTable &symbols, TypeTable &types) {
    debug("Importer::find_module {0}", name);
    // Look at cache

    if (auto res = cache.find(name); res != cache.end()) {
        transfer_symbols(res->second, symbols, name);
        return true;
    }

    if (auto mod_symbols = read_module(name, types); mod_symbols) {
        transfer_symbols(*mod_symbols, symbols, name);
        cache[name] = *mod_symbols;
        return true;
    }
    return false;
}

} // namespace ax