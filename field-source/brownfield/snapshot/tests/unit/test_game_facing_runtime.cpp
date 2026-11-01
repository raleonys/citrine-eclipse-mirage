#include "preloader_l/debug/debug_api.hpp"
#include "preloader_l/core/request_dispatch.hpp"
#include "preloader_l/runtime/loader_bridge.hpp"
#include "preloader_l/runtime/loader_signals.hpp"
#include "preloader_l/runtime/runtime_state.hpp"
#include <cstdint>
#include <cstring>

int main() {
    using namespace preloader_l::runtime;
    reset_signals_for_test(); reset_state_for_test(); preloader_l_debug_reset_stats();
    void* module = reinterpret_cast<void*>(static_cast<uintptr_t>(0x7ff700000000ull));
    handle_loader_event(module, loader_process_attach);
    preloader_l_debug_runtime_status rs{};
    if (preloader_l_debug_get_runtime_status(&rs, sizeof(rs)) != 0u || rs.phase != static_cast<uint32_t>(Phase::loader_attached) || rs.process_attach_count != 1u) return 1;
    constexpr char request[] = "<LSX><Request id=\"40\"><GetConfig/></Request></LSX>";
    char output[4096]{}; size_t written=9;
    if (preloader_l_debug_dispatch(request, sizeof(request)-1, output, sizeof(output), &written) != PRELOADER_L_DEBUG_RUNTIME_NOT_INITIALIZED || written != 0u) return 2;
    if (preloader_l_debug_runtime_init() != static_cast<uint32_t>(InitResult::ok)) return 3;
    if (preloader_l_debug_self_test(nullptr, nullptr) != 0u) return 4;
    if (preloader_l_debug_dispatch(request, sizeof(request)-1, output, sizeof(output), &written) != static_cast<uint32_t>(preloader_l::core::DispatchStatus::ok)) return 5;
    if (!std::strstr(output, "<GetConfigResponse>")) return 6;
    handle_surge_entry();
    if (preloader_l_debug_get_runtime_status(&rs, sizeof(rs)) != 0u || rs.phase != static_cast<uint32_t>(Phase::initialized) || rs.surge_call_count != 1u) return 7;
    if (preloader_l_debug_runtime_shutdown() != static_cast<uint32_t>(ShutdownResult::ok)) return 8;
    handle_loader_event(module, loader_process_detach);
    if (preloader_l_debug_get_runtime_status(&rs, sizeof(rs)) != 0u || rs.phase != static_cast<uint32_t>(Phase::detached) || rs.process_detach_count != 1u) return 9;
    return 0;
}
