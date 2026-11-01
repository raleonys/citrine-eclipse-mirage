#include "preloader_l/runtime/runtime_status.hpp"
#include <cassert>
#include <cstdint>
#include <iostream>

int main() {
    using namespace preloader_l::runtime;
    const auto s = active_runtime_snapshot();
    assert(s.unresolved_mask == 0u);

    const uint32_t historical_allocator_debt =
        active_consistency_open_registry_hook_allocator_13a70 |
        active_consistency_open_htons_hook_allocator_12980;
    const uint32_t historical_other_debt =
        active_consistency_open_service_object_abi_7000 |
        active_consistency_open_native_crypt_query_tailcall_topology |
        active_consistency_open_packaging_mapper_4eba00 |
        active_consistency_open_d5f0_vector_crt_allocator_binding |
        active_consistency_open_ae00_aux_vector_crt_allocator_binding |
        active_consistency_open_optional_host_hook_engine_6b31 |
        active_consistency_open_optional_hook_installer_11a38 |
        active_consistency_open_optional_host_callback_1019d;
    assert((s.consistency_open_mask & historical_allocator_debt) == 0u);
    assert((s.consistency_open_mask & historical_other_debt) == 0u);
    assert(s.consistency_open_mask == 0u);
    assert(s.strict_model_consistency_open_mask == 0u);

    std::cout << "active state allocator closure: PASS\n";
}
