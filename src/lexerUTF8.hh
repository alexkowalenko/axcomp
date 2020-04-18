//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include "lexer.hh"

namespace ax {

class Character32 : CharacterClass<Char> {
  public:
    ~Character32() override = default;
    static bool        isspace(Char c) { return std::iswspace(c); };
    static bool        isxdigit(Char c) { return std::iswxdigit(c); };
    static bool        isdigit(Char c) { return std::iswdigit(c); };
    static bool        isalnum(Char c) { return std::iswalnum(c); };
    static bool        isalpha(Char c) { return std::iswalpha(c); };
    static std::string to_string(Char c) { return std::string(1, c); }
};

class LexerUTF8 : public LexerInterface<Char, Character32> {
  public:
    LexerUTF8(std::istream &stream, ErrorManager const &e) : LexerInterface{stream, e} {
        ptr = buf.end();
    };
    ~LexerUTF8() override = default;

  private:
    Char get() override;
    Char peek() override;

    void get_line();

    std::string           buf;
    std::string::iterator ptr;
};

} // namespace ax