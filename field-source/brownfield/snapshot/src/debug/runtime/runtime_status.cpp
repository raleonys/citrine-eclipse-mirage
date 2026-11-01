#include "preloader_l/runtime/runtime_status.hpp"
#include "preloader_l/runtime/consistency_closure.hpp"
#include "runtime/strict_consistency_status.hpp"

namespace preloader_l { namespace runtime {
namespace {
volatile uintptr_t g_module = 0;
volatile uint32_t g_attach_count = 0;

constexpr uint32_t k_curio =
    active_curio_ae00_store_entitlements |
    active_curio_native_crypt_query |
    active_curio_registry_stage_13a70 |
    active_curio_service_stage_7000 |
    active_curio_htons_stage_12980 |
    active_curio_d5f0_store_payload |
    active_curio_ae00_aux_spans |
    active_curio_optional_host_hook_6b31 |
    active_curio_optional_hook_installer_11a38;

constexpr uint32_t k_integrated =
    active_integrated_ae00_store_entitlements |
    active_integrated_native_crypt_query |
    active_integrated_registry_stage_13a70 |
    active_integrated_service_stage_7000 |
    active_integrated_htons_stage_12980 |
    active_integrated_d5f0_store_payload |
    active_integrated_ae00_aux_spans;

// Curio registry policies are installed during the Windows bootstrap.
// The release-server runtime uses Winsock 2.2 startup, listener selection and
// the curio 0x558/0x548 ownership geometry with alias-owner strong/weak
// lifetime. Native hook targets use process-wide export-entry detours with
// relocation-aware trampoline mapping, page-protection coordination and the
// curio allocator/free-range placement model. Strict/model debt remains
// separately accounted instead of treating current-host blocker-zero as consistency.
constexpr uint32_t k_runtime_active =
    active_runtime_active_ae00_store_entitlements |
    active_runtime_active_native_crypt_query |
    active_runtime_active_registry_stage_13a70 |
    active_runtime_active_service_stage_7000 |
    active_runtime_active_htons_stage_12980 |
    active_runtime_active_ae00_aux_spans;

// Production consistency-open state is derived from the evidence-backed closure
// registry. Historical bit identities remain trace-compatible but are not
// emitted as current production debt. The optional host-signature chain is
// retained as curio evidence/debug tooling only; it is not source-integrated
// or runtime-active in the normalized production target.
constexpr uint32_t k_unresolved = 0u;
}

void note_active_attach(void* module) noexcept {
    g_module = reinterpret_cast<uintptr_t>(module);
    ++g_attach_count;
}


ActiveRuntimeSnapshot active_runtime_snapshot() noexcept {
    return ActiveRuntimeSnapshot{
        g_module,
        g_attach_count,
        k_unresolved,
        k_curio,
        k_integrated,
        k_runtime_active,
        active_consistency_closure_snapshot().production_open_mask,
        active_strict_consistency_status_snapshot().strict_open_mask
    };
}

} }
