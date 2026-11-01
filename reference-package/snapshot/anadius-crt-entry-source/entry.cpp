#include "entry.hpp"
#include <bit>
#include <limits>

namespace anadius::crt_entry_recovered {
namespace {
bool al(std::uint64_t value) { return (value & 0xffu) != 0; }
}
bool Entry::attach_count_positive() const noexcept {
    return std::bit_cast<std::int32_t>(state_.attach_count_bits) > 0;
}
bool Entry::initialize_crt_04b584(std::uint32_t module_kind) {
    if (module_kind == 0) state_.initialized_as_dll = 1;
    effects_.initialize_cpu_04b26c();
    if (!al(effects_.vcrt_initialize_04d55c())) return false;
    if (al(effects_.acrt_initialize_064594())) return true;
    (void)effects_.vcrt_uninitialize_04d5a8(false);
    return false;
}
bool Entry::uninitialize_crt_04b708(bool terminating, bool from_exit) {
    if (state_.initialized_as_dll == 0 || !from_exit) {
        effects_.acrt_uninitialize_0645cc(terminating);
        (void)effects_.vcrt_uninitialize_04d5a8(terminating);
    }
    return true;
}
bool Entry::initialize_onexit_tables_04b5c0(std::uint32_t module_kind) {
    if (state_.onexit_initialized == 0) {
        if (module_kind > 1) effects_.fastfail_04bfec(5);
        constexpr auto sentinel = std::numeric_limits<std::uintptr_t>::max();
        state_.onexit_table = {sentinel, sentinel, sentinel};
        state_.quickexit_table = {sentinel, sentinel, sentinel};
        state_.onexit_initialized = 1;
    }
    return true;
}
bool Entry::initialize_onexit_tables_04b488() {
    return initialize_onexit_tables_04b5c0(0);
}
bool Entry::thread_attach_04b4a0() {
    if (!al(effects_.vcrt_thread_attach_04d584())) return false;
    if (al(effects_.acrt_thread_attach_0645a8())) return true;
    effects_.vcrt_thread_detach_04d598();
    return false;
}
bool Entry::thread_detach_04b4c8() {
    effects_.acrt_thread_detach_0645bc();
    effects_.vcrt_thread_detach_04d598();
    return true;
}
bool Entry::after_initialize_c_04b454() {
    // 03af80 returns EAX=1. The _configure_narrow_argv result is tested as EAX.
    if (effects_.configure_narrow_argv_0638e8(1) != 0) return false;
    effects_.initialize_narrow_environment_064054();
    return true;
}
void Entry::uninitialize_c_04b540() {
    if (effects_.c_termination_complete_05cdb8() == 0) effects_.cexit_05cd9c();
}
void Entry::late_cleanup_04b570() {
    effects_.acrt_late_cleanup_064604(false); // ECX explicitly zeroed
    effects_.vcrt_late_cleanup_04d5c4();
}
bool Entry::process_attach_04ba30(std::uintptr_t module, std::uintptr_t reserved) {
    if (!initialize_crt_04b584(0)) return false;
    const bool nested = acquire_startup_lock_04b418();
    if (state_.startup_state != 0) effects_.fastfail_04bfec(7);
    state_.startup_state = 1;
    bool failed = true;
    if (initialize_onexit_tables_04b488()) {
        // 04c220 walks an empty range in this target: no callback is invoked.
        effects_.initialize_slist_04c1e0();
        effects_.configure_stdio_04c1fc();
        if (effects_.initterm_e_064658() == 0 && after_initialize_c_04b454()) {
            effects_.initterm_064614();
            state_.startup_state = 2;
            failed = false;
        }
    }
    release_startup_lock_04b6e4(nested);
    if (failed) return false; // No normal-path cleanup is invented here.
    const auto* slot = effects_.tls_callback_slot_04c218();
    if (*slot != 0 && al(effects_.slot_in_nonwritable_image_04b64c(slot))) {
        effects_.invoke_tls_callback(*slot, module, 2, reserved);
    }
    ++state_.attach_count_bits; // exact unsigned modulo-2^32 INC
    return true;
}
bool Entry::process_detach_04bb48(bool terminating) {
    if (!attach_count_positive()) return false;
    --state_.attach_count_bits; // Original decrements before state validation.
    const bool nested = acquire_startup_lock_04b418();
    if (state_.startup_state != 2) effects_.fastfail_04bfec(7);
    uninitialize_c_04b540();
    effects_.release_slist_04c1f0();
    // 04c25c also walks an empty range in this target.
    state_.startup_state = 0;
    release_startup_lock_04b6e4(nested);
    const bool result = uninitialize_crt_04b708(terminating, false);
    late_cleanup_04b570();
    return result;
}
std::uint32_t Entry::crt_reason_04b9e0(std::uintptr_t module,
    std::uint32_t reason, std::uintptr_t reserved) {
    switch (reason) {
    case 0: return process_detach_04bb48(reserved != 0);
    case 1: return process_attach_04ba30(module, reserved);
    case 2: return thread_attach_04b4a0();
    case 3: return thread_detach_04b4c8();
    default: return 1;
    }
}
std::uint32_t Entry::dispatch_04bbc8(std::uintptr_t module,
    std::uint32_t reason, std::uintptr_t reserved) {
    if (reason == 0 && !attach_count_positive()) return 0;
    // Unsigned EDX-1 comparison, including unknown and wrapping reasons.
    if (reason - 1u < 2u && crt_reason_04b9e0(module, reason, reserved) == 0) return 0;
    const std::uint32_t app_result =
        effects_.application_dll_main_038870(module, reason, reserved);
    if (reason == 1 && app_result == 0) {
        (void)effects_.application_dll_main_038870(module, 0, reserved);
        (void)process_detach_04bb48(reserved != 0);
    }
    // The target's raw DllMain hook at 180088040 is null. Detach replaces
    // the application's return with normalized CRT cleanup success.
    if (reason == 0 || reason == 3)
        return crt_reason_04b9e0(module, reason, reserved) != 0 ? 1u : 0u;
    return app_result;
}
std::uint32_t Entry::entry_04bcfc(std::uintptr_t module,
    std::uint32_t reason, std::uintptr_t reserved) {
    if (reason == 1) effects_.security_init_cookie_04c134();
    return dispatch_04bbc8(module, reason, reserved);
}
}
