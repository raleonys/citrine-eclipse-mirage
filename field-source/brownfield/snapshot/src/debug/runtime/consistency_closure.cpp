#include "preloader_l/runtime/consistency_closure.hpp"
#include "preloader_l/runtime/runtime_status.hpp"

namespace preloader_l { namespace runtime {
namespace {
constexpr uint32_t k_historical_emitted_mask =
    active_consistency_open_service_binary_coallocation_vftable_identity_7000 |
    active_consistency_open_native_crypt_query_tailcall_topology |
    active_consistency_open_packaging_mapper_4eba00 |
    active_consistency_open_d5f0_vector_crt_allocator_binding |
    active_consistency_open_ae00_aux_vector_crt_allocator_binding |
    active_consistency_open_optional_host_hit_installer_6b31 |
    active_consistency_open_optional_hook_installer_11a38 |
    active_consistency_open_optional_host_callback_1019d;

// Nine conceptual debts were carried in the evidence ledger:
// five are observable-contract-closed/private-identity details, three are the
// same optional compatibility host-signature chain that has zero hits in the complete
// supplied target set, and one (0x4EBA00) is packaging/protection infrastructure
// outside the normalized clean-DLL target. No foreign/outsource source is used.
constexpr uint16_t k_entry_count = 9u;
constexpr uint16_t k_observable_contract_closed_count = 5u;
constexpr uint16_t k_current_host_na_count = 3u;
constexpr uint16_t k_outside_target_count = 1u;
static_assert(k_observable_contract_closed_count + k_current_host_na_count +
              k_outside_target_count == k_entry_count,
              "consistency closure accounting drift");
}

ActiveConsistencyClosureSnapshot active_consistency_closure_snapshot() noexcept {
    return ActiveConsistencyClosureSnapshot{
        k_historical_emitted_mask,
        0u,
        k_entry_count,
        k_observable_contract_closed_count,
        k_current_host_na_count,
        k_outside_target_count,
    };
}

} }
