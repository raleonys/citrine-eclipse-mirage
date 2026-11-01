#if !defined(_WIN32)
#error preloader_l allocator Windows backend is Windows-only.
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include "preloader_l/runtime/heap_allocator.hpp"
#if defined(PRELOADER_L_ENABLE_ALLOCATOR_THROW_BRIDGE)
#include "preloader_l/runtime/exception_policy.hpp"
#endif

namespace preloader_l { namespace runtime {
namespace {

INIT_ONCE g_handler_once = INIT_ONCE_STATIC_INIT;
CRITICAL_SECTION g_handler_lock{};
volatile LONG g_handler_lock_state = 0; // 0=never, 1=live, 2=retired-final-unload
RuntimeNewHandler g_handler = nullptr;
void* g_handler_context = nullptr;

BOOL CALLBACK init_handler_lock(PINIT_ONCE, PVOID, PVOID*) {
    if (::InterlockedCompareExchange(&g_handler_lock_state, 0, 0) == 2) return FALSE;
    ::InitializeCriticalSection(&g_handler_lock);
    (void)::InterlockedExchange(&g_handler_lock_state, 1);
    return TRUE;
}
bool ensure_handler_lock() noexcept {
    if (::InterlockedCompareExchange(&g_handler_lock_state, 0, 0) == 2) return false;
    PVOID ignored = nullptr;
    return ::InitOnceExecuteOnce(&g_handler_once, &init_handler_lock, nullptr, &ignored) != FALSE &&
           ::InterlockedCompareExchange(&g_handler_lock_state, 0, 0) == 1;
}

void* heap_alloc_impl(void*, size_t size) noexcept {
    HANDLE heap = ::GetProcessHeap();
    return heap ? ::HeapAlloc(heap, 0, size) : nullptr;
}

int heap_free_impl(void*, void* ptr) noexcept {
    if (!ptr) return 1;
    HANDLE heap = ::GetProcessHeap();
    return heap && ::HeapFree(heap, 0, ptr) ? 1 : 0;
}

int call_new_handler_impl(void*, size_t size) {
    RuntimeNewHandler handler = nullptr;
    void* context = nullptr;
    if (!ensure_handler_lock()) return 0;
    ::EnterCriticalSection(&g_handler_lock);
    handler = g_handler;
    context = g_handler_context;
    ::LeaveCriticalSection(&g_handler_lock);
    // CALLBACK proves the callback invocation happens after the getter has
    // released its critical section; exceptions therefore do not unwind it.
    return handler ? handler(size, context) : 0;
}

const AllocatorPlatformOps g_ops{
    nullptr,
    &heap_alloc_impl,
    &heap_free_impl,
    &call_new_handler_impl,
};

} // namespace

RuntimeNewHandler set_runtime_new_handler(RuntimeNewHandler handler,
                                          void* handler_context,
                                          void** previous_context) noexcept {
    if (!ensure_handler_lock()) {
        if (previous_context) *previous_context = nullptr;
        return nullptr;
    }
    ::EnterCriticalSection(&g_handler_lock);
    RuntimeNewHandler previous = g_handler;
    if (previous_context) *previous_context = g_handler_context;
    g_handler = handler;
    g_handler_context = handler_context;
    ::LeaveCriticalSection(&g_handler_lock);
    return previous;
}

const AllocatorPlatformOps& windows_allocator_platform_ops() noexcept {
    return g_ops;
}

bool shutdown_allocator_platform_lock_windows() noexcept {
    const LONG observed = ::InterlockedCompareExchange(&g_handler_lock_state, 0, 0);
    if (observed == 2) return true;
    if (observed == 0) {
        (void)::InterlockedExchange(&g_handler_lock_state, 2);
        return true;
    }
    if (::InterlockedCompareExchange(&g_handler_lock_state, 2, 1) != 1) return false;
    ::EnterCriticalSection(&g_handler_lock);
    g_handler = nullptr;
    g_handler_context = nullptr;
    ::LeaveCriticalSection(&g_handler_lock);
    ::DeleteCriticalSection(&g_handler_lock);
    return true;
}

#if defined(PRELOADER_L_ENABLE_ALLOCATOR_THROW_BRIDGE)
void* allocate_block_or_throw_windows(size_t size,
                                      AllocationRetryPolicy policy) {
    return allocate_block_or_throw(size, policy, windows_allocator_platform_ops());
}
#endif

} }
