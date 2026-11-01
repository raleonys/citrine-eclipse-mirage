#include "search.hpp"
#include <bit>
#include <limits>
#include <stdexcept>

namespace anadius::crt_wildcard_search {
namespace {
bool separator(std::uint8_t value) {
    return value == '/' || value == ':' || value == '\\';
}
std::int64_t element_count(const Vector& vector) {
    const auto difference = vector.end - vector.begin;
    // x86-64 SUB/SAR, including signed interpretation, without C++ overflow.
    const auto signed_difference = std::bit_cast<std::int64_t>(difference);
    return signed_difference >= 0 ? signed_difference / 8
        : -1 - static_cast<std::int64_t>((~difference) / 8);
}
}
std::uint32_t search_06e984(Address pattern, Address wildcard, Vector& vector, Effects& fx) {
    auto position = wildcard;
    while (position != pattern && !separator(fx.read_u8(position))) {
        const auto previous = fx.previous_077168(pattern, position);
        if (previous < pattern || previous >= position)
            throw std::domain_error("previous-character provider did not progress within pattern");
        position = previous;
    }
    const auto character = fx.read_u8(position);
    if (character == ':' && position != pattern + 1)
        return crt_wildcard_storage::append_06e800(pattern, 0, 0, vector, fx);
    const auto prefix_bytes = separator(character) ? position - pattern + 1 : 0;
    const auto wide = fx.to_wide_06004c(pattern, fx.choose_code_page());
    Address filename{};
    const auto handle = fx.find_first(wide.status == 0 ? wide.data : 0, filename);
    if (handle == std::numeric_limits<Address>::max()) {
        const auto result = crt_wildcard_storage::append_06e800(pattern, 0, 0, vector, fx);
        if (wide.owned) fx.free_0668c0(wide.data);
        return result;
    }
    const auto initial_count = element_count(vector);
    for (;;) {
        const auto narrow = fx.to_narrow_06e430(filename, fx.choose_code_page());
        const auto text = narrow.status == 0 ? narrow.data : 0;
        if (text == 0)
            throw std::domain_error("original narrow conversion path dereferences NULL");
        bool dot = false;
        if (fx.read_u8(text) == '.') {
            const auto second = fx.read_u8(text + 1);
            dot = second == 0 || (second == '.' && fx.read_u8(text + 2) == 0);
        }
        if (!dot) {
            const auto result = crt_wildcard_storage::append_06e800(text, pattern, prefix_bytes, vector, fx);
            if (result != 0) {
                if (narrow.owned) fx.free_0668c0(narrow.data);
                fx.find_close(handle);
                if (wide.owned) fx.free_0668c0(wide.data);
                return result;
            }
        }
        if (narrow.owned) fx.free_0668c0(narrow.data);
        if (fx.find_next(handle, filename) == 0) break;
    }
    const auto final_count = element_count(vector);
    if (initial_count != final_count) {
        const auto first = vector.begin + static_cast<std::uint64_t>(initial_count) * 8;
        const auto count = static_cast<std::uint64_t>(final_count) - static_cast<std::uint64_t>(initial_count);
        fx.sort_new_segment(first, count);
    }
    fx.find_close(handle);
    if (wide.owned) fx.free_0668c0(wide.data);
    return 0;
}
} // namespace anadius::crt_wildcard_search
