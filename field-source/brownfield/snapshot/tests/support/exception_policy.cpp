#include "preloader_l/runtime/exception_policy.hpp"

#include <new>

namespace preloader_l { namespace runtime {
namespace {
class UnwindOnlyGuard {
public:
    UnwindOnlyGuard(UnwindCleanup cleanup, void* object) noexcept
        : cleanup_(cleanup), object_(object), armed_(true) {}
    ~UnwindOnlyGuard() noexcept {
        if (armed_ && cleanup_) cleanup_(object_);
    }
    void dismiss() noexcept { armed_ = false; }
private:
    UnwindCleanup cleanup_;
    void* object_;
    bool armed_;
};
} // namespace

AllocatorEscapeContract allocator_escape_contract(AllocatorEscapeShape shape) noexcept {
    switch (shape) {
    case AllocatorEscapeShape::noexcept_a:
        return {AllocatorEscapeDisposition::search_terminate, 6u, 0x1D0u, false, false, false};
    case AllocatorEscapeShape::target_unwind:
        return {AllocatorEscapeDisposition::target_unwind_terminate, 6u, 0x190u, true, true, false};
    case AllocatorEscapeShape::noexcept_b:
        return {AllocatorEscapeDisposition::search_terminate, 6u, 0x170u, false, false, false};
    }
    return {AllocatorEscapeDisposition::search_terminate, 0u, 0u, false, false, false};
}

[[noreturn]] void throw_allocation_terminal(AllocationTerminal terminal) {
    // The curio overflow terminal is bad_array_new_length, whose
    // catchable chain includes bad_alloc -> exception. Ordinary exhaustion is
    // the curio std::bad_alloc terminal.
    if (terminal == AllocationTerminal::bad_array_new_length)
        throw std::bad_array_new_length{};
    throw std::bad_alloc{};
}

void* allocate_block_or_throw(size_t size,
                              AllocationRetryPolicy policy,
                              const AllocatorPlatformOps& ops) {
    const AllocationResult result = allocate_block(size, policy, ops);
    if (result.terminal != AllocationTerminal::none)
        throw_allocation_terminal(result.terminal);
    return result.pointer;
}

void invoke_with_unwind_cleanup(ThrowingBody body,
                                void* body_context,
                                UnwindCleanup cleanup,
                                void* cleanup_object) {
    UnwindOnlyGuard guard(cleanup, cleanup_object);
    body(body_context);
    guard.dismiss();
}

void invoke_noexcept_boundary(ThrowingBody body, void* body_context) noexcept {
    body(body_context);
}

} }
