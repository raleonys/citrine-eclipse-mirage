#include "preloader_l/runtime/bootstrap_state.hpp"
#include <cassert>

int main()
{
    using namespace preloader_l::runtime;
    reset_bootstrap_for_test();
    auto s = snapshot_bootstrap();
    assert(s.phase == BootstrapPhase::cold && s.bound_port == 0);
    assert(mark_bootstrap_scheduled());
    assert(!mark_bootstrap_scheduled());
    assert(mark_bootstrap_running());
    assert(mark_bootstrap_runtime_initialized());
    assert(!mark_bootstrap_listener_active(0));
    assert(mark_bootstrap_listener_active(3226));
    assert(mark_bootstrap_client_served());
    assert(mark_bootstrap_completed());
    s = snapshot_bootstrap();
    assert(s.phase == BootstrapPhase::completed);
    assert(s.bound_port == 3226 && s.schedule_count == 1 && s.run_count == 1 && s.client_count == 1);

    reset_bootstrap_for_test();
    assert(mark_bootstrap_scheduled());
    assert(mark_bootstrap_cancelled_before_run());
    s = snapshot_bootstrap();
    assert(s.phase == BootstrapPhase::completed && s.run_count == 0);

    reset_bootstrap_for_test();
    assert(mark_bootstrap_scheduled());
    assert(mark_bootstrap_failed_if_unreported(66));
    assert(!mark_bootstrap_failed_if_unreported(77));
    s = snapshot_bootstrap();
    assert(s.phase == BootstrapPhase::failed && s.failure_code == 66);

    reset_bootstrap_for_test();
    assert(mark_bootstrap_scheduled());
    mark_bootstrap_failed(77);
    assert(!mark_bootstrap_failed_if_unreported(88));
    s = snapshot_bootstrap();
    assert(s.phase == BootstrapPhase::failed && s.failure_code == 77);
}
