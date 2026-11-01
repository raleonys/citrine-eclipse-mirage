#include "preloader_l/runtime/runtime_status.hpp"
using namespace preloader_l::runtime;
static_assert(active_consistency_open_service_object_abi_7000 == (1u << 1), "7000 ABI bit drift");
static_assert(active_consistency_open_service_object_layout_7000 == active_consistency_open_service_object_abi_7000,
              "7000 compatibility alias drift");
int main() {
    const auto s = active_runtime_snapshot();
    if (s.unresolved_mask != 0u) return 1;
    if (!(s.curio_mask & active_curio_service_stage_7000)) return 2;
    if (!(s.integrated_mask & active_integrated_service_stage_7000)) return 3;
    if (!(s.runtime_active_mask & active_runtime_active_service_stage_7000)) return 4;
    if (s.consistency_open_mask & active_consistency_open_service_object_abi_7000) return 5;
    if (s.strict_model_consistency_open_mask != 0u) return 6;
    return 0;
}
