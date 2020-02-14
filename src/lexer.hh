//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <iostream>
#include <string>

#include "token.hh"

namespace ax {

class Lexer {
  public:
    Lexer(std::istream const &stream) : is(stream){};

    Token get_token();

  private:
    std::istream const &is;
};
} // namespace ax