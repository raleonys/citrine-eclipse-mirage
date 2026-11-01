#if !defined(_WIN32)
#error secondary_integration_entry.cpp is Windows-only.
#endif

#include "preloader_l/runtime/bootstrap_thread.hpp"
#include "preloader_l/runtime/bootstrap_runner.hpp"
#include "preloader_l/runtime/loader_bridge.hpp"
#include "preloader_l/runtime/release_server_io.hpp"
#include "preloader_l/runtime/secondary_integration.hpp"

namespace preloader_l { namespace runtime {
#if PRELOADER_L_TRACE_DIAGNOSTICS
void secondary_note_attach(uint32_t crypt_status, bool bootstrap) noexcept;
#endif

bool secondary_integration_entry(void* module, uint32_t reason, void*) noexcept {
    if (reason == loader_process_detach) {
        // Loader-lock path is publication-only. DllShutdown owns waits,
        // MinHook disable/remove, service release and PTD/FLS cleanup.
        request_bootstrap_stop();
        request_release_server_stop_windows();
        return true;
    }
    if (reason != loader_process_attach) return true;

    // SAFE-FREEZE: never enter MinHook's freeze path while
    // the Windows loader lock is held. The bootstrap thread is created here
    // without waiting; Windows cannot execute its DLL code until DllMain has
    // returned. Its first post-loader action installs all nine MinHook hooks.
    const bool scheduled = schedule_release_bootstrap(module);
#if PRELOADER_L_TRACE_DIAGNOSTICS
    secondary_note_attach(0u, scheduled);
#else
    (void)scheduled;
#endif
    return true;
}

} }
