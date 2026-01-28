//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "importer.hh"

#include <filesystem>
#include <format>
#include <fstream>
#include <memory>
#include <optional>
#include <sstream>

#include <llvm/ADT/Statistic.h>
#include <llvm/Support/Debug.h>

#include "ast/all.hh"
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

    for (const auto &path : paths) {
        std::error_code ec;
        auto            directory = std::filesystem::path(path);
        if (!std::filesystem::exists(directory, ec) ||
            !std::filesystem::is_directory(directory, ec)) {
            throw CodeGenException("Can't open {0}", path);
        }

        // Iterate through the files in the directory
        for (auto const &entry : std::filesystem::directory_iterator(directory, ec)) {
            if (ec) {
                throw CodeGenException("Can't read {0}: {1}", path, ec.message());
            }
            if (!entry.is_regular_file()) {
                continue;
            }
            auto const &file_path = entry.path();
            if (file_path.extension() != suffix) {
                continue;
            }
            auto       fname = file_path.filename().string();
            auto       dot_pos = fname.find_last_of('.');
            auto const dname = dot_pos == std::string::npos ? fname : fname.substr(0, dot_pos);
            if (dname != name) {
                continue;
            }

            // Found definition file now import it
            SymbolFrameTable module_symbols;
            std::ifstream    is(file_path, std::ios::binary);
            if (!is) {
                throw CodeGenException("Can't open {0}", file_path.string());
            }
            std::string text{std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>()};
            try {
                LexerUTF8 lex(std::move(text), errors);
                DefParser parser(lex, module_symbols, types, errors);
                auto      ast = parser.parse();
                Inspector inspect(module_symbols, types, errors, *this);
                inspect.check(ast);
            } catch (AXException const &e) {
                throw CodeGenException("Importer MODULE {0} error: {1} at: {2}", name,
                                       e.error_msg(), file_path.string());
            }
            ++st_imports;
            return module_symbols;
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

    if (const auto res = cache.find(name); res != cache.end()) {
        transfer_symbols(res->second, symbols, name);
        return true;
    }

    if (const auto mod_symbols = read_module(name, types); mod_symbols) {
        transfer_symbols(*mod_symbols, symbols, name);
        cache[name] = *mod_symbols;
        return true;
    }
    return false;
}

} // namespace ax
