#include "preloader_l/debug/debug_api.hpp"
#include "preloader_l/runtime/loader_signals.hpp"
#include "preloader_l/runtime/runtime_state.hpp"

extern "C" uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_runtime_init() noexcept
{
    return static_cast<uint32_t>(preloader_l::runtime::try_initialize());
}

extern "C" uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_runtime_shutdown() noexcept
{
    return static_cast<uint32_t>(preloader_l::runtime::try_shutdown());
}

extern "C" uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_get_runtime_status(preloader_l_debug_runtime_status* out, size_t out_size) noexcept
{
    if (!out || out_size < sizeof(preloader_l_debug_runtime_status)) return 1u;
    const auto state = preloader_l::runtime::snapshot_state();
    const auto signals = preloader_l::runtime::snapshot_signals();
    out->phase = static_cast<uint32_t>(state.phase);
    out->generation = state.generation;
    out->module_base = static_cast<uint64_t>(state.module_base);
    out->initialize_count = state.initialize_count;
    out->shutdown_count = state.shutdown_count;
    out->process_attach_count = signals.process_attach_count;
    out->process_detach_count = signals.process_detach_count;
    out->surge_call_count = signals.surge_call_count;
    out->reserved = 0;
    return 0u;
}
