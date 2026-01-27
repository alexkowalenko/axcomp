//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <cstdlib>
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
#include "options.hh"
#include "parser.hh"
#include "printer.hh"
#include "type.hh"
#include "typetable.hh"

using namespace ax;

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
    for (const auto &[name, snd] : types) {
        std::cout << std::string(name) << " : " << string(snd->id) << "\n";
    }
}

Options do_args(int argc, char **argv) {
    Options options;

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

    app.add_argument("-L").help("path searched for runtime files").store_into(options.axlib_path);

    app.add_argument("-p", "--parse")
        .help("parse only")
        .flag()
        .default_value(false)
        .store_into(options.debug_parse);

    app.add_argument("-g", "--debug")
        .help("turn on debugging")
        .flag()
        .default_value(false)
        .store_into(options.debug);

    std::vector<std::string> doptions;
    app.add_argument("-dg", "--debug_options")
        .help("debug options")
        .append()
        .nargs(argparse::nargs_pattern::at_least_one)
        .store_into(doptions);

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

    try {
        app.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        std::println("{}", err.what());
        exit(1);
    }

    llvm::DebugFlag = options.debug;
    for (auto const &x : doptions) {
        llvm::setCurrentDebugType(x.c_str());
        llvm::DebugFlag = true;
    }

    if (stats) {
        llvm::EnableStatistics(stats);
    }

    if (std::ranges::find(doptions, "config") != doptions.end()) {
        std::println("Options: file: {}", options.file_name);
        std::println("         axlib: {}", options.axlib_path);
        std::println("         output_funct: {}", options.output_funct);
        std::println("         output_main: {}", options.output_main);
        std::println("         output_defs: {}", options.output_defs);
        std::println("         only_ll: {}", options.only_ll);
        std::println("         debug: {}", options.debug);
        for (auto const &x : doptions) {
            std::println("         debug_option: {}", x);
        }
    }

    return options;
}

int main(int argc, char **argv) {

    Options options = do_args(argc, argv);

    ErrorManager errors;
    std::string  input_text;
    if (!options.file_name.empty()) {
        std::ifstream file(options.file_name, std::ios::binary);
        if (!file) {
            std::println("Cannot open {}", options.file_name);
            return EXIT_FAILURE;
        }
        input_text.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    } else {
        // This should not happen, as the filename is a compulsory option.
        input_text.assign(std::istreambuf_iterator<char>(std::cin),
                          std::istreambuf_iterator<char>());
    }
    LexerUTF8 lexer(std::move(input_text), errors);

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
            return EXIT_SUCCESS;
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
            return EXIT_FAILURE;
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
        return EXIT_FAILURE;
    } catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
