#include "preloader_l/runtime/bootstrap_state.hpp"
#include "preloader_l/runtime/bootstrap_runner.hpp"
#include <cassert>

namespace {
struct Fake {
    int init;
    int start;
    int serve;
    int stop;
    int shutdown;
    bool fail_serve;
};
bool initialize(void* c) noexcept { ++static_cast<Fake*>(c)->init; return true; }
bool start(void* c, uint16_t* port) noexcept { ++static_cast<Fake*>(c)->start; *port=3227; return true; }
bool serve(void* c) noexcept { auto* f=static_cast<Fake*>(c); ++f->serve; return !f->fail_serve; }
void stop(void* c) noexcept { ++static_cast<Fake*>(c)->stop; }
bool shutdown(void* c) noexcept { ++static_cast<Fake*>(c)->shutdown; return true; }
}

int main()
{
    using namespace preloader_l::runtime;
    Fake ok{};
    BootstrapBackend backend{&ok, initialize, start, serve, stop, shutdown};
    reset_bootstrap_for_test();
    assert(mark_bootstrap_scheduled());
    assert(run_one_shot_bootstrap(backend)==BootstrapRunResult::ok);
    auto s=snapshot_bootstrap();
    assert(s.phase==BootstrapPhase::completed && s.bound_port==3227 && s.client_count==1);
    assert(ok.init==1 && ok.start==1 && ok.serve==1 && ok.stop==1 && ok.shutdown==1);

    Fake fail{}; fail.fail_serve=true;
    backend.context=&fail;
    reset_bootstrap_for_test();
    assert(mark_bootstrap_scheduled());
    assert(run_one_shot_bootstrap(backend)==BootstrapRunResult::serve_failed);
    s=snapshot_bootstrap();
    assert(s.phase==BootstrapPhase::failed && s.failure_code==static_cast<uint32_t>(BootstrapRunResult::serve_failed));
    assert(fail.stop==1 && fail.shutdown==1);
}
