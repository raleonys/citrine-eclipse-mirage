#pragma once
// ASCII-only paths in synthetic fixtures; never truncate a UTF-16 code unit.
#include <stdexcept>
#include <string>
#include <string_view>

namespace preloader_l::test_support {
inline std::string fixture_ascii(std::u16string_view value) {
    std::string result;
    result.reserve(value.size());
    for (const char16_t unit : value) {
        if (unit > 0x7f)
            throw std::invalid_argument("Non-ASCII path entered ASCII-only test fixture");
        result.push_back(static_cast<char>(unit));
    }
    return result;
}
} // namespace preloader_l::test_support
