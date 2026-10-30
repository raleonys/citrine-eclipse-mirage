#include "preloader_l/platform/hook_cleanup.hpp"
namespace preloader_l::network_hook_recovered {
void cleanup_hooks_0102b0(std::span<const std::uintptr_t> targets, CleanupEffects& e) {
    for (const auto& target : targets) {
        if (e.disable_hook_046a00(target) != 0) {
            e.log_literal_006880("disabling hook failed");
            e.log_finish_0088e0();
        }
    }
    e.log_literal_006880("MinHook uninitialization ");
    const auto status = e.uninitialize_engine_046ac0();
    e.log_literal_006880(status == 0 ? "succeeded" : "failed");
    e.log_finish_0088e0();
}
}
