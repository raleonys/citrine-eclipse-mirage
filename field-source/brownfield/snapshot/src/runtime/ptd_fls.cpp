#if !defined(_WIN32)
#error preloader_l PTD Windows backend is Windows-only.
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include "preloader_l/runtime/ptd_fls.hpp"

namespace preloader_l { namespace runtime {
namespace {

INIT_ONCE g_ptd_lock_once = INIT_ONCE_STATIC_INIT;
CRITICAL_SECTION g_ptd_locks[6]{};
volatile LONG g_ptd_locks_state = 0; // 0=never, 1=live, 2=retired-final-unload

BOOL CALLBACK init_ptd_locks(PINIT_ONCE, PVOID, PVOID*) noexcept {
    if (::InterlockedCompareExchange(&g_ptd_locks_state, 0, 0) == 2) return FALSE;
    for (unsigned i = 0; i < 6; ++i) {
        if (!::InitializeCriticalSectionEx(&g_ptd_locks[i], 4000, 0)) {
            while (i) ::DeleteCriticalSection(&g_ptd_locks[--i]);
            return FALSE;
        }
    }
    (void)::InterlockedExchange(&g_ptd_locks_state, 1);
    return TRUE;
}

bool ensure_ptd_locks() noexcept {
    if (::InterlockedCompareExchange(&g_ptd_locks_state, 0, 0) == 2) return false;
    return ::InitOnceExecuteOnce(&g_ptd_lock_once, &init_ptd_locks, nullptr, nullptr) != FALSE &&
           ::InterlockedCompareExchange(&g_ptd_locks_state, 0, 0) == 1;
}

struct SharedAnchor { volatile LONG refs; };
SharedAnchor g_shared_anchor{1};
void* g_locale_state = nullptr;
void (*g_retain_locale)(void*) noexcept = nullptr;
void (*g_release_locale)(void*) noexcept = nullptr;
PtdSharedBindings g_default_bindings{
    &g_shared_anchor,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
};

uint32_t get_last_error(void*) noexcept { return static_cast<uint32_t>(::GetLastError()); }
void set_last_error(void*, uint32_t value) noexcept { ::SetLastError(static_cast<DWORD>(value)); }
uint32_t fls_alloc(void*, void (*callback)(void*) noexcept) noexcept {
    return static_cast<uint32_t>(::FlsAlloc(reinterpret_cast<PFLS_CALLBACK_FUNCTION>(callback)));
}
int fls_free(void*, uint32_t index) noexcept { return ::FlsFree(static_cast<DWORD>(index)) ? 1 : 0; }
void* fls_get_value(void*, uint32_t index) noexcept { return ::FlsGetValue(static_cast<DWORD>(index)); }
int fls_set_value(void*, uint32_t index, void* value) noexcept {
    return ::FlsSetValue(static_cast<DWORD>(index), value) ? 1 : 0;
}
void* alloc_zero(void*, size_t size) noexcept {
    return ::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, size);
}
int free_mem(void*, void* value) noexcept {
    if (!value) return 1;
    return ::HeapFree(::GetProcessHeap(), 0, value) ? 1 : 0;
}
void enter_lock(void*, uint32_t lock_id) noexcept {
    if (lock_id < 6 && ensure_ptd_locks()) ::EnterCriticalSection(&g_ptd_locks[lock_id]);
}
void leave_lock(void*, uint32_t lock_id) noexcept {
    if (lock_id < 6) ::LeaveCriticalSection(&g_ptd_locks[lock_id]);
}
void retain_shared(void* value) noexcept {
    auto* anchor = static_cast<SharedAnchor*>(value);
    (void)::InterlockedIncrement(&anchor->refs);
}
void release_shared(void* value) noexcept {
    auto* anchor = static_cast<SharedAnchor*>(value);
    if (anchor->refs > 1) (void)::InterlockedDecrement(&anchor->refs);
}

const PtdPlatformOps g_windows_ops{
    nullptr, &get_last_error, &set_last_error,
    &fls_alloc, &fls_free, &fls_get_value, &fls_set_value,
    &alloc_zero, &free_mem, &enter_lock, &leave_lock,
};

void refresh_bindings() noexcept {
    g_default_bindings.shared_state = &g_shared_anchor;
    g_default_bindings.locale_state = g_locale_state;
    g_default_bindings.retain_shared = &retain_shared;
    g_default_bindings.release_shared = &release_shared;
    g_default_bindings.retain_locale = g_retain_locale;
    g_default_bindings.release_locale = g_release_locale;
}

} // namespace

#if defined(PRELOADER_L_ENABLE_PTD_LOCALE_BINDING_MODEL)
bool configure_windows_ptd_locale_binding(void* locale_state,
                                          void (*retain_locale)(void*) noexcept,
                                          void (*release_locale)(void*) noexcept) noexcept {
    // The binding is process-init state. Rebinding after FLS/PTD bootstrap would
    // make existing and future PTDs observe different owners, which is outside
    // the curio contract.
    if (ptd_fls_index() != ptd_fls_out_of_indexes) return false;
    g_locale_state = locale_state;
    g_retain_locale = retain_locale;
    g_release_locale = release_locale;
    refresh_bindings();
    return true;
}
#endif

bool initialize_ptd_runtime_windows() noexcept {
    if (!ensure_ptd_locks()) return false;
    refresh_bindings();
    return initialize_ptd_runtime(g_windows_ops, g_default_bindings);
}

bool shutdown_ptd_platform_locks_windows() noexcept {
    if (ptd_cleanup_pending()) return false;
    const LONG state = ::InterlockedCompareExchange(&g_ptd_locks_state, 2, 1);
    if (state == 2 || state == 0) {
        if (state == 0) (void)::InterlockedExchange(&g_ptd_locks_state, 2);
        return true;
    }
    if (state != 1) return false;
    // At this final-unload boundary all PTD/FLS owners are already quiescent.
    // Acquire/release each lock before deletion to drain any operation that
    // entered before the lifecycle reached this boundary.
    for (unsigned i = 0; i < 6; ++i) {
        ::EnterCriticalSection(&g_ptd_locks[i]);
        ::LeaveCriticalSection(&g_ptd_locks[i]);
        ::DeleteCriticalSection(&g_ptd_locks[i]);
    }
    return true;
}

} }
