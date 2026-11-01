#include "production_image.hpp"

// Candidate compiler-CRT application entry for a STATIC LINK DIAGNOSTIC ONLY.
// The toolchain supplies DllMainCRTStartup. Do not also call recovered
// Entry::entry_04bcfc: that would initialize a second CRT around this C++ graph.
// Neither original nor rebuilt DLL is loaded or executed by the build script.
extern "C" BOOL WINAPI DllMain(HINSTANCE module, DWORD reason, LPVOID reserved) noexcept {
    if (reason == DLL_PROCESS_ATTACH) {
        auto* runtime = anadius::dll_link_gate::require_loader_attach_runtime(module, reserved);
        if (!runtime) return FALSE; // factory must already have rolled back
        // Real strong source chain: Runtime -> RuntimeOwner -> Resolver ->
        // application_entry -> Bootstrap::application_dll_main -> initialize.
        // Non-null promises a live Bound runtime. noexcept prevents exceptions
        // escaping the DLL ABI; violating that phase contract is terminal.
        // Full native EH equivalence is not established by this candidate.
        return runtime->application_entry()(module, reason, reserved);
    }
    if (reason == DLL_PROCESS_DETACH)
        anadius::dll_link_gate::require_loader_detach_quiescence(module, reserved);
    // Recovered application main has no thread-attach/detach work. Compiler CRT
    // thread/TLS work remains owned by DllMainCRTStartup, not this projection.
    return TRUE;
}
