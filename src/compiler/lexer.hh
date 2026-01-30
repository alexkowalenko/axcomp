//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <stack>
#include <string>

#include "ax.hh"
#include "error.hh"
#include "location.hh"
#include "token.hh"

namespace ax {

struct Options;

class LexerUTF8 {
  public:
    LexerUTF8(std::string text, ErrorManager const &e, Options *options = nullptr);

    ~LexerUTF8() = default;

    [[nodiscard]] Location get_location() const { return {line_no, char_pos}; }

    Token peek_token();
    void  push_token(Token const &t) { next_token.push(t); }
    Token get_token();

  private:
    Char get();
    Char peek() const;
    Char get_char(); // get the next non-whitespace or comment char
    void push_char(Char const c) { last_char = c; }

    void set_newline() {
        line_no++;
        char_pos = 0;
    }

    void get_comment();
    void get_line_comment();
    void scan_directive();

    Token scan_digit(Char c);
    Token scan_ident(Char c);
    Token scan_string(Char start);

    std::string                 buffer;
    std::string::const_iterator cursor;
    ErrorManager const         &errors;
    std::stack<Token>           next_token;
    Options                    *options{nullptr};

    int line_no{1};
    int char_pos{0};

    Char last_char{0};
};

} // namespace ax
