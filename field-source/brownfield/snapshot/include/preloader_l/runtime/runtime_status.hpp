#pragma once
#include <stdint.h>

namespace preloader_l { namespace runtime {

// Curio evidence-backed semantics from the model binary family.
// Recovery, source integration, runtime activation, and strict implementation
// consistency are tracked independently. A curio bit alone never means that a
// production bootstrap currently activates that behavior.
enum ActiveCurioMask : uint32_t {
    active_curio_ae00_store_entitlements = 1u << 0,
    active_curio_native_crypt_query = 1u << 3,
    active_curio_registry_stage_13a70 = 1u << 4,
    active_curio_service_stage_7000 = 1u << 5,
    active_curio_htons_stage_12980 = 1u << 6,
    active_curio_d5f0_store_payload = 1u << 7,
    active_curio_ae00_aux_spans = 1u << 8,
    // Protected active/11A10 paths are semantically classified
    // as optional host-signature compatibility hook infrastructure.
    active_curio_optional_host_hook_6b31 = 1u << 9,
    active_curio_optional_hook_installer_11a38 = 1u << 10,
};

// Same bit layout as ActiveCurioMask. "Integrated" means a production
// source implementation is present/compiled; it does not by itself imply that
// the current bootstrap invokes or installs it.
enum ActiveIntegratedMask : uint32_t {
    active_integrated_ae00_store_entitlements = 1u << 0,
    active_integrated_native_crypt_query = 1u << 3,
    active_integrated_registry_stage_13a70 = 1u << 4,
    active_integrated_service_stage_7000 = 1u << 5,
    active_integrated_htons_stage_12980 = 1u << 6,
    active_integrated_d5f0_store_payload = 1u << 7,
    active_integrated_ae00_aux_spans = 1u << 8,
    // Stable source/debug identifier for the curio 44-byte wildcard scanner.
    // The normalized production target does not emit this integrated bit.
    active_integrated_optional_host_scanner_6b31 = 1u << 9,
};

// Same bit layout again. These bits report that a production lifecycle path
// actively installs/invokes the curio semantic implementation. Per-run
// installation success is telemetry (hook snapshot/trace), not encoded here.
// This is not a byte-for-byte claim about the original hook/object ABI.
enum ActiveRuntimeActiveMask : uint32_t {
    active_runtime_active_ae00_store_entitlements = 1u << 0,
    active_runtime_active_native_crypt_query = 1u << 3,
    active_runtime_active_registry_stage_13a70 = 1u << 4,
    active_runtime_active_service_stage_7000 = 1u << 5,
    active_runtime_active_htons_stage_12980 = 1u << 6,
    active_runtime_active_ae00_aux_spans = 1u << 8,
    // Stable source/debug identifier for the historical scanner placement.
    // The normalized production target does not invoke the scanner or emit this bit.
    active_runtime_active_optional_host_scanner_6b31 = 1u << 9,
};

// Historical strict-consistency debt identifiers retained for trace/source compatibility.
// Current production state classifies each formerly emitted debt using first-party
// evidence: observable-contract-closed private identity/topology, current-host
// not-applicable optional compatibility, or packaging outside the normalized DLL
// target. The identifiers remain stable, but the production consistency-open mask is 0.
enum ActiveConsistencyOpenMask : uint32_t {
    // Historical hook-engine bit positions retained for trace/source
    // compatibility. The current model covers relocation/IP mapping and the
    // model VEH + page-trap coordination, plus observable
    // allocator/free-range placement semantics. These historical bits are no
    // longer emitted by the current runtime consistency-open mask.
    active_consistency_open_registry_hook_allocator_13a70 = 1u << 0,
    active_consistency_open_registry_hook_thread_trap_allocator_13a70 = active_consistency_open_registry_hook_allocator_13a70,
    active_consistency_open_registry_hook_relocation_ip_allocator_13a70 = active_consistency_open_registry_hook_allocator_13a70,
    active_consistency_open_registry_hook_engine_13a70 = active_consistency_open_registry_hook_allocator_13a70,
    active_consistency_open_registry_hook_scope_13a70 = active_consistency_open_registry_hook_allocator_13a70,
    // The current model recovers InitOnce + temporary-owner lifetime and the
    // complete 0x558/0x548 physical map; the live path integrates alias-owner strong/
    // weak/four-slot dispatch and binds that model-shaped storage into
    // the live Windows lifetime. The remaining strict bit tracks compiler-
    // private allocator/free provenance and strict model vftable identity.
    active_consistency_open_service_binary_coallocation_vftable_identity_7000 = 1u << 1,
    active_consistency_open_service_object_abi_7000 = active_consistency_open_service_binary_coallocation_vftable_identity_7000,
    active_consistency_open_service_live_msvc_binding_7000 = active_consistency_open_service_binary_coallocation_vftable_identity_7000,
    active_consistency_open_service_object_layout_7000 = active_consistency_open_service_binary_coallocation_vftable_identity_7000,
    active_consistency_open_htons_hook_allocator_12980 = 1u << 2,
    active_consistency_open_htons_hook_thread_trap_allocator_12980 = active_consistency_open_htons_hook_allocator_12980,
    active_consistency_open_htons_hook_relocation_ip_allocator_12980 = active_consistency_open_htons_hook_allocator_12980,
    active_consistency_open_htons_hook_engine_12980 = active_consistency_open_htons_hook_allocator_12980,
    // RAVEN recovers observable argument mutation; 2.7.3 exact-target evidence
    // closes installer/original-slot/restore/tail-jump topology. The historical
    // bit remains defined only for trace/source compatibility.
    active_consistency_open_native_crypt_query_tailcall_topology = 1u << 5,
    active_consistency_open_native_crypt_query_side_effects = active_consistency_open_native_crypt_query_tailcall_topology,
    // 0x4EBA00 is a generic PE manual-mapping / protection-loader helper. 2.7.3
    // closes its exact target binary/callback contract; activation provenance
    // remains historical metadata, not an exact runtime debt.
    active_consistency_open_packaging_mapper_4eba00 = 1u << 6,
    // RAVEN recovers the physical 24-byte vector layout and release rules.
    // D5F0 exact output allocator binding remains open. AE00's native consumer,
    // allocator ancestry and forced-failure retry/new-handler path are closed by
    // 2.7.3 native evidence; its historical bit remains as a stable identifier.
    active_consistency_open_d5f0_vector_crt_allocator_binding = 1u << 7,
    active_consistency_open_d5f0_vector_object_abi = active_consistency_open_d5f0_vector_crt_allocator_binding,
    active_consistency_open_ae00_aux_vector_crt_allocator_binding = 1u << 8,
    active_consistency_open_ae00_aux_vector_object_abi = active_consistency_open_ae00_aux_vector_crt_allocator_binding,
    // Debug tooling reproduces the curio 44-byte scanner; normalized production does not invoke it.
    // The compatibility hit path still lacks the exact original hook-installer topology.
    // The evidence pins the 0x10150 outer shell, VM entry adapter and native epilogue.
    // VECTOR pins the arg2 DWORD store_payload byte-count output. MATRIX further
    // pins tested-path argument observability: all relocation cases preserve the
    // trace, arg1/3/4/5/6 may be NULL on that trace, and only arg2 pointee memory
    // is written. Broader 0x1019D semantics/lifecycle remain open. Keep the old symbol as an alias.
    active_consistency_open_optional_host_hit_installer_6b31 = 1u << 9,
    active_consistency_open_optional_host_hook_engine_6b31 = active_consistency_open_optional_host_hit_installer_6b31,
    active_consistency_open_optional_hook_installer_11a38 = 1u << 10,
    // The 0x1019D detour callback belongs to the same optional compatibility
    // host-signature compatibility chain. VECTOR recovers the arg2 DWORD store_payload byte-count output. MATRIX shows
    // arg1/3/4/5/6 pointee windows are not dereferenced on the controlled trace;
    // their semantic types remain open and the callback contract is still partial. The curio 44-byte
    // signature is absent from every supplied PE, so the remainder is compatibility
    // compatibility consistency debt rather than a current-host application gap.
    active_consistency_open_optional_host_callback_1019d = 1u << 11,
};

// Historical bit positions are retained for trace compatibility. A constant
// can exist without being present in the current unresolved mask. As of v1.8.5
// 13A70/7000/12980 are feature-complete at semantic/runtime level and are
// tracked in ActiveConsistencyOpenMask instead. 0x4EBA00 is retained below only as a
// historical trace bit; v1.8.7 reclassifies it as packaging/protection consistency.
enum ActiveUnresolvedMask : uint32_t {
    active_unresolved_hook_stage_13a70 = 1u << 0,
    active_unresolved_service_stage_7000 = 1u << 1,
    active_unresolved_hook_stage_12980 = 1u << 2,

