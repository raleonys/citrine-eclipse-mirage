#pragma once
#include <stdint.h>

namespace preloader_l { namespace runtime {

enum class BootstrapRunResult : uint32_t {
    ok = 0,
    invalid_argument = 1,
    invalid_state = 2,
    initialize_failed = 3,
    listener_failed = 4,
    serve_failed = 5,
    shutdown_failed = 6,
};

using BootstrapInitializeFn = bool (*)(void* context) noexcept;
using BootstrapStartListenerFn = bool (*)(void* context, uint16_t* bound_port) noexcept;
using BootstrapServeOneFn = bool (*)(void* context) noexcept;
using BootstrapStopListenerFn = void (*)(void* context) noexcept;
using BootstrapShutdownFn = bool (*)(void* context) noexcept;

struct BootstrapBackend {
    void* context;
    BootstrapInitializeFn initialize;
    BootstrapStartListenerFn start_listener;
    BootstrapServeOneFn serve_one;
    BootstrapStopListenerFn stop_listener;
    BootstrapShutdownFn shutdown;
    // Native concurrent release backend counts clients at the real accept boundary.
    // Serial/test backends leave this false so the runner accounts one serve call.
    bool backend_counts_clients;
};

void clear_bootstrap_stop_request() noexcept;
void request_bootstrap_stop() noexcept;
bool bootstrap_stop_requested() noexcept;

bool valid_bootstrap_backend(const BootstrapBackend& backend) noexcept;
BootstrapRunResult run_one_shot_bootstrap(const BootstrapBackend& backend) noexcept;
BootstrapRunResult run_persistent_bootstrap(const BootstrapBackend& backend) noexcept;

} }
