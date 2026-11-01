#pragma once
#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace platform { namespace addon_hook_policy {
// Conservative, bounded, ASCII casefold matching against a configured path.
// Missing, Unicode non-ASCII, incomplete or overlong input never blocks CreateProcess.
inline bool ascii_fold_equal(const char* actual, const char* configured,
                             size_t configured_len, size_t limit = 32768u) noexcept {
    if (!actual || !configured || !configured_len || configured_len >= limit) return false;
    for (size_t i = 0; i < configured_len; ++i) {
        const unsigned char a = static_cast<unsigned char>(actual[i]);
        const unsigned char b = static_cast<unsigned char>(configured[i]);
        if (!a || a > 127u || b == 0u || b > 127u) return false;
        const unsigned char al = a >= 'A' && a <= 'Z' ? static_cast<unsigned char>(a + 32) : a;
        const unsigned char bl = b >= 'A' && b <= 'Z' ? static_cast<unsigned char>(b + 32) : b;
        if (al != bl) return false;
    }
    return actual[configured_len] == '\0';
}
inline bool ascii_fold_equal_wide(const wchar_t* actual, const char* configured,
                                  size_t configured_len, size_t limit = 32768u) noexcept {
    if (!actual || !configured || !configured_len || configured_len >= limit) return false;
    for (size_t i = 0; i < configured_len; ++i) {
        const uint32_t a = static_cast<uint32_t>(actual[i]);
        const unsigned char b = static_cast<unsigned char>(configured[i]);
        if (!a || a > 127u || b == 0u || b > 127u) return false;
        const uint32_t al = a >= 'A' && a <= 'Z' ? a + 32 : a;
        const uint32_t bl = b >= 'A' && b <= 'Z' ? b + 32u : b;
        if (al != bl) return false;
    }
    return actual[configured_len] == L'\0';
}
inline bool exact_true(const char* value, size_t count) noexcept {
    return value && ((count == 4 && value[0]=='t' && value[1]=='r' && value[2]=='u' && value[3]=='e') ||
                     (count == 1 && value[0]=='1'));
}
inline bool selected_documents(int csidl, const char* documents, size_t units) noexcept {
    return csidl == 5 && documents && units > 0 && units < 32768u;
}
} } }
