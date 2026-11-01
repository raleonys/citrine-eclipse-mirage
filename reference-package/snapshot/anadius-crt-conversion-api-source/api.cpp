#include "api.hpp"

namespace anadius::crt_conversion_api {
namespace {
bool zero_flags_code_page(std::uint32_t cp) noexcept {
    switch (cp) {
    case 42: case 50220: case 50221: case 50222: case 50225:
    case 50227: case 50229: case 65000:
        return true;
    default:
        return cp >= 57002 && cp <= 57011;
    }
}
}
std::int32_t multi_byte_to_wide_06ccc8(std::uint32_t cp,
    std::uint32_t flags, Address source, std::int32_t source_count,
    Address destination, std::int32_t capacity, Effects& effects) {
    if (zero_flags_code_page(cp)) flags = 0;
    else if (cp == 54936 || cp == 65001) flags &= 8;
    return effects.multi_byte_to_wide(cp, flags, source, source_count,
                                      destination, capacity);
}
std::int32_t wide_char_to_multi_byte_06cd58(std::uint32_t cp,
    std::uint32_t flags, Address source, std::int32_t source_count,
    Address destination, std::int32_t capacity,
    Address default_character, Address used_default_character, Effects& effects) {
    if (zero_flags_code_page(cp) || cp == 54936 || cp == 65001) flags = 0;
    else flags &= 0xffffff7fU;
    // The unsigned LEA/CMP accepts exactly 65000 and 65001, even at uint32 wrap.
    if (cp - std::uint32_t{65000} <= 1) {
        if (used_default_character != 0)
            effects.write_bool32(used_default_character, 0);
        default_character = 0;
        used_default_character = 0;
    }
    return effects.wide_char_to_multi_byte(cp, flags, source, source_count,
        destination, capacity, default_character, used_default_character);
}
std::uint32_t file_api_ansi_066c1c(crt_runtime_recovered::State& state,
    crt_runtime_recovered::Effects& resolver, FileApiEffects& effects) {
    constexpr std::uint32_t candidates[]{16}; // VA 18008b8f8, one uint32 entry.
    const auto address = crt_runtime_recovered::resolve_0669b0(
        state, resolver, 0, "AreFileApisANSI", candidates);
    return address == 0 ? 1 : effects.call_are_file_apis_ansi(address);
}
} // namespace anadius::crt_conversion_api
