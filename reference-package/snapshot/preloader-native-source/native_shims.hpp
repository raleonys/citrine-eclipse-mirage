#pragma once
// New integration glue for the recovered embedded preloader; not recovered CRT.
#include "../preloader-source/preloader_core.hpp"
#include <exception>
#include <type_traits>

namespace recovered_preloader::native {
enum class Failure { Unbound, AlreadyBound, CallsInFlight, DifferentErrors, WrongOwner };

// This policy must outlive every Binding and every possible callback, including
// callbacks after close(). Production policies must terminate/trap/report then
// terminate; they must not manufacture an API success or continue execution.
struct Errors {
    virtual ~Errors() = default;
    [[noreturn]] virtual void reject(Failure) = 0;
    [[noreturn]] virtual void escaped_exception(std::exception_ptr) noexcept = 0;
};

// One process-wide policy, immutable after the first successful installation.
// Reinstalling the same object is idempotent; replacement is rejected using the
// installed policy. Calling a shim before this setup is a contract violation and
// terminates: no policy exists to receive it. No default success implementation.
void configure_errors(Errors&);

class Binding final {
public:
    // Core, Host and HookEngine must outlive this object and all active calls.
    // Binding is process-wide, not thread_local. No implicit Core is constructed.
    Binding(Core&, Errors&);
    ~Binding() noexcept;
    Binding(const Binding&) = delete;
    Binding& operator=(const Binding&) = delete;
    Binding(Binding&&) = delete;
    Binding& operator=(Binding&&) = delete;
    // Idempotent after success. Rejects active callbacks without unbinding; caller
    // quiesces its hook entry points and joins users before retry/destruction.
    // Does not disable hooks, free a trampoline, or destroy the supplied Core.
    void close();
private:
    Errors& errors_;
    bool closed_ = false; // close/destruction belongs to one controlling thread.
};

BOOL WINAPI process_next(HANDLE, LPPROCESSENTRY32W) noexcept;
int __cdecl mutex_trylock(void*) noexcept;
BOOL WINAPI crypt_query(DWORD, const void*, DWORD, DWORD, DWORD, DWORD*, DWORD*,
                        DWORD*, HCERTSTORE*, HCRYPTMSG*, const void**) noexcept;
// Functional application DllMain only. Not exported, not an OS/CRT entry point.
BOOL WINAPI application_dll_main(HMODULE, DWORD, LPVOID) noexcept;

using ApplicationDllMain = BOOL (WINAPI *)(HMODULE, DWORD, LPVOID);
static_assert(std::is_convertible_v<decltype(&process_next), decltype(&::Process32NextW)>);
static_assert(std::is_convertible_v<decltype(&mutex_trylock), MutexRoutine>);
static_assert(std::is_convertible_v<decltype(&crypt_query), decltype(&::CryptQueryObject)>);
static_assert(std::is_convertible_v<decltype(&application_dll_main), ApplicationDllMain>);
static_assert(!std::is_copy_constructible_v<Binding> && !std::is_move_constructible_v<Binding>);

// Use these to fill Core's existing Bindings; target pointers remain mandatory
// caller input. These are new wrappers around the recovered Core methods.
Bindings with_detours(ProcessNext target_process, MutexRoutine target_mutex,
                      CryptQuery target_crypt) noexcept;
} // namespace recovered_preloader::native
