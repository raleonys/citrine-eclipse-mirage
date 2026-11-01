// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <cstddef>
#include <cstdint>
namespace preloader_l::runtime {
// Transient buffers require only a physical heap, not retry/new-handler/PTD.
// heap_alloc supplies normally aligned raw storage. heap_free returns nonzero
// only after releasing it; zero leaves the complete allocation valid for retry.
// The descriptor is copied, but its context and callbacks must outlive every
// allocation and queued/in-progress release, under the retained module pin.
struct TransientPlatformOps {
    void* context;
    void* (*heap_alloc)(void*,std::size_t) noexcept;
    int (*heap_free)(void*,void*) noexcept;
};
void* allocate_transient_bytes(std::size_t,const TransientPlatformOps&) noexcept;
bool release_transient_bytes(void*) noexcept;
// Release is a one-time ownership transfer. On failure the internal queue owns
// the block; callers must not access it or submit that payload a second time.
bool retry_pending_transient_releases() noexcept;
bool transient_release_pending() noexcept;
std::size_t transient_pending_release_count() noexcept;
std::size_t transient_pending_release_bytes() noexcept;
// Count/byte snapshots cover queued blocks only. A detached batch can therefore
// report zero while transient_release_pending() still fences an active retry.
// After producers quiesce, explicit shutdown must pass this fence before
// releasing the DLL pin. Failure retains all queued blocks for a later call.
bool drain_transient_releases(std::uint32_t timeout_ms) noexcept;
}
