// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/runtime/memory_allocator.hpp"
#include "preloader_l/runtime/ptd_runtime.hpp"

#include <stdint.h>

namespace preloader_l { namespace runtime {
namespace {

bool ops_complete(const AllocatorPlatformOps& ops) noexcept {
    return ops.heap_alloc && ops.heap_free && ops.call_new_handler;
}

void set_enomem() noexcept {
    if (int* slot = current_errno_slot()) *slot = allocation_errno_enomem;
}

struct RawAttempt {
    void* ptr;
    uint32_t handler_calls;
};

RawAttempt heap_with_inner_policy(size_t request,
                                  AllocationRetryPolicy policy,
                                  const AllocatorPlatformOps& ops) {
    uint32_t calls = 0;
    for (;;) {
        if (void* p = ops.heap_alloc(ops.context, request)) return {p, calls};

        if (policy == AllocationRetryPolicy::inner_then_outer) {
            ++calls;
            if (ops.call_new_handler(ops.context, request) != 0) continue;
        }

        // Curio ordering: ENOMEM occurs only after the inner retry layer
        // has declined. In policy 0 this is the first heap failure and happens
        // before the outer operator-new handler is consulted.
        set_enomem();
        return {nullptr, calls};
    }
}

uintptr_t align_up_32_after_metadata(uintptr_t raw) noexcept {
    // Eight bytes immediately before the user pointer preserve the raw heap
    // allocation used by the matching large-allocation free path.
    return (raw + 8u + 31u) & ~uintptr_t{31u};
}

} // namespace

AllocationResult allocate_block(size_t size,
                                AllocationRetryPolicy policy,
                                const AllocatorPlatformOps& ops) {
    AllocationResult out{nullptr, AllocationTerminal::none, size, 0, 0, false};
    if (!ops_complete(ops)) {
        out.terminal = AllocationTerminal::bad_alloc;
        return out;
    }

    // HEAP replay proves a true zero-size short circuit.
    if (size == 0) return out;

    size_t request = size;
    const bool large = size >= large_allocation_threshold;
    if (large) {
        if (size > static_cast<size_t>(-1) - large_allocation_overhead) {
            // Curio overflow classification is bad_array_new_length rather than
            // the earlier provisional length_error interpretation.
            out.terminal = AllocationTerminal::bad_array_new_length;
            return out;
        }
        request += large_allocation_overhead;
    }
    out.heap_request_size = request;

    for (;;) {
        RawAttempt attempt = heap_with_inner_policy(request, policy, ops);
        out.new_handler_calls += attempt.handler_calls;
        if (attempt.ptr) {
            if (!large) {
                out.pointer = attempt.ptr;
                return out;
            }

            const uintptr_t raw = reinterpret_cast<uintptr_t>(attempt.ptr);
            const uintptr_t aligned = align_up_32_after_metadata(raw);
            void* user = reinterpret_cast<void*>(aligned);
            reinterpret_cast<void**>(user)[-1] = attempt.ptr;
            out.pointer = user;
            out.aligned32 = true;
            return out;
        }

        // Distinct outer operator-new retry layer. Missing/declining callback
        // selects the bad_alloc terminal; a nonzero result retries the CRT
        // allocation layer from its beginning.
        ++out.new_handler_calls;
        if (ops.call_new_handler(ops.context, request) == 0) {
            out.terminal = AllocationTerminal::bad_alloc;
            return out;
        }
    }
}

AllocationReleaseStatus free_direct(void* ptr, const AllocatorPlatformOps& ops) noexcept {
    if (!ptr) return AllocationReleaseStatus::ok;
    if (!ops.heap_free || !ops.heap_free(ops.context, ptr))
        return AllocationReleaseStatus::release_failed;
    return AllocationReleaseStatus::ok;
}

AllocationReleaseStatus free_aligned32(void* aligned_ptr,
                                               size_t logical_size,
                                               const AllocatorPlatformOps& ops) noexcept {
    if (!aligned_ptr) return AllocationReleaseStatus::ok;
    if (logical_size < large_allocation_threshold ||
        logical_size > static_cast<size_t>(-1) - large_allocation_overhead)
        return AllocationReleaseStatus::malformed_large_allocation;

    const uintptr_t aligned = reinterpret_cast<uintptr_t>(aligned_ptr);
    if ((aligned & uintptr_t{31u}) != 0u)
        return AllocationReleaseStatus::malformed_large_allocation;

    void* raw = reinterpret_cast<void**>(aligned_ptr)[-1];
    if (!raw) return AllocationReleaseStatus::malformed_large_allocation;
    const uintptr_t raw_address = reinterpret_cast<uintptr_t>(raw);
    if (raw_address >= aligned || aligned - raw_address < 8u)
        return AllocationReleaseStatus::malformed_large_allocation;

    const uintptr_t recovered_delta = aligned - raw_address - 8u;
    if (recovered_delta > 0x1fu)
        return AllocationReleaseStatus::malformed_large_allocation;

    if (!ops.heap_free || !ops.heap_free(ops.context, raw))
        return AllocationReleaseStatus::release_failed;
    return AllocationReleaseStatus::ok;
}

AllocationReleaseStatus release_block(void* ptr, size_t logical_size,
                                      const AllocatorPlatformOps& ops) noexcept {
    if (!ptr) return AllocationReleaseStatus::ok;
    if (logical_size >= large_allocation_threshold)
        return free_aligned32(ptr, logical_size, ops);
    return free_direct(ptr, ops);
}


} }
