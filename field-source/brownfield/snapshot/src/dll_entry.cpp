// Loader-safe Windows DLL entry bridge.
// Keep PROCESS_ATTACH bounded to loader bookkeeping plus one nonblocking
// bootstrap dispatch. PTD/FLS initialization, hook mutation, networking, XML,
// registry work, waits and teardown belong to the post-loader worker/export.

#if !defined(_WIN32)
#error preloader_l is a Windows-only DLL.
#endif

#include "preloader_l/runtime/loader_bridge.hpp"
#include "preloader_l/runtime/entry_flow.hpp"
#include "preloader_l/runtime/bootstrap_thread.hpp"

#if defined(_MSC_VER) && defined(PRELOADER_L_COMPATIBILITY_CUSTOM_ENTRY)
// Comparison-only 3.1.7 boundary: /ENTRY:DllMain bypasses the default CRT startup.
// Normalized 3.1.8 shipping does not define this macro and uses _DllMainCRTStartup.
extern "C" void __cdecl __security_init_cookie(void);
#define PRELOADER_L_ENTRY_COOKIE_BOOTSTRAP __declspec(safebuffers)
#else
#define PRELOADER_L_ENTRY_COOKIE_BOOTSTRAP
#endif
#if PRELOADER_L_TRACE_DIAGNOSTICS
#include "preloader_l/runtime/runtime_status.hpp"
#endif
#if defined(PRELOADER_L_RELEASE_BOOTSTRAP)
#include "preloader_l/runtime/secondary_integration.hpp"
#endif

extern "C" void SURGE() noexcept
{
    // Golden model parity: exported SURGE is a single-return stub.
    // Do not add telemetry, allocation, state mutation, or forwarding here.
}

extern "C" uint32_t __stdcall DllShutdown(uint32_t timeout_ms) noexcept
{
    return static_cast<uint32_t>(
        preloader_l::runtime::shutdown_release_runtime_for_unload(timeout_ms));
}

namespace {
int active_entry(void* module, uint32_t reason, void*, void*) noexcept {
    preloader_l::runtime::handle_loader_event(module, reason);
#if PRELOADER_L_TRACE_DIAGNOSTICS
    if (reason == preloader_l::runtime::loader_process_attach)
        preloader_l::runtime::note_active_attach(module);
#endif
    return 1;
}
int secondary_entry(void* module, uint32_t reason, void* reserved, void*) noexcept {
    // Curio top-level entry sends non-attach reasons directly to the
    // secondary layer. Keep clean runtime detach bookkeeping there as well.
    if (reason != preloader_l::runtime::loader_process_attach)
        preloader_l::runtime::handle_loader_event(module, reason, reserved);
#if defined(PRELOADER_L_RELEASE_BOOTSTRAP)
    const int result = preloader_l::runtime::secondary_integration_entry(module, reason, reserved) ? 1 : 0;
#else
    const int result = 1;
#endif
    return result;
}
}

extern "C" PRELOADER_L_ENTRY_COOKIE_BOOTSTRAP int __stdcall DllMain(void* module, unsigned long reason, void* reserved) noexcept
{
#if defined(_MSC_VER) && defined(PRELOADER_L_COMPATIBILITY_CUSTOM_ENTRY)
    if (reason == preloader_l::runtime::loader_process_attach) {
        __security_init_cookie();
    }
#endif
    return preloader_l::runtime::run_curio_entry_flow(
        module, static_cast<uint32_t>(reason), reserved,
        &active_entry, &secondary_entry, nullptr);
}
