#pragma once
#include <array>
#include <cstdint>
#include <span>
#include <string_view>

namespace anadius::crt_nls_dispatch_recovered {
using Address = std::uint64_t;
// Read-only .rdata at 18008b9e0 and 18008ba18, respectively.
inline constexpr std::array<std::uint32_t, 2> lcmap_modules{3, 16};
inline constexpr std::array<std::uint32_t, 2> locale_name_modules{3, 16};

class Effects {
public:
    virtual ~Effects() = default;
    virtual Address resolve_0669b0(std::uint32_t procedure, std::string_view name,
                                  std::span<const std::uint32_t> modules) = 0;
    virtual std::int32_t call_lc_map_string_ex(Address function, Address locale_name,
        std::uint32_t flags, Address source, std::int32_t source_count,
        Address destination, std::int32_t destination_count, Address version,
        Address reserved, Address sort_handle) = 0;
    virtual std::int32_t call_lc_map_string_w(std::uint32_t locale,
        std::uint32_t flags, Address source, std::int32_t source_count,
        Address destination, std::int32_t destination_count) = 0;
    virtual std::uint32_t call_locale_name_to_lcid(Address function,
        Address locale_name, std::uint32_t flags) = 0;
    virtual std::uint32_t fallback_locale_name_07310c(Address locale_name) = 0;
};

std::int32_t lc_map_string_0671d4(Address locale_name, std::uint32_t flags,
    Address source, std::int32_t source_count, Address destination,
    std::int32_t destination_count, Address version, Address reserved,
    Address sort_handle, Effects& effects);
std::uint32_t locale_name_to_lcid_0672c0(Address locale_name, std::uint32_t flags,
                                      Effects& effects);
}
