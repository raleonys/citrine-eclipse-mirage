#include "preloader_l/platform/optional_host_compat.hpp"

namespace preloader_l { namespace platform {
namespace {
constexpr uint8_t k_signature[optional_host_compat_signature_length] = {
    0x48,0x89,0x5c,0x24,0x00,0x4c,0x89,0x4c,0x24,0x00,0x48,0x89,0x4c,0x24,0x00,
    0x55,0x56,0x57,0x41,0x54,0x41,0x55,0x41,0x56,0x41,0x57,0x48,0x8d,0xac,0x24,
    0x00,0x00,0x00,0x00,0x48,0x81,0xec,0x00,0x00,0x00,0x00,0x4c,0x8b,0xea
};
constexpr uint8_t k_mask[optional_host_compat_signature_length] = {
    1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    0,0,0,0,1,1,1,0,0,0,0,1,1,1
};

bool signature_matches(const uint8_t* p) noexcept {
    for (size_t i = 0; i < optional_host_compat_signature_length; ++i) {
        if (k_mask[i] && p[i] != k_signature[i]) return false;
    }
    return true;
}
}

OptionalHostCompatScanResult scan_optional_host_compat_text(
    const uint8_t* text, size_t text_size, uint32_t text_rva) noexcept
{
    if (!text) {
        return OptionalHostCompatScanResult{
            OptionalHostCompatScanStatus::invalid_argument, 0u, 0u};
    }
    if (text_size < optional_host_compat_signature_length) {
        return OptionalHostCompatScanResult{
            OptionalHostCompatScanStatus::no_match, 0u, 0u};
    }

    uint32_t hits = 0u;
    uint32_t first = 0u;
    const size_t last = text_size - optional_host_compat_signature_length;
    for (size_t offset = 0; offset <= last; ++offset) {
        if (!signature_matches(text + offset)) continue;
        if (hits == 0u) {
            const uint64_t rva = static_cast<uint64_t>(text_rva) +
                                 static_cast<uint64_t>(offset);
            first = rva <= UINT32_MAX ? static_cast<uint32_t>(rva) : 0u;
        }
        if (hits != UINT32_MAX) ++hits;
    }

    const auto status = hits == 0u ? OptionalHostCompatScanStatus::no_match :
                        hits == 1u ? OptionalHostCompatScanStatus::one_match :
                                     OptionalHostCompatScanStatus::multiple_matches;
    return OptionalHostCompatScanResult{status, hits, first};
}

} }
