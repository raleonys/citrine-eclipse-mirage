#include "preloader_l/debug/debug_api.hpp"
#include "preloader_l/debug/debug_diagnostics.hpp"
#include "preloader_l/debug/self_test.hpp"
#include "preloader_l/core/request_dispatch.hpp"
#include "preloader_l/runtime/runtime_state.hpp"

extern "C" uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_self_test(uint32_t* passed_mask, uint32_t* failed_mask) noexcept
{
    const auto report = preloader_l::debug::run_self_test();
    if (passed_mask) *passed_mask = report.passed_mask;
    if (failed_mask) *failed_mask = report.failed_mask;
    return report.failed_mask;
}

extern "C" uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_dispatch(const char* request, size_t request_length,
                                                       char* response, size_t response_capacity,
                                                       size_t* written) noexcept
{
    if (written) *written = 0;
    if (preloader_l::runtime::snapshot_state().phase != preloader_l::runtime::Phase::initialized) {
        return PRELOADER_L_DEBUG_RUNTIME_NOT_INITIALIZED;
    }
    const auto result = preloader_l::core::dispatch_plaintext_request(request, request_length, response, response_capacity);
    const bool ok = result.status == preloader_l::core::DispatchStatus::ok;
    preloader_l::debug::note_dispatch(ok);
    if (written) *written = result.written;
    return static_cast<uint32_t>(result.status);
}

extern "C" void PRELOADER_L_DEBUG_CALL preloader_l_debug_reset_stats() noexcept
{
    preloader_l::debug::reset_stats();
}

extern "C" uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_get_stats(preloader_l_debug_stats* out, size_t out_size) noexcept
{
    if (!out || out_size < sizeof(preloader_l_debug_stats)) return 1u;
    const auto s = preloader_l::debug::snapshot_stats();
    out->self_test_runs = s.self_test_runs;
    out->self_test_failures = s.self_test_failures;
    out->dispatch_calls = s.dispatch_calls;
    out->dispatch_ok = s.dispatch_ok;
    out->dispatch_non_ok = s.dispatch_non_ok;
    out->codec_checks = s.codec_checks;
    out->framing_checks = s.framing_checks;
    out->route_checks = s.route_checks;
    return 0u;
}
