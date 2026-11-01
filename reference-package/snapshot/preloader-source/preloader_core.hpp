#pragma once
// Evidence-directed reconstruction of preloader_l.dll (SHA256
// 36e775ed97b7a41c4f0cb98b94ee8edb74fab5cfe17d5fa8cfed265228301846).
// Compilable core objects; not a complete or behavior-equivalent DLL project.
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <tlhelp32.h>
#include <wincrypt.h>
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace recovered_preloader {
using ProcessNext = BOOL (WINAPI *)(HANDLE, LPPROCESSENTRY32W);
// msvcp140's mutex object remains opaque; the imported function returns int in EAX.
using MutexRoutine = int (__cdecl *)(void*);
using CryptQuery = decltype(&::CryptQueryObject);
static_assert(sizeof(void*) == 8, "The recovered binary is Windows x64");
static_assert(sizeof(wchar_t) == 2);
static_assert(offsetof(PROCESSENTRY32W, th32ProcessID) == 8);
static_assert(offsetof(PROCESSENTRY32W, th32ParentProcessID) == 0x20);
static_assert(sizeof(std::atomic<std::uint32_t>) == 4);
static_assert(std::atomic<std::uint32_t>::is_always_lock_free);

enum class TrapSite {
    MissingOriginal, NullMainImage, PoisonedOnce, PoisonedImageRange,
    ConsumedInitializer, EngineInitialization, Allocation, InvalidUtf16
};

// Required dependencies, deliberately abstract: no successful dummy engine/API.
// A production binding must satisfy these contracts; tests inject mocks only.
struct Host {
    virtual ~Host() = default;
    virtual BOOL disable_thread_library_calls(HMODULE) = 0;
    virtual DWORD get_last_error() = 0;
    virtual DWORD current_process_id() = 0;
    virtual HMODULE main_module() = 0; // GetModuleHandleA(nullptr), no LoadLibrary.
    virtual int mutex_lock(void*) = 0; // msvcp140!_Mtx_lock, not std::mutex.
    // Native contract: WaitOnAddress(&state, &comparand, 4, INFINITE).
    virtual BOOL wait32(std::atomic<std::uint32_t>& state, std::uint32_t comparand) = 0;
    virtual void wake_all(std::atomic<std::uint32_t>& state) = 0;
    // GetProcessHeap + HeapAlloc/ReAlloc/Free, flags 0, alignment 1 here.
    virtual void* allocate(std::size_t) = 0;
    virtual void* reallocate(void*, std::size_t) = 0;
    virtual void release(void*) = 0;
    // Native implementation must emit UD2. Mock may throw to inspect the branch.
    [[noreturn]] virtual void invalid_instruction(TrapSite) = 0;
};

struct HookEngine {
    virtual ~HookEngine() = default;
    virtual int initialize() = 0; // 0 success / 1 already initialized; caller also accepts -2.
    virtual int create(void* target, void* detour, void** trampoline) = 0;
    virtual int enable(void* target) = 0; // nullptr means all hooks.
};

struct OnceCall { std::uint32_t next_state = 0; bool was_poisoned = false; };
class Once {
public:
    enum : std::uint32_t { Complete = 0, Running = 1, Poisoned = 2, Incomplete = 3, Waiters = 4 };
    std::atomic<std::uint32_t> state{Incomplete};
    using Callback = void (*)(void*, OnceCall&);
    // Recovered state transitions at 0x180003f8f. No invented exception cleanup:
    // if an injected callback throws, state remains Running (as the normal path
    // never reaches XCHG). Native core callbacks terminate with UD2 on failure.
    void call(Host&, bool ignore_poison, void* context, Callback);
};

struct Bindings {
    ProcessNext process_next_target{};
    MutexRoutine trylock_target{};
    CryptQuery crypt_target{};
    // Integrator supplies ABI-correct free-function wrappers calling this Core.
    ProcessNext process_next_detour{};
    MutexRoutine trylock_detour{};
    CryptQuery crypt_detour{};
};

class Core {
public:
    Core(Host& host, HookEngine& engine, Bindings bindings)
        : host_(host), engine_(engine), bindings_(bindings) {}
    BOOL dll_main(HMODULE module, DWORD reason, LPVOID reserved);
    BOOL process_next(HANDLE, LPPROCESSENTRY32W);
    int mutex_trylock(void* opaque_mutex);
    BOOL crypt_query(DWORD object_type, const void* object, DWORD expected_content,
                     DWORD expected_format, DWORD flags, DWORD* encoding,
                     DWORD* content, DWORD* format, HCERTSTORE* store,
                     HCRYPTMSG* message, const void** context);
    // Explicitly visible for integration and state-directed tests; null originals
    // trap exactly as the observed global-null branches do.
    ProcessNext original_process_next{}; // DAT_1800072c8
    MutexRoutine original_trylock{};     // DAT_1800072c0
    CryptQuery original_crypt{};         // DAT_1800072d0
    Once image_once;                     // DAT_180007010
    Once mutex_once;                     // DAT_180007018
    Once engine_once;                    // DAT_18000701c
private:
    Host& host_;
    HookEngine& engine_;
    Bindings bindings_;
    std::uintptr_t image_begin_{};        // recovered RAX return, DAT_180007000
    std::uintptr_t image_end_{};          // recovered RDX return, DAT_180007008
    void ensure_engine();
    void create_one(void* target, void* detour, void** result);
};
} // namespace recovered_preloader
