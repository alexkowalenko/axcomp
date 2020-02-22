//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <iostream>

#include "codegen.hh"
#include "error.hh"
#include "lexer.hh"
#include "parser.hh"
#include "printer.hh"

int main() {
    ax::Lexer  lexer(std::cin);
    ax::Parser parser(lexer);

    try {
        auto ast = parser.parse();

        ax::ASTPrinter printer(std::cout);
        printer.print(ast);

        ax::CodeGenerator code;
        code.generate(ast);

    } catch (ax::AXException &e) {
        std::cerr << e.error_msg() << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "Unknown error " << std::endl;
        return -1;
    }
    return 0;
}