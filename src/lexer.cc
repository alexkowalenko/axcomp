//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <unordered_map>

#include <llvm/Support/FormatVariadic.h>

#include "error.hh"
#include "lexer.hh"

namespace ax {

inline constexpr bool debug_lexer{false};

template <typename... T> inline void debug(const T &... msg) {
    if constexpr (debug_lexer) {
        std::cerr << llvm::formatv(msg...) << std::endl;
    }
}

static const Token nullToken = Token(TokenType::null);

static const std::unordered_map<std::string, Token> keyword_map = {
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
};

static const std::unordered_map<char, Token> token_map = {
    {-1, Token(TokenType::eof)},
    {';', Token(TokenType::semicolon, ";")},
    {'.', Token(TokenType::period, ".")},
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
};

Lexer::Lexer(std::istream &stream, ErrorManager const &e) : is{stream}, errors{e} {}

void Lexer::get_comment() {
    get(); // get asterisk
    do {
        char c = get();
        if (c == '*' && is.peek() == ')') {
            is.get();
            return;
        }
        if (c == '(' && is.peek() == '*') {
            // suport nested comments, call
            // recursively
            get_comment();
        }
        if (c == '\n') {
            set_newline();
        }
    } while (is);
}

char Lexer::get_char() {
    while (is) {
        char c = get();
        if (c == '\n') {
            set_newline();
            continue;
        }
        if (c == '(' && is.peek() == '*') {
            get_comment();
            continue;
        }
        if (std::isspace(c)) {
            continue;
        }
        return c;
    }
    return -1;
}

Token Lexer::scan_digit(char c) {
    std::string digit(1, c);
    c = is.peek();
    while (std::isxdigit(c)) {
        get();
        digit += c;
        c = is.peek();
    }
    return Token(TokenType::integer, digit);
}

Token Lexer::scan_ident(char c) {
    std::string ident(1, c);
    c = is.peek();
    while (std::isalnum(c) || c == '_') {
        get();
        ident += c;
        c = is.peek();
    }
    // Look for keywords
    if (auto res = keyword_map.find(ident); res != keyword_map.end()) {
        return res->second;
    }
    return Token(TokenType::ident, ident);
}

Token Lexer::get_token() {
    // Check if there is already a token
    if (!next_token.empty()) {
        debug("size: {}", next_token.size());
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
        if (is.peek() == '=') {
            get_char();
            return Token(TokenType::assign, ":=");
        }
        return Token(TokenType::colon, ":");
    case '<':
        if (is.peek() == '=') {
            get_char();
            return Token(TokenType::leq, "<=");
        }
        return Token(TokenType::less, "<");
    case '>':
        if (is.peek() == '=') {
            get_char();
            return Token(TokenType::gteq, ">=");
        }
        return Token(TokenType::greater, ">");
    default:;
    }
    if (std::isdigit(c)) {
        return scan_digit(c);
    }
    if (std::isalpha(c)) {
        return scan_ident(c);
    }
    throw LexicalException(std::string("Unknown character ") + c, get_location());
}

void Lexer::push_token(Token const &t) {
    next_token.push(t);
}

Token Lexer::peek_token() {
    if (next_token.empty()) {
        Token t{get_token()};
        next_token.push(t);
        return t;
    }
    return next_token.top();
}

} // namespace ax