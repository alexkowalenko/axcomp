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

void parse_options(Options &options, std::string debugStr) {
    if (debugStr.find('p') != std::string::npos) {
        options.debug_parse = true;
    }
}

int main(int argc, char **argv) {

    CLI::App app{"AX Oberon compiler"};

    std::string debug_options;
    app.add_option("-D", debug_options, "Debug options : p=parse");

    std::string file;
    app.add_option("--file,-f", file, "file to compile")
        ->check(CLI::ExistingFile);
    CLI11_PARSE(app, argc, argv);

    ax::Options options;
    parse_options(options, debug_options);

    std::istream *input = &std::cin;
    if (file.size() > 0) {
        input = new std::ifstream(file);
    }
    ax::Lexer  lexer(*input);
    ax::Parser parser(lexer);

    try {
        auto ast = parser.parse();

        if (options.debug_parse) {
            ax::ASTPrinter printer(std::cout);
            printer.print(ast);
        }

        ax::CodeGenerator code;
        code.generate(ast);

    } catch (ax::AXException &e) {
        std::cerr << e.error_msg() << std::endl;
        return -1;
    } catch (std::exception &e) {
        std::cerr << "Exception " << e.what() << std::endl;
        return -1;
    }
    return 0;
}