#pragma once
#include "../anadius-bootstrap-runtime-source/factory.hpp"

namespace anadius::dll_link_gate {
// REQUIRED production definitions, deliberately absent. These are new source
// integration contracts, not recovered native functions or callable image VAs.
// A definition must own the complete Resources graph, permanent Errors, seed,
// callbacks and worker lifetimes. No test fixture, null runtime, success stub,
// hidden original-DLL fallback or separately initialized recovered CRT qualifies.
//
// The existing Runtime control contract is OUTSIDE loader lock. Merely creating
// and publishing Runtime here violates it. The production implementation must
// first resolve that conflict, constructor failure/attach rollback, and original
// initialization order. The missing definition makes the DLL link fail today.
// A null result is permitted ONLY after complete attach rollback and exclusion
// of future callbacks. It means DLL load failure, never successful startup.
bootstrap_runtime::Runtime* require_loader_attach_runtime(
    HINSTANCE module, LPVOID reserved) noexcept;

// Loader detach cannot join workers or wait for their loader notifications.
// A production definition must establish pre-detach quiescence, distinguish
// FreeLibrary from process termination, and cover failed attach and CRT teardown.
// Calling Runtime::close() here is not an acceptable unproven implementation.
void require_loader_detach_quiescence(HINSTANCE module, LPVOID reserved) noexcept;
} // namespace anadius::dll_link_gate
