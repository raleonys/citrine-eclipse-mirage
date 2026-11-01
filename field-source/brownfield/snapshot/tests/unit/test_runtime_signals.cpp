#include "preloader_l/runtime/loader_signals.hpp"
#include <cstdlib>

int main() {
    using namespace preloader_l::runtime;
    reset_signals_for_test();
    auto s = snapshot_signals();
    if (s.process_attach_count || s.process_detach_count || s.surge_call_count || s.last_loader_event != LoaderEvent::none) return 1;
    note_process_attach();
    note_surge_call();
    note_surge_call();
    s = snapshot_signals();
    if (s.process_attach_count != 1 || s.process_detach_count != 0 || s.surge_call_count != 2 || s.last_loader_event != LoaderEvent::process_attach) return 2;
    note_process_detach();
    s = snapshot_signals();
    if (s.process_detach_count != 1 || s.last_loader_event != LoaderEvent::process_detach) return 3;
    return 0;
}
