#include "preloader_l/debug/debug_diagnostics.hpp"
#include <stdint.h>

int main() {
    using namespace preloader_l::debug;
    reset_stats();
    auto s = snapshot_stats();
    if (s.self_test_runs || s.dispatch_calls || s.codec_checks || s.framing_checks || s.route_checks) return 1;
    note_self_test(true);
    note_self_test(false);
    note_dispatch(true);
    note_dispatch(false);
    note_codec_check();
    note_framing_check();
    note_route_check();
    s = snapshot_stats();
    if (s.self_test_runs != 2 || s.self_test_failures != 1) return 2;
    if (s.dispatch_calls != 2 || s.dispatch_ok != 1 || s.dispatch_non_ok != 1) return 3;
    if (s.codec_checks != 1 || s.framing_checks != 1 || s.route_checks != 1) return 4;
    return 0;
}
