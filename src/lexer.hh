//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <cctype>
#include <iostream>
#include <stack>
#include <string>
#include <unordered_map>

#include <llvm/Support/FormatVariadic.h>

#include "ax.hh"
#include "error.hh"
#include "location.hh"
#include "token.hh"

namespace ax {

template <typename C> class CharacterClass {
  public:
    virtual ~CharacterClass() = default;
    static bool        isspace(C);
    static bool        isxdigit(C);
    static bool        isdigit(C);
    static bool        isalnum(C);
    static bool        isalpha(C);
    static std::string to_string(C);
};

template <typename C, class CharClass> class LexerInterface {
  public:
    LexerInterface(std::istream &stream, ErrorManager const &e) : is{stream}, errors{e} {};
    virtual ~LexerInterface() = default;

    virtual Token get_token();
    virtual void  push_token(Token const &t) { next_token.push(t); }
    virtual Token peek_token();

    [[nodiscard]] Location get_location() const { return Location{lineno, charpos}; }

  protected:
    void set_newline() {
        lineno++;
        charpos = 0;
    }

    virtual C get() = 0;
    virtual C peek() = 0;
    virtual C get_char(); // get next non whitespace or comment char

    void get_comment();

    int charpos{0};

    std::istream &is;

  private:
    Token scan_digit(C c);
    Token scan_ident(C c);

    ErrorManager const &errors;
    std::stack<Token>   next_token;

    int lineno{1};
};

class Character8 : CharacterClass<char> {
  public:
    ~Character8() override = default;
    static bool        isspace(char c) { return std::isspace(c); };
    static bool        isxdigit(char c) { return std::isxdigit(c); };
    static bool        isdigit(char c) { return std::isdigit(c); };
    static bool        isalnum(char c) { return std::isalnum(c); };
    static bool        isalpha(char c) { return std::isalpha(c); };
    static std::string to_string(char c) { return std::string(1, c); }
};

class Lexer : public LexerInterface<char, Character8> {
  public:
    Lexer(std::istream &stream, ErrorManager const &e) : LexerInterface{stream, e} {};
    ~Lexer() override = default;

  private:
    char get() override {
        charpos++;
        return is.get();
    };
    char peek() override { return is.peek(); };
};

/////////////////////////////////////////////////////////////////////////////////////////////

extern const std::unordered_map<std::string, Token> keyword_map;
extern const std::unordered_map<char, Token>        token_map;

inline constexpr bool debug_lexer{false};

template <typename... T> inline void debugl(const T &... msg) {
    if constexpr (debug_lexer) {
        std::cerr << std::string(llvm::formatv(msg...)) << std::endl;
    }
}

template <typename C, class CharClass> Token LexerInterface<C, CharClass>::peek_token() {
    if (next_token.empty()) {
        Token t{get_token()};
        next_token.push(t);
        return t;
    } // namespace ax
    return next_token.top();
}

template <typename C, class CharClass> Token LexerInterface<C, CharClass>::get_token() {
    // Check if there is already a token
    if (!next_token.empty()) {
        debugl("size: {}", next_token.size());
        Token s{next_token.top()};
        next_token.pop();
        return s;
    }

    // Get next token
    auto c = get_char();

    // Check single digit character tokens
    if (auto res = token_map.find(c); res != token_map.end()) {
        return res->second;
    }

    // Check multiple character tokens
    switch (c) {
    case ':':
        if (peek() == '=') {
            get_char();
            return Token(TokenType::assign, ":=");
        }
        return Token(TokenType::colon, ":");
    case '<':
        if (peek() == '=') {
            get_char();
            return Token(TokenType::leq, "<=");
        }
        return Token(TokenType::less, "<");
    case '>':
        if (peek() == '=') {
            get_char();
            return Token(TokenType::gteq, ">=");
        }
        return Token(TokenType::greater, ">");
    default:;
    }
    if (CharClass::isdigit(c)) {
        return scan_digit(c);
    }
    if (CharClass::isalpha(c)) {
        return scan_ident(c);
    }
    std::cout << "character: " << int(c) << std::endl;
    throw LexicalException("Unknown character " + CharClass::to_string(c), get_location());
}

template <typename C, class CharClass> void LexerInterface<C, CharClass>::get_comment() {
    get(); // get asterisk
    do {
        auto c = get();
        if (c == '*' && peek() == ')') {
            get();
            return;
        }
        if (c == '(' && peek() == '*') {
            // suport nested comments, call
            // recursively
            this->get_comment();
        }
        if (c == '\n') {
            set_newline();
        }
    } while (is);
}

template <typename C, class CharClass> Token LexerInterface<C, CharClass>::scan_digit(C c) {
    std::string digit(1, c);
    c = peek();
    while (CharClass::isxdigit(c)) {
        get();
        digit += c;
        c = peek();
    }
    return Token(TokenType::integer, digit);
}

template <typename C, class CharClass> Token LexerInterface<C, CharClass>::scan_ident(C c) {
    std::string ident(1, c);
    c = peek();
    while (CharClass::isalnum(c) || c == '_') {
        get();
        ident += c;
        c = peek();
    }
    // Look for keywords
    if (auto res = keyword_map.find(ident); res != keyword_map.end()) {
        return res->second;
    }
    return Token(TokenType::ident, ident);
}

/**
 * @brief get first non whitespace or comment character
 *
 * @return char
 */
template <typename C, class CharClass> C LexerInterface<C, CharClass>::get_char() {
    while (is) {
        auto c = get();
        if (c == '\n') {
            set_newline();
            continue;
        }
        if (c == '(' && peek() == '*') {
            get_comment();
            continue;
        }
        if (CharClass::isspace(c)) {
            continue;
        }
        return c;
    }
    return -1;
}

} // namespace ax