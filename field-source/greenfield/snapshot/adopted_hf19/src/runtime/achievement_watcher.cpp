#include "preloader_l/runtime/achievement_watcher.hpp"
#include <stdint.h>

#if defined(_WIN32)
extern "C" {
__declspec(dllimport) unsigned long __stdcall GetCurrentThreadId();
long _InterlockedCompareExchange(long volatile*, long, long);
long _InterlockedExchange(long volatile*, long);
}
#if defined(_MSC_VER)
#pragma intrinsic(_InterlockedCompareExchange)
#pragma intrinsic(_InterlockedExchange)
#endif
#else
#include <errno.h>
#endif

namespace preloader_l { namespace runtime {
namespace {
AchievementWatcherSink g_sink=nullptr;

#if defined(_WIN32)
volatile long g_callback_owner=0;
long current_callback_owner_token() noexcept {
    const unsigned long tid=GetCurrentThreadId();
    return tid?static_cast<long>(tid):1l;
}
long callback_owner() noexcept {
    return _InterlockedCompareExchange(&g_callback_owner,0,0);
}
bool try_claim_callback_owner(long token) noexcept {
    return _InterlockedCompareExchange(&g_callback_owner,token,0)==0;
}
void release_callback_owner() noexcept {
    (void)_InterlockedExchange(&g_callback_owner,0);
}
#else
uintptr_t g_callback_owner=0;
uintptr_t current_callback_owner_token() noexcept {
    const uintptr_t token=reinterpret_cast<uintptr_t>(&errno);
    return token?token:static_cast<uintptr_t>(1u);
}
uintptr_t callback_owner() noexcept {
    return __atomic_load_n(&g_callback_owner,__ATOMIC_ACQUIRE);
}
bool try_claim_callback_owner(uintptr_t token) noexcept {
    uintptr_t expected=0;
    return __atomic_compare_exchange_n(&g_callback_owner,&expected,token,false,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE);
}
void release_callback_owner() noexcept {
    __atomic_store_n(&g_callback_owner,static_cast<uintptr_t>(0u),__ATOMIC_RELEASE);
}
#endif
}

void bind_achievement_watcher_sink(AchievementWatcherSink sink) noexcept { g_sink=sink; }
bool achievement_watcher_active() noexcept { return g_sink!=nullptr; }
bool achievement_watcher_callback_active() noexcept {
    const auto owner=callback_owner();
    return owner!=0&&owner==current_callback_owner_token();
}

bool publish_achievement_watcher_snapshot(const char* xml,size_t length) noexcept {
    const auto sink=g_sink;
    if(!sink)return true;
    if(!xml&&length)return false;
    const auto owner=current_callback_owner_token();
    if(!try_claim_callback_owner(owner))return false;
    const bool accepted=sink(xml,length);
    release_callback_owner();
    return accepted;
}

void clear_achievement_watcher_sink_for_test() noexcept {
    g_sink=nullptr;
    release_callback_owner();
}

} }
