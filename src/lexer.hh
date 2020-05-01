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
    static bool isspace(C);
    static bool isxdigit(C);
    static bool isdigit(C);
    static bool isalnum(C);
    static bool isalpha(C);

    static std::string to_string(C);
    static void        add_string(std::string const &s, Char c);
};

class LexerInterface {
  public:
    virtual ~LexerInterface() = default;

    virtual Token get_token() = 0;
    virtual void  push_token(Token const &t) = 0;
    virtual Token peek_token() = 0;

    [[nodiscard]] virtual Location get_location() const = 0;
};

template <typename C, class CharClass> class LexerImplementation : public LexerInterface {
  public:
    LexerImplementation(std::istream &stream, ErrorManager const &e) : is{stream}, errors{e} {};
    ~LexerImplementation() override = default;

    Token get_token() override;
    void  push_token(Token const &t) override { next_token.push(t); }
    Token peek_token() override;

    [[nodiscard]] Location get_location() const override { return Location{lineno, charpos}; }

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
    Token scan_string(C c);

    ErrorManager const &errors;
    std::stack<Token>   next_token;

    int lineno{1};
};

class Character8 : CharacterClass<char> {
  public:
    ~Character8() override = default;
    static bool isspace(char c) { return std::isspace(c); };
    static bool isxdigit(char c) { return std::isxdigit(c); };
    static bool isdigit(char c) { return std::isdigit(c); };
    static bool isalnum(char c) { return std::isalnum(c); };
    static bool isalpha(char c) { return std::isalpha(c); };

    static std::string to_string(char c) { return std::string(1, c); }
    static void        add_string(std::string &s, char c) { s.push_back(c); }
};

class Lexer : public LexerImplementation<char, Character8> {
  public:
    Lexer(std::istream &stream, ErrorManager const &e) : LexerImplementation{stream, e} {};
    ~Lexer() = default;

  private:
    char get() override {
        charpos++;
        return is.get();
    };
    char peek() override { return is.peek(); };
};

/////////////////////////////////////////////////////////////////////////////////////////////

extern const std::unordered_map<std::string, Token> keyword_map;
extern const std::unordered_map<char, Token>        single_tokens;

template <typename C, class CharClass> Token LexerImplementation<C, CharClass>::peek_token() {
    if (next_token.empty()) {
        Token t{get_token()};
        next_token.push(t);
        return t;
    } // namespace ax
    return next_token.top();
}

template <typename C, class CharClass> Token LexerImplementation<C, CharClass>::get_token() {
    // Check if there is already a token
    if (!next_token.empty()) {
        Token s{next_token.top()};
        next_token.pop();
        return s;
    }

    // Get next token
    C c = get_char();

    // Check single digit character tokens
    if (auto res = single_tokens.find(c); res != single_tokens.end()) {
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
    case '.':
        if (peek() == '.') {
            get_char();
            return Token(TokenType::dotdot, "..");
        }
        return Token(TokenType::period, ".");
    case '\'':
    case '\"':
        return scan_string(c);
    default:;
    }
    if (CharClass::isdigit(c)) {
        return scan_digit(c);
    }
    if (CharClass::isalnum(c)) {
        return scan_ident(c);
    }
    std::cout << "character: " << int(c) << std::endl;
    throw LexicalException("Unknown character " + CharClass::to_string(c), get_location());
}

template <typename C, class CharClass> void LexerImplementation<C, CharClass>::get_comment() {
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

template <typename C, class CharClass> Token LexerImplementation<C, CharClass>::scan_digit(C c) {
    std::string digit(1, c);
    c = peek();
    while (CharClass::isxdigit(c)) {
        get();
        digit += c;
        c = peek();
    }
    if (c == 'H') {
        // Numbers in this format 0cafeH
        get();
        return Token(TokenType::hexinteger, digit);
    }
    if (c == 'X') {
        // Characters in this format 0d34X
        get();
        return Token(TokenType::hexchr, digit);
    };
    return Token(TokenType::integer, digit);
} // namespace ax

template <typename C, class CharClass> Token LexerImplementation<C, CharClass>::scan_ident(C c) {
    std::string ident;
    CharClass::add_string(ident, c);

    c = peek();
    while (CharClass::isalnum(c) || c == '_') {
        get();
        CharClass::add_string(ident, c);
        c = peek();
    }
    // Look for keywords
    if (auto res = keyword_map.find(ident); res != keyword_map.end()) {
        return res->second;
    }
    return Token(TokenType::ident, ident);
}

template <typename C, class CharClass>
Token LexerImplementation<C, CharClass>::scan_string(C start) {
    // Already scanned ' or "
    std::string str;
    CharClass::add_string(str, start);
    C c = get();
    if (c == start) {
        // empty string
        return Token(TokenType::string, str);
    }
    CharClass::add_string(str, c);
    C final = get();
    if (final == '\'' && start == '\'') {
        return Token(TokenType::chr, long(c));
    };
    if (final == start) {
        return Token(TokenType::string, str);
    };
    CharClass::add_string(str, final);
    c = get();
    while (c != start) {
        if (c == '\n') {
            // End of line reached - error
            throw LexicalException("Unterminated string", get_location());
        }
        CharClass::add_string(str, c);
        if (str.length() > MAX_STR_LITTERAL) {
            throw LexicalException(
                llvm::formatv("String literal greater than {0}", MAX_STR_LITTERAL),
                get_location());
        }
        c = get();
    };
    return Token(TokenType::string, str);
} // namespace ax

/**
 * @brief get first non whitespace or comment character
 *
 * @return char
 */
template <typename C, class CharClass> C LexerImplementation<C, CharClass>::get_char() {
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