//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "parser.hh"

namespace ax {

ASTBase Parser::parse() {
    return ASTProg();
};

}; // namespace ax