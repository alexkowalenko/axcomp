//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <iostream>

#include <CLI/CLI.hpp>

#include "codegen.hh"
#include "error.hh"
#include "inspector.hh"
#include "lexer.hh"
#include "options.hh"
#include "parser.hh"
#include "printer.hh"
#include "symbol.hh"
#include "typetable.hh"

using namespace ax;

int main(int argc, char **argv) {

    Options  options;
    CLI::App app{"AX Oberon compiler"};

    std::string debug_options;
    app.add_option("-D", debug_options, "Debug options : p=parse");
    app.add_option("--file,-f", options.file_name, "file to compile")
        ->check(CLI::ExistingFile);
    app.add_flag("--main,-m", options.main_module, "compile as main module");
    app.add_flag("--ll,-l", options.only_ll, "generate only the .ll file");
    app.add_flag("--symbols,-s", options.print_symbols, "print symbol table");

    CLI11_PARSE(app, argc, argv);
    if (debug_options.find('p') != std::string::npos) {
        options.debug_parse = true;
        std::cout << "Print parsed program.\n";
    }

    std::istream *input{&std::cin};
    if (!options.file_name.empty()) {
        input = new std::ifstream(options.file_name);
    }
    Lexer lexer(*input);

    TypeTable types;
    types.initialise();

    SymbolTable<Symbol> symbols(nullptr);
    ax::Parser          parser(lexer, symbols);

    try {
        auto ast = parser.parse();

        if (options.debug_parse) {
            ax::ASTPrinter printer(std::cout);
            printer.print(ast);
        }

        Inspector inpect(symbols, types);
        inpect.check(ast);

        ax::CodeGenerator code(options);
        code.generate(ast);

        if (options.print_symbols) {
            symbols.dump(std::cout);
        }

    } catch (ax::AXException &e) {
        std::cout << e.error_msg() << std::endl;
        return -1;
    } catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}