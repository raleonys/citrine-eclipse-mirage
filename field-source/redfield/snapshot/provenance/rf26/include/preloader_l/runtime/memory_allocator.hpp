// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace runtime {

enum class AllocationRetryPolicy : uint32_t {
    outer_retry_only = 0,
    inner_then_outer = 1,
};

enum class AllocationTerminal : uint32_t {
    none = 0,
    bad_alloc = 1,
    bad_array_new_length = 2,
};

enum class AllocationReleaseStatus : uint32_t {
    ok = 0,
    malformed_large_allocation = 1,
    release_failed = 2,
};

struct AllocatorPlatformOps {
    void* context;
    void* (*heap_alloc)(void* context, size_t size) noexcept;
    int (*heap_free)(void* context, void* ptr) noexcept;
    // Clean-source equivalent of the curio _callnewh boundary.
    // Nonzero requests a retry; zero declines. The callback is intentionally
    // allowed to throw so D4 can preserve the curio propagation behavior.
    int (*call_new_handler)(void* context, size_t requested_size);
};

struct AllocationResult {
    void* pointer;
    AllocationTerminal terminal;
    size_t logical_size;
    size_t heap_request_size;
    uint32_t new_handler_calls;
    bool aligned32;
};

constexpr size_t large_allocation_threshold = 0x1000;
constexpr size_t large_allocation_overhead = 0x27;
constexpr int allocation_errno_enomem = 12;

AllocationResult allocate_block(size_t size,
                                AllocationRetryPolicy policy,
                                const AllocatorPlatformOps& ops);
AllocationReleaseStatus free_direct(void* ptr, const AllocatorPlatformOps& ops) noexcept;
AllocationReleaseStatus free_aligned32(void* aligned_ptr, size_t logical_size,
                                               const AllocatorPlatformOps& ops) noexcept;
// Matching release for allocate_block(): callers provide the same logical size
// used at allocation time so the raw-vs-aligned physical representation is
// selected centrally and cannot drift across ownership sites.
AllocationReleaseStatus release_block(void* ptr, size_t logical_size,
                                      const AllocatorPlatformOps& ops) noexcept;

} }
