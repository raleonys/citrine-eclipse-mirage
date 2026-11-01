#include "preloader_l/runtime/runtime_status.hpp"
using namespace preloader_l::runtime;
static_assert(active_consistency_open_registry_hook_relocation_ip_allocator_13a70 ==
              active_consistency_open_registry_hook_thread_trap_allocator_13a70,
              "13A70 compatibility alias drift");
static_assert(active_consistency_open_htons_hook_relocation_ip_allocator_12980 ==
              active_consistency_open_htons_hook_thread_trap_allocator_12980,
              "12980 compatibility alias drift");
int main() {
    const auto s = active_runtime_snapshot();
    if (s.unresolved_mask != 0u) return 1;
    if (s.consistency_open_mask & active_consistency_open_registry_hook_thread_trap_allocator_13a70) return 2;
    if (s.consistency_open_mask & active_consistency_open_htons_hook_thread_trap_allocator_12980) return 3;
    if (s.strict_model_consistency_open_mask != 0u) return 6;
    return 0;
}
