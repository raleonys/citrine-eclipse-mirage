#include "preloader_l/debug/self_test.hpp"
#include "preloader_l/debug/debug_diagnostics.hpp"

int main() {
    preloader_l::debug::reset_stats();
    const auto r = preloader_l::debug::run_self_test();
    if (r.failed_mask != 0u || r.passed_mask != preloader_l::debug::self_test_all_mask) return 1;
    const auto s = preloader_l::debug::snapshot_stats();
    if (s.self_test_runs != 1 || s.self_test_failures != 0) return 2;
    if (s.codec_checks != 1 || s.framing_checks != 1 || s.route_checks != 1) return 3;
    return 0;
}
