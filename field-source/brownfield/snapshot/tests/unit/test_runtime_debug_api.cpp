#include "preloader_l/debug/debug_api.hpp"
#include "preloader_l/runtime/loader_bridge.hpp"
#include "preloader_l/runtime/loader_signals.hpp"
#include "preloader_l/runtime/runtime_state.hpp"
#include <cstdint>

int main() {
    using namespace preloader_l::runtime;
    reset_signals_for_test();
    reset_state_for_test();
    preloader_l_debug_runtime_status st{};
    if (preloader_l_debug_get_runtime_status(&st, sizeof(st)) != 0u || st.phase != static_cast<uint32_t>(Phase::cold)) return 1;
    if (preloader_l_debug_runtime_init() != static_cast<uint32_t>(InitResult::not_attached)) return 2;
    void* module = reinterpret_cast<void*>(static_cast<uintptr_t>(0x180000000ull));
    handle_loader_event(module, loader_process_attach);
    if (preloader_l_debug_get_runtime_status(&st, sizeof(st)) != 0u) return 3;
    if (st.phase != static_cast<uint32_t>(Phase::loader_attached) || st.process_attach_count != 1 || st.module_base != 0x180000000ull) return 4;
    if (preloader_l_debug_runtime_init() != static_cast<uint32_t>(InitResult::ok)) return 5;
    if (preloader_l_debug_get_runtime_status(&st, sizeof(st)) != 0u || st.phase != static_cast<uint32_t>(Phase::initialized) || st.initialize_count != 1) return 6;
    handle_surge_entry();
    if (preloader_l_debug_get_runtime_status(&st, sizeof(st)) != 0u || st.surge_call_count != 1) return 7;
    if (preloader_l_debug_runtime_shutdown() != static_cast<uint32_t>(ShutdownResult::ok)) return 8;
    if (preloader_l_debug_get_runtime_status(&st, sizeof(st)) != 0u || st.phase != static_cast<uint32_t>(Phase::shutdown) || st.shutdown_count != 1) return 9;
    handle_loader_event(module, loader_process_detach);
    if (preloader_l_debug_get_runtime_status(&st, sizeof(st)) != 0u || st.phase != static_cast<uint32_t>(Phase::detached) || st.process_detach_count != 1) return 10;
    return 0;
}
