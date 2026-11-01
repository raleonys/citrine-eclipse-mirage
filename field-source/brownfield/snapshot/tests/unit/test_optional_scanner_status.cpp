#include "preloader_l/runtime/runtime_status.hpp"
#include <assert.h>
#include <stdint.h>

int main() {
    using namespace preloader_l::runtime;
    const auto s = active_runtime_snapshot();

    assert(s.unresolved_mask == 0u);
    assert((s.curio_mask & active_curio_optional_host_hook_6b31) != 0u);
    assert((s.integrated_mask & active_integrated_optional_host_scanner_6b31) == 0u);
    assert((s.runtime_active_mask & active_runtime_active_optional_host_scanner_6b31) == 0u);

    // Historical identifiers remain source-compatible but current production
    // has no consistency-open requirement for this absent compatibility host-signature chain.
    assert((s.consistency_open_mask & active_consistency_open_optional_host_hit_installer_6b31) == 0u);
    assert((s.consistency_open_mask & active_consistency_open_optional_hook_installer_11a38) == 0u);
    assert((s.consistency_open_mask & active_consistency_open_optional_host_callback_1019d) == 0u);
    assert(s.strict_model_consistency_open_mask == 0u);

    static_assert(active_consistency_open_optional_host_hook_engine_6b31 == (1u << 9),
                  "6B31 consistency bit drift");
    return 0;
}
