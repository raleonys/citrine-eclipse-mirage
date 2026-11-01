#pragma once
#include <cstdint>
#include <span>
#include <string_view>

namespace anadius::crt::initial_locale_data {
struct Region { std::string_view name; std::uint64_t preferred_va; std::span<const std::uint8_t> bytes; };
struct Relocation { std::uint32_t source_region, source_offset, target_region, target_offset; };
enum Root : std::uint32_t { Locale = 0, Lconv = 1, Time = 2, Category = 3 };
// Bytes contain zero in every pointer slot. Allocate/copy each region then apply relocations.
// preferred_va is evidence metadata only; never dereference or install it as a live pointer.
std::span<const Region> regions() noexcept;
std::span<const Relocation> relocations() noexcept;
}
