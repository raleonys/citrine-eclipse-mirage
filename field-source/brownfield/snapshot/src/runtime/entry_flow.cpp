#include "preloader_l/runtime/entry_flow.hpp"
namespace preloader_l { namespace runtime {
int run_curio_entry_flow(void* module, uint32_t reason, void* reserved,
                             EntryCallback active, EntryCallback secondary,
                             void* context) noexcept {
    if (!secondary || (reason == 1u && !active)) return 0;
    if (reason == 1u && !active(module, 1u, nullptr, context)) return 0;
    return secondary(module, reason, reserved, context);
}
} }
