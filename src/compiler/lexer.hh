//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include <cctype>
#include <concepts>
#include <format>
#include <iostream>
#include <map>
#include <stack>
#include <string>

#include "ax.hh"
#include "error.hh"
#include "location.hh"
#include "token.hh"

namespace ax {

template <std::signed_integral C> class CharacterClass {
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

template <std::signed_integral C, class CharClass>
class LexerImplementation : public LexerInterface {
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

    void push(C c) { last_char = c; }
    C    last_char{0};

  private:
    Token scan_digit(C c);
    Token scan_ident(C c);
    Token scan_string(C start);

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

    static std::string to_string(char c) { return {1, c}; }
    static void        add_string(std::string &s, char c) { s.push_back(c); }
};

class Lexer : public LexerImplementation<char, Character8> {
  public:
    Lexer(std::istream &stream, ErrorManager const &e) : LexerImplementation{stream, e} {};
    ~Lexer() override = default;

  private:
    char get() override {
        if (last_char != 0) {
            auto tmp = last_char;
            last_char = 0;
            return tmp;
        }
        charpos++;
        return char(is.get());
    };
    char peek() override { return char(is.peek()); };
};

/////////////////////////////////////////////////////////////////////////////////////////////

inline const std::map<std::string, Token> keyword_map = {
    {"MODULE", Token(TokenType::module, "MODULE")},
    {"BEGIN", Token(TokenType::begin, "BEGIN")},
    {"END", Token(TokenType::end, "END")},
    {"DIV", Token(TokenType::div, "DIV")},
    {"MOD", Token(TokenType::mod, "MOD")},
    {"CONST", Token(TokenType::cnst, "CONST")},
    {"TYPE", Token(TokenType::type, "TYPE")},
    {"VAR", Token(TokenType::var, "VAR")},
    {"RETURN", Token(TokenType::ret, "RETURN")},
    {"PROCEDURE", Token(TokenType::procedure, "PROCEDURE")},
    {"TRUE", Token(TokenType::true_k, "TRUE")},
    {"FALSE", Token(TokenType::false_k, "FALSE")},
    {"OR", Token(TokenType::or_k, "OR")},
    {"IF", Token(TokenType::if_k, "IF")},
    {"THEN", Token(TokenType::then, "THEN")},
    {"ELSIF", Token(TokenType::elsif, "ELSIF")},
    {"ELSE", Token(TokenType::else_k, "ELSE")},
    {"FOR", Token(TokenType::for_k, "FOR")},
    {"TO", Token(TokenType::to, "TO")},
    {"BY", Token(TokenType::by, "BY")},
    {"DO", Token(TokenType::do_k, "DO")},
    {"WHILE", Token(TokenType::while_k, "WHILE")},
    {"REPEAT", Token(TokenType::repeat, "REPEAT")},
    {"UNTIL", Token(TokenType::until, "UNTIL")},
    {"LOOP", Token(TokenType::loop, "LOOP")},
    {"EXIT", Token(TokenType::exit, "EXIT")},
    {"ARRAY", Token(TokenType::array, "ARRAY")},
    {"OF", Token(TokenType::of, "OF")},
    {"RECORD", Token(TokenType::record, "RECORD")},
    {"DEFINITION", Token(TokenType::definition, "DEFINITION")},
    {"IMPORT", Token(TokenType::import, "IMPORT")},
    {"CASE", Token(TokenType::cse, "CASE")},
    {"POINTER", Token(TokenType::pointer, "POINTER")},
    {"NIL", Token(TokenType::nil, "NIL")},
    {"IN", Token(TokenType::in, "IN")},
};

inline const std::map<char, Token> single_tokens = {
    {-1, Token(TokenType::eof)},
    {';', Token(TokenType::semicolon, ";")},
    {',', Token(TokenType::comma, ",")},
    {'+', Token(TokenType::plus, "+")},
    {'-', Token(TokenType::dash, "-")},
    {'*', Token(TokenType::asterisk, "*")},
    {'(', Token(TokenType::l_paren, "(")},
    {')', Token(TokenType::r_paren, ")")},
    {'=', Token(TokenType::equals, "=")},
    {'#', Token(TokenType::hash, "#")},
    {'~', Token(TokenType::tilde, "~")},
    {'&', Token(TokenType::ampersand, "&")},
    {'[', Token(TokenType::l_bracket, "[")},
    {']', Token(TokenType::r_bracket, "]")},
    {'|', Token(TokenType::bar, "|")},
    {'/', Token(TokenType::slash, "/")},
    {'^', Token(TokenType::caret, "^")},
    {'{', Token(TokenType::l_brace, "{")},
    {'}', Token(TokenType::r_brace, "}")},
};

template <std::signed_integral C, class CharClass>
Token LexerImplementation<C, CharClass>::peek_token() {
    if (next_token.empty()) {
        Token t{get_token()};
        next_token.push(t);
        return t;
    }
    return next_token.top();
}

template <std::signed_integral C, class CharClass>
Token LexerImplementation<C, CharClass>::get_token() {
    // Check if there is already a token
    if (!next_token.empty()) {
        Token s{next_token.top()};
        next_token.pop();
        return s;
    }

    // Get next token
    C c = get_char();

    // Check single digit character tokens
    if (auto res = single_tokens.find(char(c)); res != single_tokens.end()) {
        return res->second;
    }

    // Check multiple character tokens
    switch (c) {
    case ':':
        if (peek() == '=') {
            get_char();
            return {TokenType::assign, ":="};
        }
        return {TokenType::colon, ":"};
    case '<':
        if (peek() == '=') {
            get_char();
            return {TokenType::leq, "<="};
        }
        return {TokenType::less, "<"};
    case '>':
        if (peek() == '=') {
            get_char();
            return {TokenType::gteq, ">="};
        }
        return {TokenType::greater, ">"};
    case '.':
        if (peek() == '.') {
            get_char();
            return {TokenType::dotdot, ".."};
        }
        return {TokenType::period, "."};
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
    throw LexicalException(get_location(), "Unknown character " + CharClass::to_string(c));
}

template <std::signed_integral C, class CharClass>
void LexerImplementation<C, CharClass>::get_comment() {
    get(); // get asterisk
    do {
        auto c = get();
        if (c == '*' && peek() == ')') {
            get();
            return;
        }
        if (c == '(' && peek() == '*') {
            // support nested comments, call
            // recursively
            this->get_comment();
        }
        if (c == '\n') {
            set_newline();
        }
    } while (is);
}

template <std::signed_integral C, class CharClass>
Token LexerImplementation<C, CharClass>::scan_digit(C c) {
    // We are assuming that all characters for digits fit in the type char, i.e. they are normal
    // western digits.
    std::string digit(1, char(c));
    c = peek();
    while (CharClass::isxdigit(c)) {
        get();
        digit += char(c);
        c = peek();
    }
    if (c == 'H') {
        // Numbers in this format 0cafeH
        get();
        return {TokenType::hexinteger, digit};
    }
    if (c == '.') {
        // may be float
        get();

        c = peek();
        // has to follow by a digit to be a real, else int.
        if (!CharClass::isdigit(c)) {
            // put back '.'
            push('.');
            // is integer
            return {TokenType::integer, digit};
        }
        digit += '.';
        while (CharClass::isdigit(c)) {
            get();
            digit += char(c);
            c = peek();
        }
        if (c == 'D' || c == 'E') {
            get();
            digit += char(c);
            c = peek();
        }
        if (c == '+' || c == '-') {
            get();
            digit += char(c);
            c = peek();
        }
        while (CharClass::isdigit(c)) {
            get();
            digit += char(c);
            c = peek();
        }
        return {TokenType::real, digit};
    }
    if (c == 'X') {
        // Characters in this format 0d34X
        get();
        return {TokenType::hexchr, digit};
    };
    return {TokenType::integer, digit};
}

template <std::signed_integral C, class CharClass>
Token LexerImplementation<C, CharClass>::scan_ident(C c) {
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
    return {TokenType::ident, ident};
}

template <std::signed_integral C, class CharClass>
Token LexerImplementation<C, CharClass>::scan_string(C start) {
    // Already scanned ' or "
    std::string str;
    CharClass::add_string(str, start);
    C c = get();
    if (c == start) {
        // empty string
        return {TokenType::string, str};
    }
    CharClass::add_string(str, c);
    C final = get();
    if (final == '\'' && start == '\'') {
        return {TokenType::chr, long(c)};
    };
    if (final == start) {
        return {TokenType::string, str};
    };
    CharClass::add_string(str, final);
    c = get();
    while (c != start) {
        if (c == '\n') {
            // End of line reached - error
            throw LexicalException(get_location(), "Unterminated string");
        }
        CharClass::add_string(str, c);
        if (str.length() > MAX_STR_LITERAL) {
            throw LexicalException(get_location(), "String literal greater than {0}",
                                   MAX_STR_LITERAL);
        }
        c = get();
    };
    return {TokenType::string, str};
}

/**
 * @brief get first non whitespace or comment character
 *
 * @return char
 */
template <std::signed_integral C, class CharClass>
C LexerImplementation<C, CharClass>::get_char() {
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