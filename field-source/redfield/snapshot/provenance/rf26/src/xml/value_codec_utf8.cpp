// SPDX-License-Identifier: GPL-3.0-only
#include "value_codec_internal.hpp"
#include "preloader_l/core/checked_math.hpp"

namespace preloader_l { namespace xml { namespace value_codec_detail {


bool valid_span(TextSpan s) noexcept {
    return s.data != nullptr || s.size == 0;
}

bool add_size(size_t* total, size_t value) noexcept {
    size_t next = 0;
    if (!total || !preloader_l::core::checked_add_size(*total, value, next)) return false;
    *total = next;
    return true;
}

bool is_xml_char(uint32_t cp) noexcept {
    return cp == 0x09u || cp == 0x0Au || cp == 0x0Du ||
           (cp >= 0x20u && cp <= 0xD7FFu) ||
           (cp >= 0xE000u && cp <= 0xFFFDu) ||
           (cp >= 0x10000u && cp <= 0x10FFFFu);
}

size_t utf8_encode(uint32_t cp, char out[4]) noexcept {
    if (cp <= 0x7Fu) {
        out[0] = static_cast<char>(cp);
        return 1;
    }
    if (cp <= 0x7FFu) {
        out[0] = static_cast<char>(0xC0u | (cp >> 6));
        out[1] = static_cast<char>(0x80u | (cp & 0x3Fu));
        return 2;
    }
    if (cp <= 0xFFFFu) {
        out[0] = static_cast<char>(0xE0u | (cp >> 12));
        out[1] = static_cast<char>(0x80u | ((cp >> 6) & 0x3Fu));
        out[2] = static_cast<char>(0x80u | (cp & 0x3Fu));
        return 3;
    }
    out[0] = static_cast<char>(0xF0u | (cp >> 18));
    out[1] = static_cast<char>(0x80u | ((cp >> 12) & 0x3Fu));
    out[2] = static_cast<char>(0x80u | ((cp >> 6) & 0x3Fu));
    out[3] = static_cast<char>(0x80u | (cp & 0x3Fu));
    return 4;
}


bool is_continuation(unsigned char c) noexcept {
    return (c & 0xC0u) == 0x80u;
}

Utf8Unit validate_utf8_unit(const char* s, size_t n, size_t pos) noexcept {
    Utf8Unit out{0u, 0u, ValueCodecStatus::ok};
    if (!s || pos >= n) {
        out.status = ValueCodecStatus::invalid_argument;
        return out;
    }

    const unsigned char b0 = static_cast<unsigned char>(s[pos]);
    if (b0 <= 0x7Fu) {
        out.codepoint = b0;
        out.size = 1u;
    } else if (b0 >= 0xC2u && b0 <= 0xDFu) {
        if (n - pos < 2u) { out.status = ValueCodecStatus::invalid_character; return out; }
        const unsigned char b1 = static_cast<unsigned char>(s[pos + 1u]);
        if (!is_continuation(b1)) { out.status = ValueCodecStatus::invalid_character; return out; }
        out.codepoint = (static_cast<uint32_t>(b0 & 0x1Fu) << 6) |
                        static_cast<uint32_t>(b1 & 0x3Fu);
        out.size = 2u;
    } else if (b0 >= 0xE0u && b0 <= 0xEFu) {
        if (n - pos < 3u) { out.status = ValueCodecStatus::invalid_character; return out; }
        const unsigned char b1 = static_cast<unsigned char>(s[pos + 1u]);
        const unsigned char b2 = static_cast<unsigned char>(s[pos + 2u]);
        const bool b1_ok = b0 == 0xE0u ? (b1 >= 0xA0u && b1 <= 0xBFu)
                         : b0 == 0xEDu ? (b1 >= 0x80u && b1 <= 0x9Fu)
                                       : is_continuation(b1);
        if (!b1_ok || !is_continuation(b2)) { out.status = ValueCodecStatus::invalid_character; return out; }
        out.codepoint = (static_cast<uint32_t>(b0 & 0x0Fu) << 12) |
                        (static_cast<uint32_t>(b1 & 0x3Fu) << 6) |
                        static_cast<uint32_t>(b2 & 0x3Fu);
        out.size = 3u;
    } else if (b0 >= 0xF0u && b0 <= 0xF4u) {
        if (n - pos < 4u) { out.status = ValueCodecStatus::invalid_character; return out; }
        const unsigned char b1 = static_cast<unsigned char>(s[pos + 1u]);
        const unsigned char b2 = static_cast<unsigned char>(s[pos + 2u]);
        const unsigned char b3 = static_cast<unsigned char>(s[pos + 3u]);
        const bool b1_ok = b0 == 0xF0u ? (b1 >= 0x90u && b1 <= 0xBFu)
                         : b0 == 0xF4u ? (b1 >= 0x80u && b1 <= 0x8Fu)
                                       : is_continuation(b1);
        if (!b1_ok || !is_continuation(b2) || !is_continuation(b3)) {
            out.status = ValueCodecStatus::invalid_character;
            return out;
        }
        out.codepoint = (static_cast<uint32_t>(b0 & 0x07u) << 18) |
                        (static_cast<uint32_t>(b1 & 0x3Fu) << 12) |
                        (static_cast<uint32_t>(b2 & 0x3Fu) << 6) |
                        static_cast<uint32_t>(b3 & 0x3Fu);
        out.size = 4u;
    } else {
        out.status = ValueCodecStatus::invalid_character;
        return out;
    }

    if (!is_xml_char(out.codepoint)) out.status = ValueCodecStatus::invalid_character;
    return out;
}

DecodedUnit decode_unit(const char* s, size_t n, size_t pos) noexcept {
    DecodedUnit out{{0,0,0,0},0,0,ValueCodecStatus::ok};
    if (!s || pos >= n) {
        out.status = ValueCodecStatus::invalid_argument;
        return out;
    }
    if (s[pos] != '&') {
        // HOTFIX14 / F1573-001: raw XML-source bytes must obey the same UTF-8
        // and XML Char contract as normalization/escaping and numeric entity
        // decoding. Previously this path copied one byte blindly, allowing
        // malformed UTF-8 and forbidden XML control characters to bypass the
        // codec contract in decode/compare callers.
        const Utf8Unit unit = validate_utf8_unit(s, n, pos);
        if (unit.status != ValueCodecStatus::ok) {
            out.status = unit.status;
            return out;
        }
        for (size_t i = 0; i < unit.size; ++i) out.bytes[i] = s[pos + i];
        out.size = unit.size;
        out.consumed = unit.size;
        return out;
    }

    size_t semi = pos + 1u;
    while (semi < n && s[semi] != ';') ++semi;
    if (semi >= n || semi == pos + 1u) {
        out.status = ValueCodecStatus::malformed_entity;
        return out;
    }
    const size_t body = pos + 1u;
    const size_t body_n = semi - body;

    auto match = [&](const char* literal, size_t literal_n, char value) noexcept -> bool {
        if (body_n != literal_n) return false;
        for (size_t i = 0; i < literal_n; ++i) if (s[body + i] != literal[i]) return false;
        out.bytes[0] = value;
        out.size = 1;
        out.consumed = semi - pos + 1u;
        return true;
    };

    if (match("amp", 3u, '&') || match("lt", 2u, '<') ||
        match("gt", 2u, '>') || match("quot", 4u, '"') ||
        match("apos", 4u, '\'')) return out;

    if (s[body] != '#') {
        out.status = ValueCodecStatus::malformed_entity;
        return out;
    }

    size_t p = body + 1u;
    bool hex = false;
    if (p < semi && (s[p] == 'x' || s[p] == 'X')) {
        hex = true;
        ++p;
    }
    if (p >= semi) {
        out.status = ValueCodecStatus::malformed_entity;
        return out;
    }

    uint32_t cp = 0;
    for (; p < semi; ++p) {
        const unsigned char c = static_cast<unsigned char>(s[p]);
        uint32_t digit = 0;
        if (c >= '0' && c <= '9') digit = static_cast<uint32_t>(c - '0');
        else if (hex && c >= 'a' && c <= 'f') digit = static_cast<uint32_t>(c - 'a' + 10u);
        else if (hex && c >= 'A' && c <= 'F') digit = static_cast<uint32_t>(c - 'A' + 10u);
        else {
            out.status = ValueCodecStatus::malformed_entity;
            return out;
        }
        const uint32_t base = hex ? 16u : 10u;
        if (cp > (UINT32_C(0x10ffff) - digit) / base) {
            out.status = ValueCodecStatus::invalid_character;
            return out;
        }
        cp = cp * base + digit;
    }
    if (!is_xml_char(cp)) {
        out.status = ValueCodecStatus::invalid_character;
        return out;
    }
    out.size = utf8_encode(cp, out.bytes);
    out.consumed = semi - pos + 1u;
    return out;
}

size_t literal_length(const char* literal) noexcept {
    size_t n = 0;
    if (literal) while (literal[n] != '\0') ++n;
    return n;
}


} } }
