#pragma once
#include <stdint.h>

namespace preloader_l { namespace runtime { namespace strict_ledger {

// 2.8.4 single source of truth for active strict/model accounting.
// Closure evidence is external/native; these constants record accepted state
// but do not themselves constitute closure evidence.
constexpr uint32_t closed_11a38_mask = 0x00000400u;
constexpr uint32_t closed_7000_mask = 0x00000002u;
constexpr uint32_t closed_shared_hook_mask = 0x00000005u;
constexpr uint32_t open_shared_hook_mask = 0x00000000u;
constexpr uint32_t strict_open_mask = 0x00000000u;
constexpr uint16_t conceptual_open_count = 0u;
constexpr uint32_t closed_native_runtime_mask =
    closed_11a38_mask | closed_7000_mask | closed_shared_hook_mask;

} } }
