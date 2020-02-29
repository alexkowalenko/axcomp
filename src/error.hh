//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <string>

namespace ax {

class AXException : std::exception {

  public:
    AXException(std::string m, int l) : msg(std::move(m)), lineno(l){};

    std::string error_msg();

    std::string msg;
    int         lineno = 0;
};

class LexicalException : public AXException {
  public:
    LexicalException(std::string const &m, int l) : AXException(m, l){};
};

class ParseException : public AXException {
  public:
    ParseException(std::string const &m, int l) : AXException(m, l){};
};

class TypeError : public AXException {
  public:
    TypeError(std::string const &m, int l) : AXException(m, l){};
};

class CodeGenException : public AXException {
  public:
    CodeGenException(std::string const &m, int l) : AXException(m, l){};
    explicit CodeGenException(std::string const &m) : AXException(m, 0){};
};

} // namespace ax