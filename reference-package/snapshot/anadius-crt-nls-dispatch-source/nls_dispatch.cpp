#include "nls_dispatch.hpp"

namespace anadius::crt_nls_dispatch_recovered {
std::uint32_t locale_name_to_lcid_0672c0(Address locale_name, std::uint32_t flags,
                                      Effects& effects) {
    const auto function = effects.resolve_0669b0(0x13, "LocaleNameToLCID", locale_name_modules);
    if (function != 0) {
        return effects.call_locale_name_to_lcid(function, locale_name, flags);
    }
    return effects.fallback_locale_name_07310c(locale_name);
}

std::int32_t lc_map_string_0671d4(Address locale_name, std::uint32_t flags,
    Address source, std::int32_t source_count, Address destination,
    std::int32_t destination_count, Address version, Address reserved,
    Address sort_handle, Effects& effects) {
    const auto function = effects.resolve_0669b0(0x11, "LCMapStringEx", lcmap_modules);
    if (function != 0) {
        return effects.call_lc_map_string_ex(function, locale_name, flags, source,
            source_count, destination, destination_count, version, reserved, sort_handle);
    }
    const auto locale = locale_name_to_lcid_0672c0(locale_name, 0, effects);
    return effects.call_lc_map_string_w(locale, flags, source, source_count,
                                      destination, destination_count);
}
}
