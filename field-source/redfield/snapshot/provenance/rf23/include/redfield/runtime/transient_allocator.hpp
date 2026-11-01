// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <cstddef>
#include <cstdint>
namespace redfield::runtime {
// Transient buffers require only a physical heap, not retry/new-handler/PTD.
struct TransientPlatformOps {
    void* context;
    void* (*heap_alloc)(void*,std::size_t) noexcept;
    int (*heap_free)(void*,void*) noexcept;
};
void* allocate_transient_bytes(std::size_t,const TransientPlatformOps&) noexcept;
bool release_transient_bytes(void*) noexcept;
bool retry_pending_transient_releases() noexcept;
bool transient_release_pending() noexcept;
std::size_t transient_pending_release_count() noexcept;
std::size_t transient_pending_release_bytes() noexcept;
// After producers quiesce, explicit shutdown must pass this fence before
// releasing the DLL pin. Failure retains all queued blocks for a later call.
bool drain_transient_releases(std::uint32_t timeout_ms) noexcept;
}
