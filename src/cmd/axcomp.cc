//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <fstream>
#include <iostream>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshadow"
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wunused-parameter"
#include "llvm/ADT/Statistic.h"
#pragma clang diagnostic pop

#include <argparse/argparse.hpp>

#include "builtin.hh"
#include "codegen.hh"
#include "defprinter.hh"
#include "error.hh"
#include "importer.hh"
#include "inspector.hh"
#include "lexer.hh"
#include "lexerUTF8.hh"
#include "options.hh"
#include "parser.hh"
#include "printer.hh"
#include "type.hh"
#include "typetable.hh"

using namespace ax;

constexpr auto std_path = ".";

std::string getEnvVar(std::string const &key) {
    char *val = std::getenv(key.c_str());
    return val == nullptr ? std::string("") : std::string(val);
}

void output_defs(std::shared_ptr<ASTModule_> const &ast, Options const &options) {
    std::string def_file{"out.def"};
    if (!options.file_name.empty()) {
        auto filename = options.file_name;
        def_file = filename.substr(0, filename.rfind('.')) + ".def";
    }

    std::ofstream output(def_file);
    DefPrinter    defs(output);
    defs.print(ast);
}

void dump_symbols(SymbolFrameTable &symbols, TypeTable &types) {
    symbols.dump(std::cout);
    std::cout << "- Types -------------------------------\n";
    std::for_each(types.begin(), types.end(), [](auto &t) {
        std::cout << std::string(t.first) << " : " << string(t.second->id) << "\n";
    });
}

Options do_args(int argc, char **argv) {
    Options options;

    auto axlib = getEnvVar("AXLIB_PATH");
    if (axlib.empty()) {
        axlib = std_path;
    }

    argparse::ArgumentParser app{"Oberon compiler", "0.3"};

    app.add_argument("-d", "--defs")
        .help("generate only the .def file")
        .flag()
        .store_into(options.output_defs);

    app.add_argument("-m", "--main")
        .help("generate function main()")
        .flag()
        .default_value(false)
        .store_into(options.output_main);

    app.add_argument("-o", "--output")
        .help("generate compiler output")
        .flag()
        .default_value(false)
        .store_into(options.output_funct);

    app.add_argument("-l", "--ll")
        .help("generate only the .ll file")
        .flag()
        .default_value(false)
        .store_into(options.only_ll);

    // cl::opt<bool> opt1("O1", cl::desc("invoke optimizer level 1"), cl::cat(oberon));
    // cl::opt<bool> opt2("O2", cl::desc("invoke optimizer level 2"), cl::cat(oberon));
    // cl::opt<bool> opt3("O3", cl::desc("invoke optimizer level 3"), cl::cat(oberon));

    app.add_argument("-O", "--opt");

    app.add_argument("-s", "--symbols")
        .help("dump the symbol table")
        .flag()
        .default_value(false)
        .store_into(options.print_symbols);

    app.add_argument("file_name").help("<input file>").store_into(options.file_name);

    // cl::opt<std::string> axlib_path("axlib_path", cl::desc("(-L) path searched for runtime
    // files"),
    //                                 cl::init(axlib), cl::cat(oberon));
    // cl::alias            axlib_pathA("L", cl::aliasopt(axlib_path));

    app.add_argument("-L", "--axlib_path")
        .help("path searched for runtime files")
        .default_value(axlib)
        .store_into(axlib);

    app.add_argument("-p", "--parse")
        .help("parse only")
        .flag()
        .default_value(false)
        .store_into(options.debug_parse);

    // cl::opt<bool>         dbg("debug", cl::desc("turn on debugging"), cl::cat(oberon));
    // cl::list<std::string> doptions("debug-only", cl::desc("debug options"),
    // cl::cat(oberon));
    bool dbg{false};
    app.add_argument("-g", "--debug")
        .help("turn on debugging")
        .flag()
        .default_value(false)
        .store_into(dbg);

    bool stats = false;
    app.add_argument("-st", "--stats")
        .help("show statistics")
        .flag()
        .default_value(false)
        .store_into(stats);

    // if (opt1) {
    //     options.optimise = 1;
    // }
    // if (opt2) {
    //     options.optimise = 2;
    // }
    // if (opt3) {
    //     options.optimise = 3;
    // }
    options.axlib_path = axlib;

    try {
        app.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        std::println("{}", err.what());
        exit(1);
    }

    llvm::DebugFlag = dbg;
    // std::for_each(begin(doptions), end(doptions),
    //               [](auto x) { llvm::setCurrentDebugType(x.c_str()); });

    if (stats) {
        llvm::EnableStatistics(stats);
    }

#if 0
    std::println("Options: file: {}", options.file_name);
    std::println("         axlib: {}", options.axlib_path);
    std::println("         output_funct: {}", options.output_funct);
    std::println("         output_main: {}", options.output_main);
    std::println("         output_defs: {}", options.output_defs);
    std::println("         only_ll: {}", options.only_ll);
#endif

    return options;
}

int main(int argc, char **argv) {

    Options options = do_args(argc, argv);

    std::istream *input{&std::cin};
    if (!options.file_name.empty()) {
        input = new std::ifstream(options.file_name);
    }
    ErrorManager errors;
    LexerUTF8    lexer(*input, errors);

    TypeTable types;
    types.initialise();

    SymbolFrameTable symbols;
    Builtin::initialise(symbols);
    ax::Parser parser(lexer, symbols, types, errors);

    try {
        auto ast = parser.parse();

        if (options.debug_parse) {
            ax::ASTPrinter printer(std::cout);
            printer.set_indent(4);
            printer.print(ast);
            return 0;
        }

        // Run the semantic inspector
        Importer importer(errors);
        importer.set_search_path(options.axlib_path);
        Inspector inspect(symbols, types, errors, importer);
        inspect.check(ast);
        if (errors.has_errors()) {
            errors.print_errors(std::cerr);
            if (options.print_symbols) {
                dump_symbols(symbols, types);
            }
            return -1;
        }

        if (options.output_defs) {
            output_defs(ast, options);
        }

        if (options.print_symbols) {
            dump_symbols(symbols, types);
        }

        ax::CodeGenerator code(options, symbols, types, importer);
        code.generate(ast);

        if (options.optimise) {
            code.optimize();
        }

        if (options.only_ll) {
            code.generate_llcode();
        }
        code.generate_objectcode();

        if (AreStatisticsEnabled()) {
            PrintStatistics();
        }
    } catch (ax::AXException &e) {
        std::cout << e.error_msg() << std::endl;
        if (options.print_symbols) {
            dump_symbols(symbols, types);
        }
        return -1;
    } catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}