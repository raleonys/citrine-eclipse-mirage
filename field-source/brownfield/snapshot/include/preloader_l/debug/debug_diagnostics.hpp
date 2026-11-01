#pragma once
#include <stdint.h>

namespace preloader_l { namespace debug {

struct DebugStats {
    uint64_t self_test_runs;
    uint64_t self_test_failures;
    uint64_t dispatch_calls;
    uint64_t dispatch_ok;
    uint64_t dispatch_non_ok;
    uint64_t codec_checks;
    uint64_t framing_checks;
    uint64_t route_checks;
};

// Debug-prototype telemetry only. Deliberately process-local, allocation-free,
// and not part of the Release ABI. The first prototype host is single-threaded;
// concurrent increments are therefore intentionally outside this stage's contract.
void reset_stats() noexcept;
DebugStats snapshot_stats() noexcept;
void note_self_test(bool passed) noexcept;
void note_dispatch(bool ok) noexcept;
void note_codec_check() noexcept;
void note_framing_check() noexcept;
void note_route_check() noexcept;

} }
