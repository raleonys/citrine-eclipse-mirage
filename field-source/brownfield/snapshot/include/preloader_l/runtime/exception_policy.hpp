#pragma once
#include "preloader_l/runtime/memory_allocator.hpp"
#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace runtime {

// Semantic exception-policy test support. The implementation is owned by tests/support; shipping code may only see this header through explicitly compile-gated non-production bridges.

// The three curio allocator escape shapes in the unwind contract. These are
// semantic source-level contracts, not copied compiler unwind metadata.
enum class AllocatorEscapeShape : uint32_t {
    noexcept_a = 0,
    target_unwind = 1,
    noexcept_b = 2,
};

enum class AllocatorEscapeDisposition : uint32_t {
    search_terminate = 0,
    target_unwind_terminate = 1,
};

struct AllocatorEscapeContract {
    AllocatorEscapeDisposition disposition;
    uint32_t route_count;
    uint32_t body_rsp_to_caller_rsp_delta;
    bool allocator_core_cleanup;
    bool allocator_mid_cleanup;
    bool catch_funclet_executes;
};

using ThrowingBody = void (*)(void* context);
using UnwindCleanup = void (*)(void* object) noexcept;

constexpr uint32_t curio_allocator_escape_routes = 18;
constexpr uint32_t curio_search_terminate_routes = 12;
constexpr uint32_t curio_target_unwind_terminate_routes = 6;
constexpr uint32_t curio_catch_funclet_routes = 0;

AllocatorEscapeContract allocator_escape_contract(AllocatorEscapeShape shape) noexcept;

// Convert D2's allocation terminal classification into the native C++ type
// surface recovered from the model handler map.
[[noreturn]] void throw_allocation_terminal(AllocationTerminal terminal);
void* allocate_block_or_throw(size_t size,
                              AllocationRetryPolicy policy,
                              const AllocatorPlatformOps& ops);

// Source-level equivalent of an unwind-only cleanup funclet: cleanup runs only
// when body exits by exception, never on the normal return path.
void invoke_with_unwind_cleanup(ThrowingBody body,
                                void* body_context,
                                UnwindCleanup cleanup,
                                void* cleanup_object);

// A true noexcept boundary. If body throws, the C++ runtime performs terminate
// semantics instead of permitting an outer catch to observe the exception.
void invoke_noexcept_boundary(ThrowingBody body, void* body_context) noexcept;

} }
