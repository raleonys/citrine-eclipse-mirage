#pragma once
#include "preloader_l/runtime/ptd_runtime.hpp"

namespace preloader_l { namespace runtime {

// Recovery seam for a native locale owner. Native evidence proves that the
// normalized clean DLL has no locale-owning subsystem or locale-sensitive CRT
// consumer, so inventing a DllMain caller would be false wiring. The seam is
// available only to explicit model/integration builds; normalized shipping
// leaves PTD locale state null.
#if defined(PRELOADER_L_ENABLE_PTD_LOCALE_BINDING_MODEL)
bool configure_windows_ptd_locale_binding(void* locale_state,
                                          void (*retain_locale)(void*) noexcept,
                                          void (*release_locale)(void*) noexcept) noexcept;
#endif

bool initialize_ptd_runtime_windows() noexcept;
// Final-unload only. Same-module restart after DllShutdown is intentionally not
// supported; a fresh LoadLibrary generation receives fresh module globals.
bool shutdown_ptd_platform_locks_windows() noexcept;

} }
