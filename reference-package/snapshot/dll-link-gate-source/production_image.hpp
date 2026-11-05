#pragma once
#include "../anadius-bootstrap-runtime-source/factory.hpp"

namespace anadius::dll_link_gate {
// Original synchronous application-entry profile selected by the user.
// Both helpers are DEFINED in entry.cpp; sources are not yet rebuilt. These names are
// source integration contracts, not recovered functions or callable image VAs.
// The definition owns the real Resources graph, permanent Errors, seeds
// and callback/worker state, then run the real application attach once. No
// fixture, null runtime, success stub or original-DLL fallback qualifies.
//
// Own the COMPLETE synchronous attach, not a Bound Runtime pointer for DllMain
// to initialize later. Both Runtime and Domain must be adopted before publish
// or ApplicationEntry.
// DllMain must not call that entry again after this function returns.
//
// Preserve038970 create/detach/host/online/synchronous035850(true)/third-worker
// ordering. No deferred Initialize API, controller readiness wait, cooperative
// pre-unload proof or worker join is introduced into this original profile.
// The general cooperative Runtime::close/Controller contract remains separate;
// it is not the implementation of original application reason0. The actual
// image owner adopts native providers/Domain/Runtime before publication. This
// source definition does not establish compiled PE or runtime/loader equivalence.
// TRUE means complete synchronous application attach, with all roots retained
// by the image owner. Do not translate an unknown/partly-published startup into
// successful attach. Compiler CRT runs once and owns the actual registered
// source exit-callback position; do not call recovered outer CRT around it.
// Root storage is deliberately retained for mapped-image CRT callbacks. Full
// native resource teardown/live FreeLibrary behavior remains unvalidated; no
// cooperative destruction or self-pin is silently substituted. Unknown startup
// failure uses the selected compiler abort policy, never a false success.
// HOST source API change from Runtime* to BOOL: entry and definition require a
// consistent rebuild. No native/recovered function ABI is being changed.
BOOL require_loader_attach_runtime(
    HINSTANCE module, LPVOID reserved) noexcept;

// DEFINED in entry.cpp. Original038870 has no application reason0 action.
// The existing name is retained for source compatibility only: this function
// does not attest quiescence. It neither joins/waits/closes Runtime nor invokes
// source exit cleanup a second time. Compiler CRT keeps its normal reserved-
// aware detach/termination behavior. Safe arbitrary hot-unload is not claimed.
void require_loader_detach_quiescence(HINSTANCE module, LPVOID reserved) noexcept;
} // namespace anadius::dll_link_gate
