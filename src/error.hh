//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <string>

namespace ax {

class AXException {

  public:
    AXException(std::string m, int l) : msg(m), lineno(l){};

    std::string error_msg();

    std::string msg;
    int         lineno = 0;
};

class LexicalException : public AXException {
  public:
    LexicalException(std::string m, int l) : AXException(m, l){};
};

class ParseException : public AXException {
  public:
    ParseException(std::string m, int l) : AXException(m, l){};
};

} // namespace ax