#pragma once
#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace platform {

enum class OptionalHostCompatScanStatus : uint32_t {
    invalid_argument = 0,
    no_match = 1,
    one_match = 2,
    multiple_matches = 3,
    image_unavailable = 4,
    invalid_pe = 5,
    text_section_missing = 6,
};

struct OptionalHostCompatScanResult {
    OptionalHostCompatScanStatus status;
    uint32_t hit_count;
    uint32_t first_rva;
};

constexpr size_t optional_host_compat_signature_length = 44u;

// The protected 0x6B31 path is recovered as a scan of the host .text
// for this 44-byte wildcard signature. This function reproduces only the
// scanner semantics; it deliberately does not fabricate the still-unknown
// 0x10150/0x1019D callback behavior.
OptionalHostCompatScanResult scan_optional_host_compat_text(
    const uint8_t* text, size_t text_size, uint32_t text_rva) noexcept;

#if defined(_WIN32)
// Scans the process main module's mapped .text section using the curio
// signature. A no-match result is a normal no-op compatibility outcome.
OptionalHostCompatScanResult scan_main_module_optional_host_compat() noexcept;
#endif

} }
