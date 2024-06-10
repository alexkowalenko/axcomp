//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <format>
#include <string>
#include <vector>

#include "location.hh"

namespace ax {

class AXException : std::exception {

  public:
    AXException(Location const &l, std::string m) : location(l), msg(std::move(m)) {};
    ~AXException() override = default;

    [[nodiscard]] std::string error_msg() const;

    Location    location;
    std::string msg;

  protected:
    AXException() = default;
};

class LexicalException : public AXException {
  public:
    LexicalException(Location const &l, std::string m) : AXException(l, m) {};

    template <typename... Args>
    LexicalException(Location const &l, std::string fmt, const Args &...args) {
        location = l;
        msg = std::vformat(fmt, std::make_format_args(args...));
    };
};

class ParseException : public AXException {
  public:
    ParseException(Location const &l, std::string const &m) : AXException(l, m) {};

    template <typename... Args>
    ParseException(Location const &l, std::string fmt, const Args &...args) {
        location = l;
        msg = std::vformat(fmt, std::make_format_args(args...));
    };
};

class TypeError : public AXException {
  public:
    TypeError(Location const &l, std::string const &m) : AXException(l, m) {};

    template <typename... Args>
    TypeError(Location const &l, std::string fmt, const Args &...args) {
        location = l;
        msg = std::vformat(fmt, std::make_format_args(args...));
    };
};

class CodeGenException : public AXException {
  public:
    explicit CodeGenException(std::string const &m) : AXException(Location{}, m) {};

    template <typename... Args> CodeGenException(std::string fmt, const Args &...args) {
        msg = std::vformat(fmt, std::make_format_args(args...));
    };

    CodeGenException(Location const &l, std::string const &m) : AXException(l, m) {};

    template <typename... Args>
    CodeGenException(Location const &l, std::string fmt, const Args &...args) {
        location = l;
        msg = std::vformat(fmt, std::make_format_args(args...));
    };
};

class ErrorManager {
  public:
    void               add(AXException const &e) { error_list.push_back(e); };
    [[nodiscard]] bool has_errors() const { return !error_list.empty(); };

    void print_errors(std::ostream &out);
    auto first() { return error_list.begin(); };

  private:
    std::vector<AXException> error_list;
};

} // namespace ax