    // Historical trace bits. The observable roles of the VM PCs are recovered
    // 0x6B31 and 0x11A38 as optional host-signature scan/hook-install
    // infrastructure, so these bits are no longer emitted as unresolved.
    active_unresolved_vm_segment_6b31 = 1u << 3,
    active_unresolved_vm_function_11a38 = 1u << 4,

    // Source-compatibility aliases retained for older trace/readout tooling.
    active_unresolved_protected_handoff_44c5c3 = active_unresolved_vm_segment_6b31,
    active_unresolved_protected_entry_11a10 = active_unresolved_vm_function_11a38,

    // Historical trace bit only; producer semantics are
    // curio/integrated/runtime-active and this bit is no longer emitted.
    active_unresolved_ae00_aux_vectors = 1u << 5,
    active_unresolved_d5f0_output_vector = 1u << 6,
    active_unresolved_manual_mapper_4eba00 = 1u << 7,

    // JAVELIN deep re-audit found a fifth direct .text -> .surge VM transfer
    // that older completeness masks omitted: native prologue 0x10150, E9 at
    // 0x1019C, envelope 0x4B0450, seeded VM PC 0x1019D. Dynamic
    // reachability: the curio host-signature path installs 0x10150 as its
    // detour callback reachability is proven. This historical bit remains for trace compatibility,
    // but does not emit it as a current-host feature gap: the scanner signature
    // has zero hits across the complete supplied PE dataset. Exact callback
    // semantics remain visible in ActiveConsistencyOpenMask.
    active_unresolved_vm_function_1019d = 1u << 8,

    // Historical aliases.
    active_unresolved_vector_ae00 = active_unresolved_ae00_aux_vectors,
    active_unresolved_vector_d5f0 = active_unresolved_d5f0_output_vector,
};

struct ActiveRuntimeSnapshot {
    uintptr_t module;
    uint32_t attach_count;
    uint32_t unresolved_mask;
    uint32_t curio_mask;
    uint32_t integrated_mask;
    uint32_t runtime_active_mask;
    // Compatibility/current-host blocker telemetry. This remains zero when the
    // supplied hosts have no production blocker; strict/model debt is
    // reported separately below.
    uint32_t consistency_open_mask;
    uint32_t strict_model_consistency_open_mask;
};

void note_active_attach(void* module) noexcept;
ActiveRuntimeSnapshot active_runtime_snapshot() noexcept;

} }
