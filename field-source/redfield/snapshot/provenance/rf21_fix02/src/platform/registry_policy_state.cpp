#include "registry_policy_internal.hpp"
#include "preloader_l/platform/registry_policy.hpp"
#include <stddef.h>
#if defined(_WIN32)
extern "C" {
__declspec(dllimport) void* __stdcall GetProcessHeap();
__declspec(dllimport) void* __stdcall HeapAlloc(void*,unsigned long,size_t);
__declspec(dllimport) int __stdcall HeapFree(void*,unsigned long,void*);
}
#endif
#if defined(_MSC_VER)
#include <intrin.h>
#pragma intrinsic(_InterlockedCompareExchange)
#endif

namespace preloader_l { namespace platform { namespace host_policy_detail {
namespace {
struct HandleNode { HandleNode* next; uintptr_t key; uint32_t flags; };
volatile long g_lock=0; HandleNode* g_handles=nullptr; bool g_pseudo_allocation_fenced=false;
void lock_state() noexcept {
#if defined(_MSC_VER)
    while(_InterlockedCompareExchange(&g_lock,1,0)!=0) {}
#else
    while(__atomic_exchange_n(&g_lock,1L,__ATOMIC_ACQUIRE)!=0L) {
        while(__atomic_load_n(&g_lock,__ATOMIC_RELAXED)!=0L) {}
    }
#endif
}
void unlock_state() noexcept {
#if defined(_MSC_VER)
    (void)_InterlockedCompareExchange(&g_lock,0,1);
#else
    __atomic_store_n(&g_lock,0L,__ATOMIC_RELEASE);
#endif
}
HandleNode* find_node_locked(uintptr_t key,HandleNode** prev_out=nullptr) noexcept {
    HandleNode* prev=nullptr;for(HandleNode* n=g_handles;n;n=n->next){if(n->key==key){if(prev_out)*prev_out=prev;return n;}prev=n;}
    if (prev_out) *prev_out = prev;
    return nullptr;
}
}
void* heap_alloc(size_t n) noexcept {
#if defined(_WIN32)
    void* heap=GetProcessHeap();return heap?HeapAlloc(heap,0,n):nullptr;
#else
    (void)n;return nullptr;
#endif
}
void heap_free(void* p) noexcept {
#if defined(_WIN32)
    if(!p)return;void* heap=GetProcessHeap();if(heap)(void)HeapFree(heap,0,p);
#else
    (void)p;
#endif
}
bool track_handle(uintptr_t key,uint32_t flags) noexcept {
    if (!key || !flags) return false;
    lock_state();
    HandleNode* n = find_node_locked(key);
    if(n){n->flags|=flags;unlock_state();return true;}unlock_state();
    auto* fresh=static_cast<HandleNode*>(heap_alloc(sizeof(HandleNode)));if(!fresh)return false;
    fresh->key=key;fresh->flags=flags;lock_state();n=find_node_locked(key);
    if(n){n->flags|=flags;unlock_state();heap_free(fresh);return true;}
    fresh->next=g_handles;g_handles=fresh;unlock_state();return true;
}
uint32_t handle_flags(uintptr_t key) noexcept { lock_state();HandleNode* n=find_node_locked(key);const uint32_t f=n?n->flags:0u;unlock_state();return f; }
bool erase_handle(uintptr_t key,uint32_t* flags_out) noexcept {
    lock_state();HandleNode* prev=nullptr;HandleNode* n=find_node_locked(key,&prev);
    if(!n){unlock_state();if(flags_out)*flags_out=0;return false;}
    if(prev)prev->next=n->next;else g_handles=n->next;const uint32_t f=n->flags;unlock_state();
    if (flags_out) *flags_out = f;
    heap_free(n);
    return true;
}
uintptr_t allocate_pseudo(uint32_t cls) noexcept {
    auto* token=static_cast<uint32_t*>(heap_alloc(sizeof(uint32_t)));if(!token)return 0u;*token=0u;
    auto* fresh=static_cast<HandleNode*>(heap_alloc(sizeof(HandleNode)));if(!fresh){heap_free(token);return 0u;}
    const uintptr_t key=reinterpret_cast<uintptr_t>(token);fresh->key=key;fresh->flags=cls|k_pseudo;
    lock_state();
    if(g_pseudo_allocation_fenced){unlock_state();heap_free(fresh);heap_free(token);return 0u;}
    HandleNode* existing=find_node_locked(key);
    if(existing){existing->flags|=fresh->flags;unlock_state();heap_free(fresh);return key;}
    fresh->next=g_handles;g_handles=fresh;unlock_state();return key;
}

} } }

namespace preloader_l { namespace platform { namespace host_policy_detail {
bool begin_teardown_impl() noexcept {
    lock_state();for(HandleNode* n=g_handles;n;n=n->next){if(n->flags&k_pseudo){unlock_state();return false;}}
    g_pseudo_allocation_fenced=true;unlock_state();return true;
}
void clear_state_impl() noexcept {
    for(;;){lock_state();HandleNode* n=g_handles;if(n)g_handles=n->next;unlock_state();if(!n)break;
        if (n->flags & k_pseudo) heap_free(reinterpret_cast<void*>(n->key));
        heap_free(n);
    }
    lock_state();g_pseudo_allocation_fenced=false;unlock_state();
}
} } }

namespace preloader_l { namespace platform {
bool begin_registry_policy_teardown() noexcept { return host_policy_detail::begin_teardown_impl(); }
void clear_registry_policy_state_for_test() noexcept { host_policy_detail::clear_state_impl(); }
} }
