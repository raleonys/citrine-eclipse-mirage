#include "preloader_l/runtime/loader_bridge.hpp"
#include "preloader_l/runtime/loader_signals.hpp"
#include "preloader_l/runtime/runtime_state.hpp"
#include <cstdint>

int main() {
    using namespace preloader_l::runtime;
    reset_signals_for_test();
    reset_state_for_test();
    void* module = reinterpret_cast<void*>(static_cast<uintptr_t>(0x7ff700000000ull));
    handle_loader_event(module, loader_thread_attach);
    if (snapshot_signals().process_attach_count != 0 || snapshot_state().phase != Phase::cold) return 1;
    handle_loader_event(module, loader_process_attach);
    auto sig = snapshot_signals(); auto st = snapshot_state();
    if (sig.process_attach_count != 1 || st.phase != Phase::loader_attached || st.module_base != reinterpret_cast<uintptr_t>(module)) return 2;
    handle_surge_entry(); handle_surge_entry();
    if (snapshot_signals().surge_call_count != 2) return 3;
    handle_loader_event(module, loader_thread_detach);
    if (snapshot_signals().process_detach_count != 0 || snapshot_state().phase != Phase::loader_attached) return 4;
    handle_loader_event(module, loader_process_detach, nullptr);
    sig = snapshot_signals(); st = snapshot_state();
    if (sig.process_detach_count != 1 || st.phase != Phase::detached || st.module_base != 0 || st.detach_kind != DetachKind::explicit_unload) return 5;
    reset_signals_for_test(); reset_state_for_test();
    handle_loader_event(module, loader_process_attach);
    handle_loader_event(module, loader_process_detach, reinterpret_cast<void*>(1));
    st = snapshot_state();
    if (st.detach_kind != DetachKind::process_termination) return 6;
    return 0;
}
