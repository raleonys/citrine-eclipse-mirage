#include "preloader_l/runtime/auth_code_service.hpp"
#include <stdint.h>

#if defined(_WIN32)
extern "C" {
struct AuthCodeFileTime { uint32_t low; uint32_t high; };
__declspec(dllimport) void __stdcall GetSystemTimeAsFileTime(AuthCodeFileTime*);
__declspec(dllimport) int __stdcall QueryPerformanceCounter(int64_t*);
__declspec(dllimport) unsigned long __stdcall GetCurrentProcessId();
__declspec(dllimport) unsigned long __stdcall GetCurrentThreadId();
long _InterlockedCompareExchange(long volatile*, long, long);
}
#if defined(_MSC_VER)
#pragma intrinsic(_InterlockedCompareExchange)
#endif
#endif

namespace preloader_l { namespace runtime {
namespace {
constexpr char k_empty[] = "";
char g_session_auth_code[9]{};
volatile long g_session_auth_lock = 0;
bool g_session_auth_ready = false;

void lock_session_auth() noexcept {
#if defined(_WIN32)
    while (_InterlockedCompareExchange(&g_session_auth_lock, 1, 0) != 0) {}
#else
    // Unit/syntax builds are single-process test surfaces; production is Win32.
#endif
}

void unlock_session_auth() noexcept {
#if defined(_WIN32)
    (void)_InterlockedCompareExchange(&g_session_auth_lock, 0, 1);
#endif
}

uint64_t mix64(uint64_t x) noexcept {
    x ^= x >> 30;
    x *= UINT64_C(0xbf58476d1ce4e5b9);
    x ^= x >> 27;
    x *= UINT64_C(0x94d049bb133111eb);
    return x ^ (x >> 31);
}

uint64_t entropy_seed() noexcept {
#if defined(_WIN32)
    AuthCodeFileTime ft{};
    GetSystemTimeAsFileTime(&ft);
    int64_t qpc = 0;
    (void)QueryPerformanceCounter(&qpc);
    const uint64_t ticks = (static_cast<uint64_t>(ft.high) << 32) | ft.low;
    const uint64_t ids = (static_cast<uint64_t>(GetCurrentProcessId()) << 32) |
                         static_cast<uint64_t>(GetCurrentThreadId());
    const uint64_t address = static_cast<uint64_t>(
        reinterpret_cast<uintptr_t>(&g_session_auth_code[0]));
    return mix64(ticks ^ static_cast<uint64_t>(qpc) ^ ids ^ address);
#else
    return mix64(UINT64_C(0x6c75747261766578) ^
                 static_cast<uint64_t>(reinterpret_cast<uintptr_t>(&g_session_auth_code[0])));
#endif
}

void generate_auth_code(char out[9]) noexcept {
    static constexpr char alphabet[] =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    uint64_t state = entropy_seed();
    for (size_t i = 0; i < 8u; ++i) {
        state = mix64(state + UINT64_C(0x9e3779b97f4a7c15) + static_cast<uint64_t>(i));
        out[i] = alphabet[state % 62u];
    }
    out[8] = '\0';
}

void copy_auth_code(char out[9], const char in[9]) noexcept {
    for (size_t i = 0; i < 9u; ++i) out[i] = in[i];
}

void reset_session_auth_code() noexcept {
    lock_session_auth();
    for (size_t i = 0; i < sizeof(g_session_auth_code); ++i)
        g_session_auth_code[i] = '\0';
    g_session_auth_ready = false;
    unlock_session_auth();
}
}

void clear_auth_code_service() noexcept {
    reset_session_auth_code();
}

bool begin_session_auth_code_lease(AuthCodeSessionLease* lease) noexcept {
    if (!lease) return false;
    for (size_t i = 0; i < sizeof(lease->value); ++i) lease->value[i] = '\0';
    lease->lock_held = false;
    lease->commit_pending = false;

    lock_session_auth();
    lease->lock_held = true;
    if (g_session_auth_ready) {
        copy_auth_code(lease->value, g_session_auth_code);
    } else {
        generate_auth_code(lease->value);
        lease->commit_pending = true;
    }
    return true;
}

AuthCodeValueView session_auth_code_lease_value(const AuthCodeSessionLease* lease) noexcept {
    if (!lease || !lease->lock_held) return AuthCodeValueView{k_empty, 0u};
    return AuthCodeValueView{lease->value, 8u};
}

void finish_session_auth_code_lease(AuthCodeSessionLease* lease, bool commit) noexcept {
    if (!lease || !lease->lock_held) return;
    if (commit && lease->commit_pending) {
        copy_auth_code(g_session_auth_code, lease->value);
        g_session_auth_ready = true;
    }
    lease->commit_pending = false;
    lease->lock_held = false;
    for (size_t i = 0; i < sizeof(lease->value); ++i) lease->value[i] = '\0';
    unlock_session_auth();
}

bool session_auth_code_ready() noexcept {
    lock_session_auth();
    const bool ready = g_session_auth_ready;
    unlock_session_auth();
    return ready;
}

} }
