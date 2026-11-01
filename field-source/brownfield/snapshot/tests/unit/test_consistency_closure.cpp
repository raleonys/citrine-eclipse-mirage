#include "preloader_l/runtime/consistency_closure.hpp"
#include "preloader_l/runtime/runtime_status.hpp"
using namespace preloader_l::runtime;
int main() {
    const auto c = active_consistency_closure_snapshot();
    const uint32_t historical =
        active_consistency_open_service_binary_coallocation_vftable_identity_7000 |
        active_consistency_open_native_crypt_query_tailcall_topology |
        active_consistency_open_packaging_mapper_4eba00 |
        active_consistency_open_d5f0_vector_crt_allocator_binding |
        active_consistency_open_ae00_aux_vector_crt_allocator_binding |
        active_consistency_open_optional_host_hit_installer_6b31 |
        active_consistency_open_optional_hook_installer_11a38 |
        active_consistency_open_optional_host_callback_1019d;
    if (c.historical_emitted_mask != historical) return 1;
    if (c.production_open_mask != 0u) return 2;
    if (c.closure_entry_count != 9u) return 3;
    if (c.observable_contract_closed_count != 5u) return 4;
    if (c.current_host_not_applicable_count != 3u) return 5;
    if (c.outside_target_count != 1u) return 6;
    if (active_runtime_snapshot().consistency_open_mask != 0u) return 7;
    if (active_runtime_snapshot().unresolved_mask != 0u) return 8;
    return 0;
}
