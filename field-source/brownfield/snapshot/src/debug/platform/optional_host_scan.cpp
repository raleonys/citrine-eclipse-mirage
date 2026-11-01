#if !defined(_WIN32)
#error optional_host_scan.cpp is Windows-only.
#endif

#include "preloader_l/platform/optional_host_compat.hpp"
#include <stddef.h>
#include <stdint.h>

namespace {
using HMODULE = void*;
extern "C" __declspec(dllimport) HMODULE __stdcall GetModuleHandleW(const wchar_t* name);

constexpr uint16_t k_dos_magic = 0x5a4du;
constexpr uint32_t k_pe_magic = 0x00004550u;
constexpr uint16_t k_pe32_plus_magic = 0x020bu;
constexpr size_t k_dos_lfanew = 0x3cu;
constexpr size_t k_file_header_size = 20u;
constexpr size_t k_optional_header_offset_from_nt = 4u + k_file_header_size;
constexpr size_t k_optional_size_of_image_offset = 56u;

uint16_t u16(const uint8_t* p) noexcept {
    return static_cast<uint16_t>(p[0]) |
           static_cast<uint16_t>(static_cast<uint16_t>(p[1]) << 8u);
}
uint32_t u32(const uint8_t* p) noexcept {
    return static_cast<uint32_t>(p[0]) |
           (static_cast<uint32_t>(p[1]) << 8u) |
           (static_cast<uint32_t>(p[2]) << 16u) |
           (static_cast<uint32_t>(p[3]) << 24u);
}
bool within(size_t off, size_t need, size_t size) noexcept {
    return off <= size && need <= size - off;
}
bool is_text_name(const uint8_t* p) noexcept {
    return p[0]=='.' && p[1]=='t' && p[2]=='e' && p[3]=='x' && p[4]=='t' &&
           p[5]==0 && p[6]==0 && p[7]==0;
}
}

namespace preloader_l { namespace platform {

OptionalHostCompatScanResult scan_main_module_optional_host_compat() noexcept
{
    auto* base = static_cast<uint8_t*>(GetModuleHandleW(nullptr));
    if (!base) {
        return OptionalHostCompatScanResult{
            OptionalHostCompatScanStatus::image_unavailable, 0u, 0u};
    }

    // PE headers for a loaded x64 image are expected in the first mapped page.
    constexpr size_t header_probe = 0x1000u;
    if (!within(0, 2, header_probe) || u16(base) != k_dos_magic ||
        !within(k_dos_lfanew, 4, header_probe)) {
        return OptionalHostCompatScanResult{
            OptionalHostCompatScanStatus::invalid_pe, 0u, 0u};
    }
    const uint32_t nt = u32(base + k_dos_lfanew);
    if (!within(nt, 24u, header_probe) || u32(base + nt) != k_pe_magic) {
        return OptionalHostCompatScanResult{
            OptionalHostCompatScanStatus::invalid_pe, 0u, 0u};
    }

    const uint16_t section_count = u16(base + nt + 6u);
    const uint16_t optional_size = u16(base + nt + 20u);
    const size_t optional = static_cast<size_t>(nt) + k_optional_header_offset_from_nt;
    if (!within(optional, optional_size, header_probe) ||
        optional_size < k_optional_size_of_image_offset + 4u ||
        u16(base + optional) != k_pe32_plus_magic) {
        return OptionalHostCompatScanResult{
            OptionalHostCompatScanStatus::invalid_pe, 0u, 0u};
    }
    const uint32_t image_size = u32(base + optional + k_optional_size_of_image_offset);
    if (!image_size) {
        return OptionalHostCompatScanResult{
            OptionalHostCompatScanStatus::invalid_pe, 0u, 0u};
    }

    const size_t sections = optional + optional_size;
    if (!within(sections, static_cast<size_t>(section_count) * 40u, header_probe)) {
        return OptionalHostCompatScanResult{
            OptionalHostCompatScanStatus::invalid_pe, 0u, 0u};
    }
    for (uint16_t i = 0; i < section_count; ++i) {
        const uint8_t* sh = base + sections + static_cast<size_t>(i) * 40u;
        if (!is_text_name(sh)) continue;
        const uint32_t virtual_size = u32(sh + 8u);
        const uint32_t virtual_address = u32(sh + 12u);
        const uint32_t raw_size = u32(sh + 16u);
        const uint32_t span = virtual_size ? virtual_size : raw_size;
        if (!span || virtual_address >= image_size || span > image_size - virtual_address) {
            return OptionalHostCompatScanResult{
                OptionalHostCompatScanStatus::invalid_pe, 0u, 0u};
        }
        return scan_optional_host_compat_text(
            base + virtual_address, static_cast<size_t>(span), virtual_address);
    }

    return OptionalHostCompatScanResult{
        OptionalHostCompatScanStatus::text_section_missing, 0u, 0u};
}

} }
