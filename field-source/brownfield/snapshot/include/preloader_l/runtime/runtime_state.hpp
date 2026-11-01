#pragma once
#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace runtime {

enum class Phase : uint32_t {
    cold = 0,
    loader_attached = 1,
    initialized = 2,
    shutdown = 3,
    detached = 4,
};

enum class InitResult : uint32_t {
    ok = 0,
    already_initialized = 1,
    not_attached = 2,
    already_shutdown = 3,
    detached = 4,
};

enum class ShutdownResult : uint32_t {
    ok = 0,
    not_initialized = 1,
    already_shutdown = 2,
    detached = 3,
};

enum class DetachKind : uint32_t {
    none = 0,
    explicit_unload = 1,
    process_termination = 2,
};

struct StateSnapshot {
    Phase phase;
    uintptr_t module_base;
    uint32_t generation;
    uint32_t initialize_count;
    uint32_t shutdown_count;
    DetachKind detach_kind;
};

void reset_state_for_test() noexcept;
void on_process_attach(void* module) noexcept;
void on_process_detach(DetachKind kind = DetachKind::explicit_unload) noexcept;
InitResult try_initialize() noexcept;
ShutdownResult try_shutdown() noexcept;
StateSnapshot snapshot_state() noexcept;

} }
