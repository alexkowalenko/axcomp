//
// AX compiler
//
// Copyright © 2020 Alex Kowalenko
//

#pragma once

#include "error.hh"
#include "lexer.hh"

#include <codecvt>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wold-style-cast"
#include <unicode/uchar.h>
#include <utf8.h>
#pragma clang diagnostic pop

namespace ax {

inline std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;

// Custom emoji checker
// This is a hack.
inline constexpr bool is_emoji(Char c) {
    return (0x1f600 <= c && c <= 0x1f64f) || // Emoticons NOLINT
           (0x1F300 <= c && c <= 0x1F5FF) || // Misc Symbols and Pictographs NOLINT
           (0x1F680 <= c && c <= 0x1F6FF) || // Transport and Map NOLINT
           (0x1F1E6 <= c && c <= 0x1F1FF) || // Regional country flags NOLINT
           (0x2600 <= c && c <= 0x26FF) ||   // Misc symbols NOLINT
           (0x2700 <= c && c <= 0x27BF) ||   // Dingbats NOLINT
           (0xE0020 <= c && c <= 0xE007F) || // Tags NOLINT
           (0xFE00 <= c && c <= 0xFE0F) ||   // Variation Selectors NOLINT
           (0x1F900 <= c && c <= 0x1F9FF) || // Supplemental Symbols and Pictographs NOLINT
           (0x1F018 <= c && c <= 0x1F270) || // Various asian characters NOLINT
           (0x238C <= c && c <= 0x2454) ||   // Misc items NOLINT
           (0x20D0 <= c && c <= 0x20FF);     // NOLINT
}

class Character32 : CharacterClass<Char> {
  public:
    ~Character32() override = default;
    static bool        isspace(Char c) { return std::iswspace(c); };
    static bool        isxdigit(Char c) { return std::iswxdigit(c); };
    static bool        isdigit(Char c) { return std::iswdigit(c); };
    static bool        isalnum(Char c) { return u_isalnum(c) || is_emoji(c); };
    static bool        isalpha(Char c) { return u_isalpha(c) || is_emoji(c); };
    static std::string to_string(Char c) { return converterX.to_bytes(std::wstring(1, c)); }
    static void        add_string(std::string &s, Char c) {
        try {
            utf8::append(char32_t(c), s);
        } catch (utf8::invalid_code_point &e) {
            throw LexicalException("Invalid code Point", Location(0, 0));
        };
    }
};

class LexerUTF8 : public LexerImplementation<Char, Character32> {
  public:
    LexerUTF8(std::istream &stream, ErrorManager const &e) : LexerImplementation{stream, e} {
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