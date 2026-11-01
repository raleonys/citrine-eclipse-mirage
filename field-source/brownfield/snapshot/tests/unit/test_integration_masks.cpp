#include "preloader_l/runtime/runtime_status.hpp"
using namespace preloader_l::runtime;

static_assert(active_curio_ae00_store_entitlements == (1u << 0), "curio bit drift");
static_assert(active_curio_native_crypt_query == (1u << 3), "curio bit drift");
static_assert(active_curio_registry_stage_13a70 == (1u << 4), "curio bit drift");
static_assert(active_curio_service_stage_7000 == (1u << 5), "curio bit drift");
static_assert(active_curio_htons_stage_12980 == (1u << 6), "curio bit drift");

int main()
{
    const auto s = active_runtime_snapshot();
    const unsigned curio_expected =
        active_curio_ae00_store_entitlements |
        active_curio_native_crypt_query |
        active_curio_registry_stage_13a70 |
        active_curio_service_stage_7000 |
        active_curio_htons_stage_12980 |
        active_curio_d5f0_store_payload |
        active_curio_ae00_aux_spans |
        active_curio_optional_host_hook_6b31 |
        active_curio_optional_hook_installer_11a38;
    const unsigned integrated_expected =
        active_integrated_ae00_store_entitlements |
        active_integrated_native_crypt_query |
        active_integrated_registry_stage_13a70 |
        active_integrated_service_stage_7000 |
        active_integrated_htons_stage_12980 |
        active_integrated_d5f0_store_payload |
        active_integrated_ae00_aux_spans;
    const unsigned active_expected =
        active_runtime_active_ae00_store_entitlements |
        active_runtime_active_native_crypt_query |
        active_runtime_active_registry_stage_13a70 |
        active_runtime_active_service_stage_7000 |
        active_runtime_active_htons_stage_12980 |
        active_runtime_active_ae00_aux_spans;

    if (s.curio_mask != curio_expected) return 1;
    if (s.integrated_mask != integrated_expected) return 2;
    if (s.runtime_active_mask != active_expected) return 3;
    if (s.consistency_open_mask != 0u) return 4;
    if (s.unresolved_mask != 0u) return 5;
    if (s.strict_model_consistency_open_mask != 0u) return 6;
    if (!(s.curio_mask & active_curio_optional_host_hook_6b31)) return 9;
    if (!(s.curio_mask & active_curio_optional_hook_installer_11a38)) return 10;
    if (s.integrated_mask & active_integrated_optional_host_scanner_6b31) return 11;
    return 0;
}
