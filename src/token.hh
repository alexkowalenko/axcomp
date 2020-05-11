//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <iostream>
#include <string>

namespace ax {

enum class TokenType {
    null = 0,
    eof,
    ident,
    semicolon,
    period,
    comma,
    plus,
    dash,
    asterisk,
    l_paren,
    r_paren,
    equals,
    colon,
    assign,
    hash,
    less,
    leq,
    greater,
    gteq,
    tilde,
    ampersand,
    l_bracket,
    r_bracket,
    dotdot,
    bar,
    slash,
    caret,

    integer,
    hexinteger,
    real,
    chr,
    hexchr,
    string,

    // Keywords
    module,
    begin,
    end,
    div,
    mod,
    cnst,
    type,
    var,
    ret,
    procedure,
    true_k,
    false_k,
    or_k,
    if_k,
    then,
    elsif,
    else_k,
    for_k,
    to,
    by,
    do_k,
    while_k,
    repeat,
    until,
    loop,
    exit,
    array,
    of,
    record,
    definition,
    import,
    cse,
    pointer,
    nil
};

std::string string(TokenType t);

class Token {
  public:
    explicit Token(TokenType t) noexcept : type(t){};
    Token(TokenType t, std::string v) : type(t), val(std::move(v)){};
    Token(TokenType t, long v) : type(t), val_int(v){};

    Token(Token const &) = default;
    Token &operator=(Token const &) = default;

    explicit             operator std::string();
    friend std::ostream &operator<<(std::ostream &os, const Token &t);

    TokenType   type;
    std::string val;
    long        val_int = 0;
};

std::ostream &operator<<(std::ostream &os, const Token &t);

} // namespace ax
