#include "win32_effects.hpp"
#include <stdexcept>
#include <bit>

namespace anadius::thread_native {
static_assert(sizeof(void*) == 8 && sizeof(SIZE_T) == 8);
static_assert(sizeof(DWORD) == sizeof(std::uint32_t));
static_assert(sizeof(threads_recovered::Handle) == sizeof(HMODULE));
Api native_api() noexcept {
    return {&::GetModuleHandleExW, &::CreateThread, &::GetLastError,
            &::CloseHandle, &::FreeLibrary, &::FreeLibraryAndExitThread,
            &::ExitThread};
}
Win32Effects::Win32Effects(LPTHREAD_START_ROUTINE start, Api api)
    : start_(start), api_(api) {
    if (!start_ || !api_.get_module || !api_.create || !api_.last_error ||
        !api_.close || !api_.release || !api_.release_and_exit || !api_.exit)
        throw std::invalid_argument("thread wrapper and complete SDK API required");
}
bool Win32Effects::get_module_handle_ex_w(std::uint32_t flags,
    std::uintptr_t address, threads_recovered::Handle* output) {
    // The recovered caller always supplies a live zeroed Context::module.
    // Copy both ways even on FALSE; do not alias uintptr_t storage as HMODULE*.
    HMODULE module = reinterpret_cast<HMODULE>(*output);
    const BOOL result = api_.get_module(static_cast<DWORD>(flags),
        reinterpret_cast<LPCWSTR>(address), &module);
    *output = reinterpret_cast<threads_recovered::Handle>(module);
    return result != FALSE;
}
threads_recovered::Handle Win32Effects::create_thread(void* security,
    std::uint64_t stack_size, threads_recovered::Context* context,
    std::uint32_t flags, std::uint32_t* id) {
    // SDK DWORD is a distinct C++ type from uint32_t with this toolchain.
    // The core consumes this local ID only after a nonzero returned HANDLE.
    DWORD native_id;
    HANDLE result = api_.create(static_cast<LPSECURITY_ATTRIBUTES>(security),
        static_cast<SIZE_T>(stack_size), start_, context,
        static_cast<DWORD>(flags), id ? &native_id : nullptr);
    if (result && id) *id = static_cast<std::uint32_t>(native_id);
    return reinterpret_cast<threads_recovered::Handle>(result);
}
std::uint32_t Win32Effects::last_error() {
    return static_cast<std::uint32_t>(api_.last_error());
}
bool Win32Effects::close_handle(threads_recovered::Handle handle) {
    return api_.close(reinterpret_cast<HANDLE>(handle)) != FALSE;
}
bool Win32Effects::free_library(threads_recovered::Handle handle) {
    return api_.release(reinterpret_cast<HMODULE>(handle)) != FALSE;
}
[[noreturn]] void Win32Effects::free_library_and_exit_thread(
    threads_recovered::Handle module, std::uint32_t code) {
    api_.release_and_exit(reinterpret_cast<HMODULE>(module), static_cast<DWORD>(code));
}
[[noreturn]] void Win32Effects::exit_thread(std::uint32_t code) {
    api_.exit(static_cast<DWORD>(code));
}
RecoveredWin32Effects::RecoveredWin32Effects(LPTHREAD_START_ROUTINE start, Api api,
    crt_runtime_recovered::State& runtime_state,
    crt_runtime_recovered::Effects& runtime_effects,
    crt_errors_recovered::State& error_state,
    crt_errors_recovered::Effects& error_effects)
    : Win32Effects(start, api), runtime_state_(runtime_state),
      runtime_effects_(runtime_effects), error_state_(error_state),
      error_effects_(error_effects) {}
void RecoveredWin32Effects::write_error_slot_05d724(std::uint32_t value) {
    crt_errors_recovered::errno_cell_05d724(error_state_, error_effects_).store(value);
}
void RecoveredWin32Effects::map_os_error_05d698(std::uint32_t value) {
    crt_errors_recovered::map_os_error_05d698(value, error_state_, error_effects_);
}
int RecoveredWin32Effects::runtime_policy_06c0d0() {
    return std::bit_cast<std::int32_t>(
        crt_runtime_recovered::runtime_policy_06c0d0(runtime_state_, runtime_effects_));
}
int RecoveredWin32Effects::initialize_runtime_067320(int mode) {
    return crt_runtime_recovered::initialize_runtime_067320(runtime_state_,
        runtime_effects_, std::bit_cast<std::uint32_t>(mode));
}
void RecoveredWin32Effects::uninitialize_runtime_06736c() {
    crt_runtime_recovered::uninitialize_runtime_06736c(runtime_state_, runtime_effects_);
}
}
