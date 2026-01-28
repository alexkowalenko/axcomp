//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <iostream>
#include <string>

namespace ax {

enum class TokenType : std::uint8_t {
    None = 0,
    Eof,
    IDENT,
    SEMICOLON,
    PERIOD,
    COMMA,
    PLUS,
    DASH,
    ASTÉRIX,
    L_PAREN,
    R_PAREN,
    EQUALS,
    COLON,
    ASSIGN,
    HASH,
    LESS,
    LEQ,
    GREATER,
    GTEQ,
    TILDE,
    AMPERSAND,
    L_BRACKET,
    R_BRACKET,
    DOTDOT,
    BAR,
    SLASH,
    CARET,
    L_BRACE,
    R_BRACE,

    INTEGER,
    HEXINTEGER,
    REAL,
    CHR,
    HEXCHR,
    STRING,

    // Keywords
    MODULE,
    BEGIN,
    END,
    DIV,
    MOD,
    CONST,
    TYPE,
    VAR,
    RETURN,
    PROCEDURE,
    TRUE,
    FALSE,
    OR,
    IF,
    THEN,
    ELSIF,
    ELSE,
    FOR,
    TO,
    BY,
    DO,
    WHILE,
    REPEAT,
    UNTIL,
    LOOP,
    EXIT,
    ARRAY,
    OF,
    RECORD,
    DEFINITION,
    IMPORT,
    CASE,
    POINTER,
    NIL,
    IN
};

std::string string(TokenType t);

class Token {
  public:
    explicit Token(const TokenType t) noexcept : type(t) {};
    Token(const TokenType t, std::string v) : type(t), val(std::move(v)) {};
    Token(const TokenType t, const long v) : type(t), val_int(v) {};

    Token(Token const &) = default;
    Token &operator=(Token const &) = default;

    explicit             operator std::string() const;
    friend std::ostream &operator<<(std::ostream &os, const Token &t);

    TokenType   type;
    std::string val;
    long        val_int = 0;
};

std::ostream &operator<<(std::ostream &os, const Token &t);

} // namespace ax
