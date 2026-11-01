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
        active_curio_htons_stage_12980;
    const unsigned integrated_expected =
        active_integrated_ae00_store_entitlements |
        active_integrated_native_crypt_query |
        active_integrated_registry_stage_13a70 |
        active_integrated_service_stage_7000 |
        active_integrated_htons_stage_12980;
    const unsigned active_expected =
        active_runtime_active_ae00_store_entitlements |
        active_runtime_active_native_crypt_query |
        active_runtime_active_registry_stage_13a70 |
        active_runtime_active_service_stage_7000 |
        active_runtime_active_htons_stage_12980;
    const unsigned consistency_expected =
        active_consistency_open_registry_hook_scope_13a70 |
        active_consistency_open_service_object_layout_7000 |
        active_consistency_open_htons_hook_engine_12980 |
        active_consistency_open_native_crypt_query_side_effects |
        active_consistency_open_packaging_mapper_4eba00;
    const unsigned unresolved_expected =
        active_unresolved_protected_handoff_44c5c3 |
        active_unresolved_protected_entry_11a10 |
        active_unresolved_vector_ae00 |
        active_unresolved_vector_d5f0;

    if (s.curio_mask != curio_expected) return 1;
    if (s.integrated_mask != integrated_expected) return 2;
    if (s.runtime_active_mask != active_expected) return 3;
    if (s.consistency_open_mask != consistency_expected) return 4;
    if (s.unresolved_mask != unresolved_expected) return 5;
    if (s.unresolved_mask & active_unresolved_hook_stage_13a70) return 8;
    if (s.unresolved_mask & active_unresolved_service_stage_7000) return 9;
    if (s.unresolved_mask & active_unresolved_hook_stage_12980) return 10;
    if (s.unresolved_mask & active_unresolved_manual_mapper_4eba00) return 11;
    if (!(s.consistency_open_mask & active_consistency_open_packaging_mapper_4eba00)) return 12;
    return 0;
}
