#include "preloader_l/runtime/loader_bridge.hpp"
#if PRELOADER_L_TRACE_DIAGNOSTICS
#include "preloader_l/runtime/loader_signals.hpp"
#endif
#include "preloader_l/runtime/runtime_state.hpp"
#include "preloader_l/runtime/trace_macros.hpp"

namespace preloader_l { namespace runtime {

void handle_loader_event(void* module, uint32_t reason, void* reserved) noexcept {
    if (reason == loader_process_attach) {
#if PRELOADER_L_TRACE_DIAGNOSTICS
        note_process_attach();
#endif
        PRELOADER_L_TRACE_NOTE(TraceEvent::process_attach, 1);
        on_process_attach(module);
    } else if (reason == loader_process_detach) {
#if PRELOADER_L_TRACE_DIAGNOSTICS
        note_process_detach();
#endif
        on_process_detach(reserved ? DetachKind::process_termination : DetachKind::explicit_unload);
    }
}

void handle_surge_entry() noexcept {
#if PRELOADER_L_TRACE_DIAGNOSTICS
    note_surge_call();
#endif
}

} }
