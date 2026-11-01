#pragma once
#include <stdint.h>
#include "preloader_l/runtime/runtime_status.hpp"
#include "strict_consistency_ledger.hpp"

namespace preloader_l { namespace runtime {

// Current-host production readiness is tracked separately from strict/model
// consistency debt. The maturity model includes evidence-backed live source
// bindings without treating that as native/binary identity closure. A zero
// current-host blocker mask must never be interpreted as strict consistency.
struct ActiveStrictConsistencyStatusSnapshot {
    uint32_t strict_open_mask;
    uint32_t current_host_blocker_mask;
    uint32_t source_model_ready_mask;
    uint32_t live_source_binding_mask;
    uint32_t behavioral_binary_open_mask;
    uint32_t compatibility_current_host_na_mask;
    uint32_t packaging_open_mask;
    uint32_t provenance_only_mask;
    uint16_t conceptual_open_count;
    uint16_t source_model_ready_count;
    uint16_t live_source_binding_count;
    uint16_t behavioral_binary_open_count;
    uint16_t compatibility_current_host_na_count;
    uint16_t packaging_open_count;
    uint16_t provenance_only_count;
};

inline ActiveStrictConsistencyStatusSnapshot active_strict_consistency_status_snapshot() noexcept {
    // 2.8.4 active ledger: 11A38, 7000 and shared-hook 0x5 are closed by
    // accepted native evidence. The historical shared-hook bit positions remain
    // useful for trace/source compatibility but are no longer open debt.
    constexpr uint32_t shared_hook_historical =
        active_consistency_open_registry_hook_allocator_13a70 |
        active_consistency_open_htons_hook_allocator_12980;
    static_assert(shared_hook_historical == strict_ledger::closed_shared_hook_mask,
                  "shared-hook historical bit drift");

    constexpr uint32_t provenance_only =
        active_consistency_open_packaging_mapper_4eba00;

    return ActiveStrictConsistencyStatusSnapshot{
        strict_ledger::strict_open_mask,
        0u,
        0u,
        0u,
        strict_ledger::open_shared_hook_mask,
        0u,
        0u,
        provenance_only,
        strict_ledger::conceptual_open_count,
        0u,
        0u,
        0u,
        0u,
        0u,
        1u,
    };
}

} }
