#include "preloader_l/platform/optional_host_compat.hpp"
#include <assert.h>
#include <stdint.h>
#include <string.h>

using preloader_l::platform::OptionalHostCompatScanStatus;
using preloader_l::platform::scan_optional_host_compat_text;

namespace {
constexpr uint8_t k_concrete_signature[44] = {
    0x48,0x89,0x5c,0x24,0x18,0x4c,0x89,0x4c,0x24,0x20,0x48,0x89,0x4c,0x24,0x28,
    0x55,0x56,0x57,0x41,0x54,0x41,0x55,0x41,0x56,0x41,0x57,0x48,0x8d,0xac,0x24,
    0x90,0xfe,0xff,0xff,0x48,0x81,0xec,0x70,0x01,0x00,0x00,0x4c,0x8b,0xea
};
}

int main() {
    uint8_t text[160]{};
    auto none = scan_optional_host_compat_text(text, sizeof(text), 0x1000u);
    assert(none.status == OptionalHostCompatScanStatus::no_match);
    assert(none.hit_count == 0u && none.first_rva == 0u);

    memcpy(text + 17, k_concrete_signature, sizeof(k_concrete_signature));
    auto one = scan_optional_host_compat_text(text, sizeof(text), 0x2000u);
    assert(one.status == OptionalHostCompatScanStatus::one_match);
    assert(one.hit_count == 1u && one.first_rva == 0x2011u);

    // Every wildcard byte may vary without changing the curio match.
    const size_t wildcards[] = {4,9,14,30,31,32,33,37,38,39,40};
    for (size_t i = 0; i < sizeof(wildcards)/sizeof(wildcards[0]); ++i)
        text[17 + wildcards[i]] ^= static_cast<uint8_t>(0x5au + i);
    one = scan_optional_host_compat_text(text, sizeof(text), 0x2000u);
    assert(one.status == OptionalHostCompatScanStatus::one_match);
    assert(one.first_rva == 0x2011u);

    // A fixed byte must still reject the candidate.
    text[17 + 3] ^= 1u;
    auto rejected = scan_optional_host_compat_text(text, sizeof(text), 0x2000u);
    assert(rejected.status == OptionalHostCompatScanStatus::no_match);
    text[17 + 3] ^= 1u;

    memcpy(text + 91, k_concrete_signature, sizeof(k_concrete_signature));
    auto many = scan_optional_host_compat_text(text, sizeof(text), 0x2000u);
    assert(many.status == OptionalHostCompatScanStatus::multiple_matches);
    assert(many.hit_count == 2u && many.first_rva == 0x2011u);

    auto invalid = scan_optional_host_compat_text(nullptr, 0, 0);
    assert(invalid.status == OptionalHostCompatScanStatus::invalid_argument);
    return 0;
}
