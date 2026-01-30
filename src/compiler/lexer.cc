//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#include <cctype>
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
#include "options.hh"

namespace ax {

namespace {

inline const std::map<std::string, Token> keyword_map = {
    {"MODULE", Token(TokenType::MODULE, "MODULE")},
    {"BEGIN", Token(TokenType::BEGIN, "BEGIN")},
    {"END", Token(TokenType::END, "END")},
    {"DIV", Token(TokenType::DIV, "DIV")},
    {"MOD", Token(TokenType::MOD, "MOD")},
    {"CONST", Token(TokenType::CONST, "CONST")},
    {"TYPE", Token(TokenType::TYPE, "TYPE")},
    {"VAR", Token(TokenType::VAR, "VAR")},
    {"RETURN", Token(TokenType::RETURN, "RETURN")},
    {"PROCEDURE", Token(TokenType::PROCEDURE, "PROCEDURE")},
    {"TRUE", Token(TokenType::TRUE, "TRUE")},
    {"FALSE", Token(TokenType::FALSE, "FALSE")},
    {"OR", Token(TokenType::OR, "OR")},
    {"IF", Token(TokenType::IF, "IF")},
    {"THEN", Token(TokenType::THEN, "THEN")},
    {"ELSIF", Token(TokenType::ELSIF, "ELSIF")},
    {"ELSE", Token(TokenType::ELSE, "ELSE")},
    {"FOR", Token(TokenType::FOR, "FOR")},
    {"TO", Token(TokenType::TO, "TO")},
    {"BY", Token(TokenType::BY, "BY")},
    {"DO", Token(TokenType::DO, "DO")},
    {"WHILE", Token(TokenType::WHILE, "WHILE")},
    {"REPEAT", Token(TokenType::REPEAT, "REPEAT")},
    {"UNTIL", Token(TokenType::UNTIL, "UNTIL")},
    {"LOOP", Token(TokenType::LOOP, "LOOP")},
    {"EXIT", Token(TokenType::EXIT, "EXIT")},
    {"ARRAY", Token(TokenType::ARRAY, "ARRAY")},
    {"OF", Token(TokenType::OF, "OF")},
    {"RECORD", Token(TokenType::RECORD, "RECORD")},
    {"DEFINITION", Token(TokenType::DEFINITION, "DEFINITION")},
    {"IMPORT", Token(TokenType::IMPORT, "IMPORT")},
    {"CASE", Token(TokenType::CASE, "CASE")},
    {"POINTER", Token(TokenType::POINTER, "POINTER")},
    {"NIL", Token(TokenType::NIL, "NIL")},
    {"IN", Token(TokenType::IN, "IN")},
};

const std::map<char, Token> single_tokens = {
    {-1, Token(TokenType::Eof)},
    {';', Token(TokenType::SEMICOLON, ";")},
    {',', Token(TokenType::COMMA, ",")},
    {'+', Token(TokenType::PLUS, "+")},
    {'-', Token(TokenType::DASH, "-")},
    {'*', Token(TokenType::ASTÉRIX, "*")},
    {'(', Token(TokenType::L_PAREN, "(")},
    {')', Token(TokenType::R_PAREN, ")")},
    {'=', Token(TokenType::EQUALS, "=")},
    {'#', Token(TokenType::HASH, "#")},
    {'~', Token(TokenType::TILDE, "~")},
    {'&', Token(TokenType::AMPERSAND, "&")},
    {'[', Token(TokenType::L_BRACKET, "[")},
    {']', Token(TokenType::R_BRACKET, "]")},
    {'|', Token(TokenType::BAR, "|")},
    {'/', Token(TokenType::SLASH, "/")},
    {'^', Token(TokenType::CARET, "^")},
    {'{', Token(TokenType::L_BRACE, "{")},
    {'}', Token(TokenType::R_BRACE, "}")},
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

LexerUTF8::LexerUTF8(std::string text, ErrorManager const &e, Options *opts)
    : buffer(std::move(text)), errors(e), options(opts) {
    if (!utf8::is_valid(buffer.begin(), buffer.end())) {
        throw LexicalException(get_location(), "Not valid UTF-8 text");
    }
    cursor = buffer.cbegin();
}

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
    Char c = get_char();
    while (c == '<' && peek() == '*') {
        scan_directive();
        c = get_char();
    }

    // Check single-digit character tokens
    if (const auto res = single_tokens.find(static_cast<char>(c)); res != single_tokens.end()) {
        return res->second;
    }

    // Check multiple character tokens
    switch (c) {
    case ':':
        if (peek() == '=') {
            get_char();
            return {TokenType::ASSIGN, ":="};
        }
        return {TokenType::COLON, ":"};
    case '<':
        if (peek() == '=') {
            get_char();
            return {TokenType::LEQ, "<="};
        }
        return {TokenType::LESS, "<"};
    case '>':
        if (peek() == '=') {
            get_char();
            return {TokenType::GTEQ, ">="};
        }
        return {TokenType::GREATER, ">"};
    case '.':
        if (peek() == '.') {
            get_char();
            return {TokenType::DOTDOT, ".."};
        }
        return {TokenType::PERIOD, "."};
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
    get(); // consume the initial '*'
    while (true) {
        const auto c = get();
        if (c == -1) {
            throw LexicalException(get_location(), "Unterminated comment");
        }
        if (c == '*' && peek() == ')') {
            get();
            return;
        }
        if (c == '(' && peek() == '*') {
            // support nested comments, call recursively
            get_comment();
        }
        if (c == '\n') {
            set_newline();
        }
    }
}

void LexerUTF8::get_line_comment() {
    // Consume the second '/'
    get();
    while (true) {
        const auto c = get();
        if (c == -1) {
            return;
        }
        if (c == '\n') {
            set_newline();
            return;
        }
    }
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
        return {TokenType::HEXINTEGER, digit};
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
            return {TokenType::INTEGER, digit};
        }
        digit += '.';
        while (std::iswdigit(c)) {
            get();
            digit += static_cast<char>(c);
            c = peek();
        }
        if (c == 'e' || c == 'E' || c == 'D') {
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
        return {TokenType::REAL, digit};
    }
    if (c == 'X') {
        // Characters in this format 0d34X
        get();
        return {TokenType::HEXCHR, digit};
    };
    return {TokenType::INTEGER, digit};
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
    bool has_upper = false;
    bool has_lower = false;
    for (unsigned char const ch : ident) {
        if (ch > 0x7F) {
            continue;
        }
        if (std::isupper(ch)) {
            has_upper = true;
        } else if (std::islower(ch)) {
            has_lower = true;
        }
    }
    if (!(has_upper && has_lower)) {
        if (const auto res = keyword_map.find(ident); res != keyword_map.end()) {
            return res->second;
        }
        if (has_lower) {
            std::string upper_ident = ident;
            for (char &ch : upper_ident) {
                if (ch >= 'a' && ch <= 'z') {
                    ch = static_cast<char>(ch - 'a' + 'A');
                }
            }
            if (const auto res = keyword_map.find(upper_ident); res != keyword_map.end()) {
                return res->second;
            }
        }
    }
    return {TokenType::IDENT, ident};
}

void LexerUTF8::scan_directive() {
    // Consume '<' already read by get_token, next char is '*'
    get(); // consume '*'
    Char c = get();
    auto skip_ws = [&]() {
        while (c != -1 && std::iswspace(c)) {
            if (c == '\n') {
                set_newline();
            }
            c = get();
        }
    };
    skip_ws();
    if (c == -1) {
        throw LexicalException(get_location(), "Unterminated directive");
    }
    if (!(u_isalnum(c) || c == '_')) {
        throw LexicalException(get_location(), "Invalid directive");
    }
    std::string ident;
    utf8::append(static_cast<char32_t>(c), ident);
    c = peek();
    while (u_isalnum(c) || c == '_') {
        get();
        utf8::append(static_cast<char32_t>(c), ident);
        c = peek();
    }

    c = get();
    skip_ws();
    if (c != '+' && c != '-') {
        throw LexicalException(get_location(), "Invalid directive");
    }
    const bool enable = (c == '+');

    c = get();
    skip_ws();
    if (c != '*') {
        throw LexicalException(get_location(), "Invalid directive");
    }
    c = get();
    if (c != '>') {
        throw LexicalException(get_location(), "Invalid directive");
    }

    if (options && (ident == "MAIN" || ident == "main")) {
        options->output_main = enable;
    } else if (!(ident == "MAIN" || ident == "main")) {
        throw LexicalException(get_location(), "Unknown directive: " + ident);
    }
}

Token LexerUTF8::scan_string(Char const start) {
    // Already scanned ' or "
    std::string str;
    utf8::append(static_cast<char32_t>(start), str);
    Char c = get();
    if (c == start) {
        // empty string
        return {TokenType::STRING, str};
    }
    utf8::append(static_cast<char32_t>(c), str);
    const Char final = get();
    if (final == '\'' && start == '\'') {
        return {TokenType::CHR, static_cast<long>(c)};
    };
    if (final == start) {
        return {TokenType::STRING, str};
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
    return {TokenType::STRING, str};
}

/**
 * @brief get first non whitespace or comment character
 *
 * @return char
 */
Char LexerUTF8::get_char() {
    while (true) {
        const auto c = get();
        if (c == -1) {
            return -1;
        }
        if (c == '\n') {
            set_newline();
            continue;
        }
        if (c == '(' && peek() == '*') {
            get_comment();
            continue;
        }
        if (c == '/' && peek() == '/') {
            get_line_comment();
            continue;
        }
        if (std::iswspace(c)) {
            continue;
        }
        return c;
    }
}

Char LexerUTF8::get() {
    Char c = 0;
    if (last_char != 0) {
        std::swap(c, last_char);
        return c;
    }
    if (cursor == buffer.cend()) {
        return -1;
    }
    try {
        c = utf8::next(cursor, buffer.cend());
    } catch (std::exception const &) {
        throw LexicalException(get_location(), "Not valid UTF-8 text");
    }
    char_pos++;
    return c;
}

Char LexerUTF8::peek() const {
    if (last_char != 0) {
        return last_char;
    }
    if (cursor == buffer.cend()) {
        return -1;
    }
    const auto tmp = cursor;
    try {
        return static_cast<Char>(utf8::peek_next(tmp, buffer.cend()));
    } catch (std::exception const &) {
        throw LexicalException(get_location(), "Not valid UTF-8 text");
    }
}

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
