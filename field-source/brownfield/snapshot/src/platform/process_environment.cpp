#if !defined(_WIN32)
#error process_environment.cpp is Windows-only.
#endif
#include "preloader_l/platform/process_environment.hpp"
#include "preloader_l/config/runtime_provider.hpp"

namespace {
using DWORD = unsigned long;
using BOOL = int;
extern "C" {
__declspec(dllimport) DWORD __stdcall GetEnvironmentVariableA(const char* name, char* buffer, DWORD size);
__declspec(dllimport) BOOL __stdcall SetEnvironmentVariableA(const char* name, const char* value);
__declspec(dllimport) DWORD __stdcall GetLastError();
__declspec(dllimport) void __stdcall SetLastError(DWORD);
}
constexpr DWORD error_envvar_not_found = 203u;
uint32_t length_cb(const char* name, void*) noexcept {
    SetLastError(0u);
    const DWORD length = GetEnvironmentVariableA(name, nullptr, 0);
    if (length != 0u) return static_cast<uint32_t>(length);
    return GetLastError() == error_envvar_not_found
        ? 0u : preloader_l::platform::environment_length_query_error;
}
bool set_cb(const char* name, const char* value, void*) noexcept {
    return SetEnvironmentVariableA(name, value) != 0;
}
}
namespace preloader_l { namespace platform {
EnvironmentStatus apply_process_environment(uint32_t* written_mask,
                                            uint32_t* preserved_mask) noexcept {
    const EnvironmentBackend backend{&length_cb, &set_cb, nullptr};
    return apply_environment(preloader_l::config::runtime_settings_provider(),
                             &backend, written_mask, preserved_mask);
}
EnvironmentStatus restore_process_environment(uint32_t written_mask,
                                              uint32_t* restored_mask) noexcept {
    const EnvironmentBackend backend{&length_cb, &set_cb, nullptr};
    return restore_environment(&backend, written_mask, restored_mask);
}
} }
