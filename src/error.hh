//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <string>

#include "location.hh"

namespace ax {

class AXException : std::exception {

  public:
    AXException(std::string m, Location const &l)
        : msg(std::move(m)), location(l){};

    std::string error_msg();

    std::string msg;
    Location         location;
};

class LexicalException : public AXException {
  public:
    LexicalException(std::string const &m, Location const &l)
        : AXException(m, l){};
};

class ParseException : public AXException {
  public:
    ParseException(std::string const &m, Location const &l)
        : AXException(m, l){};
};

class TypeError : public AXException {
  public:
    TypeError(std::string const &m, Location const &l) : AXException(m, l){};
};

class CodeGenException : public AXException {
  public:
   explicit CodeGenException(std::string const &m)
        : AXException(m, Location{}) {};
    CodeGenException(std::string const &m, Location const &l)
        : AXException(m, l){};
};

} // namespace ax