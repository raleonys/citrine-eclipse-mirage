#include "preloader_l/runtime/bootstrap_state.hpp"
#include "preloader_l/runtime/bootstrap_runner.hpp"

namespace {
struct Fake {
    unsigned init = 0;
    unsigned start = 0;
    unsigned serve = 0;
    unsigned stop = 0;
    unsigned shutdown = 0;
    bool start_ok = true;
    bool serve_ok = true;
    bool shutdown_ok = true;
};
bool initialize(void* c) noexcept { ++static_cast<Fake*>(c)->init; return true; }
bool start(void* c, uint16_t* port) noexcept {
    auto* f = static_cast<Fake*>(c);
    ++f->start;
    if (port) *port = f->start_ok ? 3226u : 0u;
    return f->start_ok;
}
bool serve(void* c) noexcept { auto* f = static_cast<Fake*>(c); ++f->serve; return f->serve_ok; }
void stop(void* c) noexcept { ++static_cast<Fake*>(c)->stop; }
bool shutdown(void* c) noexcept { auto* f = static_cast<Fake*>(c); ++f->shutdown; return f->shutdown_ok; }
}

int main() {
    using namespace preloader_l::runtime;

    // Listener failure must not hide a failed enclosing shutdown.
    Fake listener_fail{};
    listener_fail.start_ok = false;
    listener_fail.shutdown_ok = false;
    BootstrapBackend backend{&listener_fail, initialize, start, serve, stop, shutdown, false};
    reset_bootstrap_for_test();
    clear_bootstrap_stop_request();
    if (!mark_bootstrap_scheduled()) return 1;
    if (run_one_shot_bootstrap(backend) != BootstrapRunResult::shutdown_failed) return 2;
    if (listener_fail.stop != 1 || listener_fail.shutdown != 1) return 3;
    auto snapshot = snapshot_bootstrap();
    if (snapshot.failure_code != static_cast<uint32_t>(BootstrapRunResult::shutdown_failed)) return 4;

    // Persistent serve failure follows the same fail-closed boundary.
    Fake serve_fail{};
    serve_fail.serve_ok = false;
    serve_fail.shutdown_ok = false;
    backend.context = &serve_fail;
    reset_bootstrap_for_test();
    clear_bootstrap_stop_request();
    if (!mark_bootstrap_scheduled()) return 5;
    if (run_persistent_bootstrap(backend) != BootstrapRunResult::shutdown_failed) return 6;
    if (serve_fail.stop != 1 || serve_fail.shutdown != 1) return 7;
    snapshot = snapshot_bootstrap();
    if (snapshot.failure_code != static_cast<uint32_t>(BootstrapRunResult::shutdown_failed)) return 8;
    return 0;
}
