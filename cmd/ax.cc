//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <iostream>

#include "lexer.hh"

int main() {
    ax::Lexer lexer(std::cin);

    ax::Token t = lexer.get_token();

    std::cout << t << std::endl;
    return 0;
}