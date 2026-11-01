#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
namespace anadius::crt_initial_multibyte {
inline constexpr std::size_t record_size=0x228,type_size=257,case_size=256;
// Exact pointer-free image data. The returned immutable record is not mutable
// reference-counter storage: copy into caller-owned typed/lifetime-safe memory.
const std::array<std::uint8_t,record_size>& record_bytes() noexcept;
const std::array<std::uint8_t,type_size>& initial_type_table() noexcept;
const std::array<std::uint8_t,case_size>& initial_case_table() noexcept;
struct GlobalScalars {
    std::uint32_t locale_mask,locale_changed,system_cp_flag,initialized;
};
// Image initial values only. Current pointers are loader-zero-filled and the
// preferred-image initial-pair pointers require a separate live relocation.
const GlobalScalars& image_globals() noexcept;
}
