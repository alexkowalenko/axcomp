//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <iostream>

#include <CLI/CLI.hpp>

#include "codegen.hh"
#include "defprinter.hh"
#include "error.hh"
#include "inspector.hh"
#include "lexer.hh"
#include "options.hh"
#include "parser.hh"
#include "printer.hh"
#include "typetable.hh"

using namespace ax;

void output_defs(std::shared_ptr<ASTModule> ast, Options const &options) {
    std::string def_file{"out.def"};
    if (!options.file_name.empty()) {
        auto filename = options.file_name;
        def_file = filename.substr(0, filename.rfind(".")) + ".def";
    }

    std::ofstream output(def_file);
    DefPrinter    defs(output);
    defs.print(ast);
}

int main(int argc, char **argv) {

    Options  options;
    CLI::App app{"AX Oberon compiler"};

    std::string debug_options;
    app.add_option("-D", debug_options, "Debug options : p=parse");
    app.add_flag("--defs, -d", options.output_defs, "output .def file");
    app.add_option("--file,-f", options.file_name, "file to compile")
        ->check(CLI::ExistingFile);
    app.add_flag("--output_funct,-o", options.output_funct,
                 "compile as test function output");
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
    ErrorManager errors;
    Lexer        lexer(*input, errors);

    TypeTable types;
    types.initialise();

    auto       symbols = std::make_shared<SymbolTable<TypePtr>>(nullptr);
    ax::Parser parser(lexer, symbols, types, errors);
    parser.setup_builtins();

    try {
        auto ast = parser.parse();

        if (options.debug_parse) {
            ax::ASTPrinter printer(std::cout);
            printer.print(ast);
        }

        Inspector inspect(symbols, types, errors);
        inspect.check(ast);
        if (errors.has_errors()) {
            errors.print_errors(std::cout);
            return -1;
        }

        if (options.output_defs) {
            output_defs(ast, options);
            return 0;
        }

        ax::CodeGenerator code(options, types);
        code.generate(ast);

        code.generate_llcode();
        if (!options.only_ll) {
            code.generate_objectcode();
        }

        if (options.print_symbols) {
            symbols->dump(std::cout);
        }
    } catch (ax::AXException &e) {
        std::cout << e.error_msg() << std::endl;
        if (options.print_symbols) {
            symbols->dump(std::cout);
        }
        return -1;
    } catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}