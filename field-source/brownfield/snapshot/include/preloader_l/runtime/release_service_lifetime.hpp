#pragma once
#if !defined(_WIN32)
#error release_service_lifetime.hpp is Windows-only.
#endif
#include <stdint.h>
#include "preloader_l/net/socket_runtime.hpp"
#include "preloader_l/runtime/release_server.hpp"
#include "preloader_l/runtime/reference_owner.hpp"

namespace preloader_l { namespace runtime {

enum class ReleaseServiceAcquireResult : uint32_t {
    acquired = 0,
    invalid_argument,
    once_begin_failed,
    once_complete_failed,
    owner_not_published,
};

// Mirrors the curio 0x7000 aliasing-owner contract: context is the stored
// alias/service pointer and owner is the shared control block. The live path uses the
// curio strong/weak state machine for each temporary lease.
struct ReleaseServiceLease {
    // Clean runtime context used by the reconstructed release server.
    ReleaseServerContext* context;
    // Model-shaped shared_ptr alias: curio 0x7000 service subobject at
    // coallocation +0x10. Kept separate from the clean context so the live
    // ownership geometry matches the model without pretending that the
    // clean ReleaseServerContext is byte-identical to the 0x548 object.
    void* model_service;
    void* owner;
};

enum class ReleaseServiceShutdownResult : uint32_t {
    shutdown = 0,
    not_initialized = 1,
    active_leases = 2,
    resource_cleanup_pending = 3,
};

struct ReleaseServiceLifetimeSnapshot {
    uint32_t initialized;
    uint32_t active_leases;
    uintptr_t context;
    uintptr_t model_service;
    uintptr_t owner_control;
    int32_t strong_count;
    int32_t weak_count;
    uint32_t destroy_dispatches;
    uint32_t delete_this_dispatches;
    uint32_t scalar_delete_dispatches;
};

ReleaseServiceAcquireResult acquire_release_service_once(
    const preloader_l::net::SocketApi* api,
    ReleaseServiceLease* lease) noexcept;
void release_release_service(ReleaseServiceLease* lease) noexcept;
// Callers must first quiesce the bootstrap thread outside loader lock.
ReleaseServiceShutdownResult shutdown_release_service_after_quiescence() noexcept;
// Bootstrap failure runs on the bootstrap owner thread after the persistent
// service loop has terminated and the temporary lease has been released.
// This closes the permanent owner without requiring an external join first.
ReleaseServiceShutdownResult shutdown_release_service_after_bootstrap_failure() noexcept;
inline ReleaseServiceShutdownResult shutdown_release_service_for_acceptance() noexcept {
    return shutdown_release_service_after_quiescence();
}
ReleaseServiceLifetimeSnapshot release_service_lifetime_snapshot() noexcept;

} }
