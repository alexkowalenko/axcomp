//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <iostream>

#include <CLI/CLI.hpp>

#include "codegen.hh"
#include "error.hh"
#include "lexer.hh"
#include "options.hh"
#include "parser.hh"
#include "printer.hh"

using namespace ax;

int main(int argc, char **argv) {

    Options  options;
    CLI::App app{"AX Oberon compiler"};

    std::string debug_options;
    app.add_option("-D", debug_options, "Debug options : p=parse");
    if (debug_options.find('p') != std::string::npos) {
        options.debug_parse = true;
    }

    app.add_option("--file,-f", options.file_name, "file to compile")
        ->check(CLI::ExistingFile);

    app.add_flag("--main,-m", options.main_module, "compile as main module");
    app.add_flag("--ll,-l", options.only_ll, "generate only the .ll file");

    CLI11_PARSE(app, argc, argv);

    std::istream *input{&std::cin};
    if (options.file_name.size() > 0) {
        input = new std::ifstream(options.file_name);
    }
    ax::Lexer  lexer(*input);
    ax::Parser parser(lexer);

    try {
        auto ast = parser.parse();

        if (options.debug_parse) {
            ax::ASTPrinter printer(std::cout);
            printer.print(ast);
        }

        ax::CodeGenerator code(options);
        code.generate(ast);

    } catch (ax::AXException &e) {
        std::cout << e.error_msg() << std::endl;
        return -1;
    } catch (std::exception &e) {
        std::cout << "Exception " << e.what() << std::endl;
        return -1;
    }
    return 0;
}