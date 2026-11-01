#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "../anadius-thread-source/threads.hpp"
#include "../anadius-crt-runtime-source/runtime.hpp"
#include "../anadius-crt-error-source/errors.hpp"

namespace anadius::thread_native {
// New composition boundary, not a recovered global or the original CRT PTD.
// Fields retain the SDK's exact signatures, including DWORD (unsigned long).
struct Api {
    decltype(&::GetModuleHandleExW) get_module;
    decltype(&::CreateThread) create;
    decltype(&::GetLastError) last_error;
    decltype(&::CloseHandle) close;
    decltype(&::FreeLibrary) release;
    decltype(&::FreeLibraryAndExitThread) release_and_exit;
    decltype(&::ExitThread) exit;
};
Api native_api() noexcept; // Construction binds imports, but invokes none.

// Supplies only the seven SDK call sites used by the recovered thread core.
// Heap, PTD/FLS ownership, invalid-parameter handling, runtime initialization
// and invocation of the application function remain REQUIRED pure methods.
// start MUST be a rebuilt 05e7a8 wrapper with a valid Effects binding, not the
// application's Context::entry. Caller owns that binding until every worker
// has actually terminated, including suspended and not-yet-started workers.
class Win32Effects : public threads_recovered::Effects {
public:
    Win32Effects(LPTHREAD_START_ROUTINE start, Api);
    bool get_module_handle_ex_w(std::uint32_t, std::uintptr_t,
                                threads_recovered::Handle*) final;
    threads_recovered::Handle create_thread(void*, std::uint64_t,
        threads_recovered::Context*, std::uint32_t, std::uint32_t*) final;
    std::uint32_t last_error() final;
    bool close_handle(threads_recovered::Handle) final;
    bool free_library(threads_recovered::Handle) final;
    [[noreturn]] void free_library_and_exit_thread(threads_recovered::Handle,
                                                  std::uint32_t) final;
    [[noreturn]] void exit_thread(std::uint32_t) final;
private:
    const LPTHREAD_START_ROUTINE start_;
    const Api api_;
};

// New composition of the recovered error/runtime bodies with the SDK adapter.
// The two supplied providers must use the same CRT/TLS ownership domain as the
// remaining PTD methods. No copied or substitute native CRT state is created.
class RecoveredWin32Effects : public Win32Effects {
public:
    RecoveredWin32Effects(LPTHREAD_START_ROUTINE start, Api,
        crt_runtime_recovered::State&, crt_runtime_recovered::Effects&,
        crt_errors_recovered::State&, crt_errors_recovered::Effects&);
    void write_error_slot_05d724(std::uint32_t) final;
    void map_os_error_05d698(std::uint32_t) final;
    int runtime_policy_06c0d0() final;
    int initialize_runtime_067320(int) final;
    void uninitialize_runtime_06736c() final;
private:
    crt_runtime_recovered::State& runtime_state_;
    crt_runtime_recovered::Effects& runtime_effects_;
    crt_errors_recovered::State& error_state_;
    crt_errors_recovered::Effects& error_effects_;
};
}
