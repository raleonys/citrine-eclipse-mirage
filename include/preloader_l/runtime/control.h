// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#ifdef __cplusplus
extern "C" {
#endif
// Windows x64 C ABI for this rebuilt application. The caller must retain its
// LoadLibrary reference until shutdown succeeds. Calls must be outside DllMain.
// DLL attach starts one worker. Shutdown must complete before a caller releases
// its LoadLibrary lease. Readiness/port queries are not part of the public ABI.
// Branding adaptation of the RE RET-only import marker. No startup, readiness,
// authentication or shutdown result is conveyed by calling this function.
void SURGE(void);
// Loader-compatible shutdown status. Every nonzero result forbids unloading.
// This is the single public shutdown entry for the application.
enum DllShutdownStatus {
    DLL_SHUTDOWN_OK=0,
    DLL_SHUTDOWN_CONCURRENT=1,
    DLL_SHUTDOWN_TIMEOUT=2,
    DLL_SHUTDOWN_WAIT_FAILED=3,
    DLL_SHUTDOWN_CLEANUP_FAILED=4
};
DWORD WINAPI DllShutdown(DWORD timeout_ms);
#ifdef __cplusplus
}
#endif
