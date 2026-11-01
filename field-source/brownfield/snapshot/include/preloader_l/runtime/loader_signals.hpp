#pragma once
#include <stdint.h>

namespace preloader_l { namespace runtime {

enum class LoaderEvent : uint32_t {
    none = 0,
    process_attach = 1,
    process_detach = 2,
};

struct SignalSnapshot {
    uint32_t process_attach_count;
    uint32_t process_detach_count;
    uint32_t surge_call_count;
    LoaderEvent last_loader_event;
};

// Loader-safe telemetry primitives. These functions perform no allocation,
// I/O, locking, thread creation, registry access, networking, or CRT work.
void reset_signals_for_test() noexcept;
void note_process_attach() noexcept;
void note_process_detach() noexcept;
void note_surge_call() noexcept;
SignalSnapshot snapshot_signals() noexcept;

} }
