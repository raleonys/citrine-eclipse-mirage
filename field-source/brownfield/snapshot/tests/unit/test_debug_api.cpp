#include "preloader_l/debug/debug_api.hpp"
#include "preloader_l/debug/self_test.hpp"
#include "preloader_l/core/request_dispatch.hpp"
#include "preloader_l/runtime/loader_bridge.hpp"
#include "preloader_l/runtime/loader_signals.hpp"
#include "preloader_l/runtime/runtime_state.hpp"
#include <cstring>

int main() {
    preloader_l::runtime::reset_signals_for_test();
    preloader_l::runtime::reset_state_for_test();
    preloader_l::runtime::handle_loader_event(reinterpret_cast<void*>(static_cast<uintptr_t>(0x180000000ull)), preloader_l::runtime::loader_process_attach);
    if (preloader_l_debug_runtime_init() != static_cast<uint32_t>(preloader_l::runtime::InitResult::ok)) return 12;
    preloader_l_debug_reset_stats();
    uint32_t passed = 0, failed = ~0u;
    if (preloader_l_debug_self_test(&passed, &failed) != 0u) return 1;
    if (failed != 0u || passed != preloader_l::debug::self_test_all_mask) return 2;

    constexpr char request[] = "<LSX><Request id=\"33\"><GetConfig/></Request></LSX>";
    char output[4096]{};
    size_t written = 0;
    const auto status = preloader_l_debug_dispatch(request, sizeof(request) - 1, output, sizeof(output), &written);
    if (status != static_cast<uint32_t>(preloader_l::core::DispatchStatus::ok)) return 3;
    if (written == 0 || written != std::strlen(output)) return 4;
    if (!std::strstr(output, "<GetConfigResponse>")) return 5;

    constexpr char unknown[] = "<LSX><Request id=\"33\"><NoSuchCommand/></Request></LSX>";
    std::memset(output, '#', sizeof(output));
    written = 123;
    if (preloader_l_debug_dispatch(unknown, sizeof(unknown) - 1, output, sizeof(output), &written) != static_cast<uint32_t>(preloader_l::core::DispatchStatus::unknown_route)) return 6;
    if (written != 0) return 7;
    for (char c : output) if (c != '#') return 13;

    preloader_l_debug_stats stats{};
    if (preloader_l_debug_get_stats(&stats, sizeof(stats)) != 0u) return 8;
    if (stats.self_test_runs != 1 || stats.self_test_failures != 0) return 9;
    if (stats.dispatch_calls != 2 || stats.dispatch_ok != 1 || stats.dispatch_non_ok != 1) return 10;
    if (preloader_l_debug_get_stats(nullptr, sizeof(stats)) == 0u) return 11;
    return 0;
}
