#pragma once
#include "../anadius-online-version-source/version.hpp"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace anadius::module_reference {
struct Api {
    decltype(&::GetModuleHandleExW) acquire=::GetModuleHandleExW;
    decltype(&::FreeLibrary) release=::FreeLibrary;
};
// Adds one owned OS reference per successful lease, using the exact loaded
// module base as FROM_ADDRESS. Copies of that lease share the same reference.
// The caller must already own a stable executable LoadLibrary reference while
// acquiring, and must serialize acquisition against source FreeLibrary.
//
// This keeps only the selected image loaded: forwarded exports, foreign vtable
// code, hook engines and asynchronous users need independent provenance/drain.
// It neither loads a module nor supplies foreign_quiescent. No DllMain/loader
// lock use or last-reference self-unload of this provider/caller image is valid.
// Injected SDK callbacks must have nonthrowing, documented Win32 outcomes;
// their backing is retained by the factory AND every outstanding lease.
// Unknown outcomes, malformed success and release failure fail-stop.
// Acquisition failure throws system_error; bookkeeping preserves LastError.
online_version::CodeLease make_code_lease(Api={},std::shared_ptr<void> backing={});
}
