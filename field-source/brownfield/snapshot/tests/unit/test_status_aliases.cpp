#include "preloader_l/runtime/runtime_status.hpp"
using namespace preloader_l::runtime;
static_assert(active_consistency_open_native_crypt_query_side_effects == active_consistency_open_native_crypt_query_tailcall_topology,
              "crypt compatibility alias drift");
static_assert(active_consistency_open_d5f0_vector_object_abi == active_consistency_open_d5f0_vector_crt_allocator_binding,
              "D5F0 compatibility alias drift");
static_assert(active_consistency_open_ae00_aux_vector_object_abi == active_consistency_open_ae00_aux_vector_crt_allocator_binding,
              "AE00 compatibility alias drift");
int main() {
    const auto s = active_runtime_snapshot();
    if (s.unresolved_mask != 0u) return 1;
    if (s.consistency_open_mask & active_consistency_open_native_crypt_query_tailcall_topology) return 2;
    if (s.consistency_open_mask & active_consistency_open_d5f0_vector_crt_allocator_binding) return 3;
    if (s.consistency_open_mask & active_consistency_open_ae00_aux_vector_crt_allocator_binding) return 4;
    if (s.consistency_open_mask != 0u) return 5;
    if (s.strict_model_consistency_open_mask != 0u) return 6;
    return 0;
}
