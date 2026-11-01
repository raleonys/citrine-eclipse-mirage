#include "preloader_l/runtime/runtime_state.hpp"
#include <cstdint>

int main() {
    using namespace preloader_l::runtime;
    reset_state_for_test();
    auto s = snapshot_state();
    if (s.phase != Phase::cold || s.module_base != 0 || s.generation != 0 || s.detach_kind != DetachKind::none) return 1;
    if (try_initialize() != InitResult::not_attached) return 2;
    void* module = reinterpret_cast<void*>(static_cast<uintptr_t>(0x180000000ull));
    on_process_attach(module);
    s = snapshot_state();
    if (s.phase != Phase::loader_attached || s.module_base != 0x180000000ull || s.generation != 1 || s.detach_kind != DetachKind::none) return 3;
    if (try_initialize() != InitResult::ok) return 4;
    if (try_initialize() != InitResult::already_initialized) return 5;
    s = snapshot_state();
    if (s.phase != Phase::initialized || s.initialize_count != 1) return 6;
    if (try_shutdown() != ShutdownResult::ok) return 7;
    if (try_shutdown() != ShutdownResult::already_shutdown) return 8;
    s = snapshot_state();
    if (s.phase != Phase::shutdown || s.shutdown_count != 1) return 9;
    if (try_initialize() != InitResult::already_shutdown) return 10;
    on_process_detach(DetachKind::explicit_unload);
    s = snapshot_state();
    if (s.phase != Phase::detached || s.module_base != 0 || s.detach_kind != DetachKind::explicit_unload) return 11;
    if (try_initialize() != InitResult::detached || try_shutdown() != ShutdownResult::detached) return 12;
    reset_state_for_test();
    on_process_attach(module);
    on_process_detach(DetachKind::process_termination);
    s = snapshot_state();
    if (s.detach_kind != DetachKind::process_termination || s.phase != Phase::detached) return 13;
    return 0;
}
