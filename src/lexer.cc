//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include "lexer.hh"

namespace ax {

Token Lexer::get_token() {
    return Token(TokenType::eof);
}

} // namespace ax