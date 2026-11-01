#include "preloader_l/debug/debug_diagnostics.hpp"

namespace preloader_l { namespace debug {
namespace {
DebugStats g_stats{};
}

void reset_stats() noexcept { g_stats = DebugStats{}; }
DebugStats snapshot_stats() noexcept { return g_stats; }
void note_self_test(bool passed) noexcept {
    ++g_stats.self_test_runs;
    if (!passed) ++g_stats.self_test_failures;
}
void note_dispatch(bool ok) noexcept {
    ++g_stats.dispatch_calls;
    if (ok) ++g_stats.dispatch_ok;
    else ++g_stats.dispatch_non_ok;
}
void note_codec_check() noexcept { ++g_stats.codec_checks; }
void note_framing_check() noexcept { ++g_stats.framing_checks; }
void note_route_check() noexcept { ++g_stats.route_checks; }

} }
