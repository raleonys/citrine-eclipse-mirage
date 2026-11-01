#if !defined(_WIN32) || !defined(_M_X64)
#error GF00 shipping-ABI skeleton requires Windows MSVC x64.
#endif
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cstdint>
#include "greenfield/core/lifecycle.hpp"

extern "C" void __cdecl SURGE() noexcept {
    // Intentionally empty, matching the historical exported stub's ABI shape.
    // No SDK, registry, network or compatibility behavior is implemented.
}
extern "C" std::uint32_t __stdcall DllShutdown(std::uint32_t timeout_ms) noexcept {
    return greenfield::core::runtime_lifecycle().shutdown(timeout_ms);
}
extern "C" BOOL WINAPI DllMain(HINSTANCE module, DWORD reason, LPVOID reserved) noexcept {
    (void)reserved;
    (void)module;
    (void)reason;
    // No worker, parser, hook, allocation or I/O under loader lock.
    // Do not DisableThreadLibraryCalls here: GF00 links the static CRT (/MT),
    // which retains responsibility for thread initialization/notifications.
    return TRUE;
}
