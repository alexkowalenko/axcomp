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
#include "llvm/Support/CommandLine.h"
#pragma clang diagnostic pop

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

    cl::OptionCategory oberon("Oberon compiler");

    cl::opt<bool> defs("defs", cl::desc("(-d) generate only the .def file"), cl::cat(oberon));
    cl::alias     defsA("d", cl::aliasopt(defs));

    cl::opt<bool> main("main", cl::desc("(-m) generate function main()"), cl::cat(oberon));
    cl::alias     mainA("m", cl::aliasopt(main));

    cl::opt<bool> output("output_funct", cl::desc("(-o) generate compiler test function output()"),
                         cl::cat(oberon));
    cl::alias     outputA("o", cl::aliasopt(output));

    cl::opt<bool> ll("ll", cl::desc("(-l) generate only the .ll file"), cl::cat(oberon));
    cl::alias     llA("l", cl::aliasopt(ll));

    cl::opt<bool> opt1("O1", cl::desc("invoke optimizer level 1"), cl::cat(oberon));
    cl::opt<bool> opt2("O2", cl::desc("invoke optimizer level 2"), cl::cat(oberon));
    cl::opt<bool> opt3("O3", cl::desc("invoke optimizer level 3"), cl::cat(oberon));

    cl::opt<bool> symbols("symbols", cl::desc("(-s) dump the symbol table"), cl::cat(oberon));
    cl::alias     symbolsA("s", cl::aliasopt(symbols));

    cl::opt<std::string> file_name(cl::Positional, cl::desc("<input file>"));

    cl::opt<std::string> axlib_path("axlib_path", cl::desc("(-L) path searched for runtime files"),
                                    cl::init(axlib), cl::cat(oberon));
    cl::alias            axlib_pathA("L", cl::aliasopt(axlib_path));

    cl::opt<bool> parse("p", cl::desc("parse only"), cl::cat(oberon));

    cl::opt<bool>         dbg("debug", cl::desc("turn on debugging"), cl::cat(oberon));
    cl::list<std::string> doptions("debug-only", cl::desc("debug options"), cl::cat(oberon));
    // cl::opt<bool>         stats("stats", cl::desc("show statistics"), cl::cat(oberon));

    cl::ParseCommandLineOptions(argc, argv, "AX Oberon compiler");

    options.output_defs = defs;
    options.output_main = main;
    options.output_funct = output;
    options.only_ll = ll;
    if (opt1) {
        options.optimise = 1;
    }
    if (opt2) {
        options.optimise = 2;
    }
    if (opt3) {
        options.optimise = 3;
    }

    options.print_symbols = symbols;
    options.file_name = file_name;
    options.axlib_path = axlib_path;
    options.debug_parse = parse;

    llvm::DebugFlag = dbg;
    std::for_each(begin(doptions), end(doptions),
                  [](auto x) { llvm::setCurrentDebugType(x.c_str()); });
    // if (stats) {
    //    llvm::EnableStatistics(stats);
    // }
    llvm::EnableStatistics(true);

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

        // Always generate .def files
        output_defs(ast, options);
        if (options.output_defs) {
            return 0;
        }

        if (options.print_symbols) {
            dump_symbols(symbols, types);
        }

        ax::CodeGenerator code(options, symbols, types, importer);
        code.generate(ast);

        if (options.optimise) {
            code.optimize();
        }

        code.generate_llcode();
        if (!options.only_ll) {
            code.generate_objectcode();
        }

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