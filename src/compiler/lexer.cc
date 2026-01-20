//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <exception>
#include <map>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wold-style-cast"
#include <unicode/uchar.h>
#include <utf8.h>
#pragma clang diagnostic pop

#include "error.hh"
#include "lexer.hh"

namespace ax {

namespace {

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

const std::map<char, Token> single_tokens = {
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

// Custom emoji checker
// This is a hack.
constexpr bool is_emoji(const Char c) {
    return (0x1f600 <= c && c <= 0x1f64f) || // Emoticons
           (0x1F300 <= c && c <= 0x1F5FF) || // Misc Symbols and Pictographs
           (0x1F680 <= c && c <= 0x1F6FF) || // Transport and Map
           (0x1F1E6 <= c && c <= 0x1F1FF) || // Regional country flags
           (0x2600 <= c && c <= 0x26FF) ||   // Misc symbols
           (0x2700 <= c && c <= 0x27BF) ||   // Dingbats
           (0xE0020 <= c && c <= 0xE007F) || // Tags
           (0xFE00 <= c && c <= 0xFE0F) ||   // Variation Selectors
           (0x1F900 <= c && c <= 0x1F9FF) || // Supplemental Symbols and Pictographs
           (0x1F018 <= c && c <= 0x1F270) || // Various asian characters
           (0x238C <= c && c <= 0x2454) ||   // Misc items
           (0x20D0 <= c && c <= 0x20FF);     //
}

} // namespace

std::string wcharToString(const wchar_t wchar);

Token LexerUTF8::peek_token() {
    if (next_token.empty()) {
        Token t{get_token()};
        next_token.push(t);
        return t;
    }
    return next_token.top();
}

Token LexerUTF8::get_token() {
    // Check if there is already a token
    if (!next_token.empty()) {
        Token s{next_token.top()};
        next_token.pop();
        return s;
    }

    // Get the next token
    const Char c = get_char();

    // Check single-digit character tokens
    if (const auto res = single_tokens.find(static_cast<char>(c)); res != single_tokens.end()) {
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
    if (std::iswdigit(c)) {
        return scan_digit(c);
    }
    if (u_isalnum(c) || is_emoji(c)) {
        return scan_ident(c);
    }
    // std::cout << "character: " << static_cast<int>(c) << std::endl;
    throw LexicalException(get_location(), "Unknown character " + wcharToString(c));
}

void LexerUTF8::get_comment() {
    get(); // get asterisk
    do {
        auto const c = get();
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

Token LexerUTF8::scan_digit(Char c) {
    // We are assuming that all characters for digits fit in the type char, i.e. they are normal
    // western digits.
    std::string digit(1, static_cast<char>(c));
    c = peek();
    while (std::iswxdigit(c)) {
        get();
        digit += static_cast<char>(c);
        c = peek();
    }
    if (c == 'H') {
        // Numbers in this format 0cafeH
        get();
        return {TokenType::hexinteger, digit};
    }
    if (c == '.') {
        // maybe float
        get();

        c = peek();
        // has to follow by a digit to be a real, else int.
        if (!std::iswdigit(c)) {
            // put back '.'
            push_char('.');
            // is integer
            return {TokenType::integer, digit};
        }
        digit += '.';
        while (std::iswdigit(c)) {
            get();
            digit += static_cast<char>(c);
            c = peek();
        }
        if (c == 'D' || c == 'E') {
            get();
            digit += static_cast<char>(c);
            c = peek();
        }
        if (c == '+' || c == '-') {
            get();
            digit += static_cast<char>(c);
            c = peek();
        }
        while (std::iswdigit(c)) {
            get();
            digit += static_cast<char>(c);
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

Token LexerUTF8::scan_ident(Char c) {
    std::string ident;
    utf8::append(static_cast<char32_t>(c), ident);

    c = peek();
    while (u_isalnum(c) || is_emoji(c) || c == '_') {
        get();
        utf8::append(static_cast<char32_t>(c), ident);
        c = peek();
    }
    // Look for keywords
    if (const auto res = keyword_map.find(ident); res != keyword_map.end()) {
        return res->second;
    }
    return {TokenType::ident, ident};
}

Token LexerUTF8::scan_string(Char const start) {
    // Already scanned ' or "
    std::string str;
    utf8::append(static_cast<char32_t>(start), str);
    Char c = get();
    if (c == start) {
        // empty string
        return {TokenType::string, str};
    }
    utf8::append(static_cast<char32_t>(c), str);
    const Char final = get();
    if (final == '\'' && start == '\'') {
        return {TokenType::chr, static_cast<long>(c)};
    };
    if (final == start) {
        return {TokenType::string, str};
    };
    utf8::append(static_cast<char32_t>(final), str);
    c = get();
    while (c != start) {
        if (c == '\n') {
            // End of line reached - error
            throw LexicalException(get_location(), "Unterminated string");
        }
        utf8::append(static_cast<char32_t>(c), str);
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
Char LexerUTF8::get_char() {
    while (is) {
        auto const c = get();
        if (c == '\n') {
            set_newline();
            continue;
        }
        if (c == '(' && peek() == '*') {
            get_comment();
            continue;
        }
        if (std::iswspace(c)) {
            continue;
        }
        return c;
    }
    return -1;
}

class EOFException : std::exception {};

Char LexerUTF8::get() {
    Char c = 0;
    if (last_char != 0) {
        std::swap(c, last_char);
        return c;
    }
    try {
        while (line_ptr == current_line.end()) {
            get_line();
        }
        c = utf8::next(line_ptr, current_line.end());
    } catch (EOFException &) {
        c = -1;
        return c;
    }
    char_pos++;
    return c;
}

Char LexerUTF8::peek() {
    return static_cast<Char>(utf8::peek_next(line_ptr, current_line.end()));
}

void LexerUTF8::get_line() {
    if (!getline(is, current_line)) {
        throw EOFException{};
    }
    current_line.push_back('\n');
    // check UTF-8 correctness
    if (!utf8::is_valid(current_line.begin(), current_line.end())) {
        throw LexicalException(get_location(), "Not valid UTF-8 text");
    }
    line_ptr = current_line.begin();
};

std::string wcharToString(const wchar_t wchar) {
    auto                  state = std::mbstate_t();
    std::array<char, 120> buffer;
    const size_t          length = std::wcrtomb(buffer.data(), wchar, &state);

    if (length == static_cast<size_t>(-1)) {
        throw std::runtime_error("Failed to convert wide character to multibyte string");
    }

    return {buffer.data(), length};
}

} // namespace ax