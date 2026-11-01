// Internal IDS06 Windows test host. Never part of the shipping DLL or the
// canonical Golden release-host binary. Loads the *actual* built DLL and
// explicitly waits for the test client before requesting quiescent shutdown.
#if !defined(_WIN32)
#error Windows-only test host
#endif
#include <windows.h>
#include <cstdio>
#include <cstdint>

using Shutdown = uint32_t (__stdcall*)(uint32_t) noexcept;
int main() noexcept {
    // The release host requires these hook prerequisites before loading DLL.
    HMODULE advapi = LoadLibraryW(L"ADVAPI32.dll");
    if (!advapi) return 11;
    const char* const required[]{"RegOpenKeyExA", "RegOpenKeyExW",
        "RegQueryValueExA", "RegQueryValueExW", "RegCloseKey"};
    for (const char* name : required) {
        if (!GetProcAddress(advapi, name)) return 12;
    }
    HMODULE dll = LoadLibraryW(L"preloader_l.dll");
    if (!dll) return 13;
    auto shutdown = reinterpret_cast<Shutdown>(GetProcAddress(dll, "DllShutdown"));
    if (!shutdown) return 14;
    std::puts("IDS06_HOST_READY"); std::fflush(stdout);
    if (std::getchar() == EOF) return 15;
    const uint32_t rc = shutdown(30000);
    if (rc != 0) { std::printf("IDS06_SHUTDOWN_FAILED:%u\n",rc); return 16; }
    if (!FreeLibrary(dll)) return 17;
    if (!FreeLibrary(advapi)) return 18;
    std::puts("IDS06_SHUTDOWN_PASS");
    return 0;
}
