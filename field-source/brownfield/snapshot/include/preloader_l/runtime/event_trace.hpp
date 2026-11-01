#pragma once
#include <stddef.h>
#include <stdint.h>

#ifndef PRELOADER_L_TRACE_DIAGNOSTICS
#define PRELOADER_L_TRACE_DIAGNOSTICS 0
#endif

namespace preloader_l { namespace runtime {

enum class TraceEvent : uint16_t {
    // Established trace numeric contract: 0..65.
    none = 0,
    process_attach,
    bootstrap_scheduled,
    worker_started,
    runtime_initialized,
    listener_active,
    client_accepted,
    bytes_received,
    frame_dispatched,
    response_sent,
    peer_closed,
    shutdown_started,
    shutdown_complete,
    bootstrap_complete,
    failure,
    secondary_init_begin,
    crypt_observer_status,
    secondary_init_complete,
    reconstructed_identity_ready,
    reconstructed_identity_branch,
    crypt_observer_sweep,
    crypt_query_call,
    crypt_query_object_type,
    crypt_query_expected_content,
    crypt_query_expected_format,
    crypt_query_flags,
    crypt_query_caller_rva,
    crypt_query_caller_is_main,
    crypt_query_path_read,
    crypt_query_path_matches_model,
    crypt_query_result,
    crypt_query_error,
    crypt_query_encoding_type,
    crypt_query_content_type,
    crypt_query_format_type,
    crypt_query_message_nonnull,
    crypt_query_code_window_read,
    crypt_query_call_length,
    crypt_query_rdx_producer_kind,
    crypt_query_rdx_source_register,
    crypt_query_path_restore_status,
    crypt_query_path_restore_length,
    crypt_msg_observer_status,
    crypt_msg_observer_sweep,
    crypt_msg_call,
    crypt_msg_param_type,
    crypt_msg_index,
    crypt_msg_message_nonnull,
    crypt_msg_size_before,
    crypt_msg_size_after,
    crypt_msg_caller_rva,
    crypt_msg_caller_is_main,
    crypt_msg_result,
    crypt_msg_error,
    crypt_msg_null_guard,
    cert_find_observer_status,
    cert_find_observer_sweep,
    cert_find_call,
    cert_find_encoding_type,
    cert_find_flags,
    cert_find_type,
    cert_find_caller_rva,
    cert_find_caller_is_main,
    cert_find_result,
    cert_find_error,
    cert_find_context_nonnull,

    // Network redirect append-only additions.
    connect_redirect_status = 66,
    connect_selected_port,
    connect_redirect_sweep,
    connect_redirect_mapped,

    // Runtime-configuration/environment append-only additions.
    runtime_configuration_directory = 70,
    runtime_configuration_status,
    runtime_configuration_sections,
    environment_policy_status,
    environment_written_mask,
    environment_preserved_mask,

    // Registry bridge append-only event.
    registry_iat_hook_status = 76,

    // Session/runtime append-only events.
    persistent_session_closed = 77,
    reserved_compatibility_78,
    reserved_compatibility_79,
    reserved_compatibility_80,
    active_unresolved_mask,
    parallel_server_started,
    client_capacity_rejected,

    // Append-only evidence/recovery events.
    store_parse_status = 84,
    store_present,
    store_entitlement_count,
    active_curio_mask,

    // Feature-completeness state split; append-only.
    active_integrated_mask = 88,
    active_runtime_active_mask,

    // Feature-complete-but-strict-consistency-open state.
    active_consistency_open_mask,

    // Curio optional-host 0x6B31 scanner telemetry.
    optional_host_compat_scan_status = 95,
    optional_host_compat_hits,
    optional_host_compat_first_rva,
};

struct TraceRecord {
    uint32_t sequence;
    TraceEvent event;
    uint16_t reserved;
    uint32_t value;
};

constexpr size_t trace_capacity = 64;

// Numeric IDs are a compatibility contract. Never insert new events into an
// older range; append them and pin important boundaries.
static_assert(static_cast<uint16_t>(TraceEvent::runtime_initialized) == 4,
              "golden trace ID drift");
static_assert(static_cast<uint16_t>(TraceEvent::listener_active) == 5,
              "golden trace ID drift");
static_assert(static_cast<uint16_t>(TraceEvent::cert_find_context_nonnull) == 65,
              "golden trace ID drift");
static_assert(static_cast<uint16_t>(TraceEvent::connect_redirect_status) == 66,
              "connect redirect trace ID drift");
static_assert(static_cast<uint16_t>(TraceEvent::connect_redirect_sweep) == 68,
              "connect redirect trace ID drift");
static_assert(static_cast<uint16_t>(TraceEvent::runtime_configuration_directory) == 70,
              "runtime-configuration trace ID drift");
static_assert(static_cast<uint16_t>(TraceEvent::environment_preserved_mask) == 75,
              "runtime-configuration trace ID drift");
static_assert(static_cast<uint16_t>(TraceEvent::registry_iat_hook_status) == 76,
              "registry bridge trace ID drift");
static_assert(static_cast<uint16_t>(TraceEvent::client_capacity_rejected) == 83,
              "session/runtime trace ID drift");
static_assert(static_cast<uint16_t>(TraceEvent::store_parse_status) == 84,
              "store parser trace ID drift");
static_assert(static_cast<uint16_t>(TraceEvent::active_curio_mask) == 87,
              "store parser trace ID drift");
static_assert(static_cast<uint16_t>(TraceEvent::active_integrated_mask) == 88,
              "feature-state trace ID drift");
static_assert(static_cast<uint16_t>(TraceEvent::active_runtime_active_mask) == 89,
              "feature-state trace ID drift");
static_assert(static_cast<uint16_t>(TraceEvent::active_consistency_open_mask) == 90,
              "consistency-open trace ID drift");
static_assert(static_cast<uint16_t>(TraceEvent::optional_host_compat_scan_status) == 95,
              "optional-host trace ID drift");
static_assert(static_cast<uint16_t>(TraceEvent::optional_host_compat_first_rva) == 97,
              "optional-host trace ID drift");

#if PRELOADER_L_TRACE_DIAGNOSTICS
void reset_trace_for_test() noexcept;
void note_trace(TraceEvent event, uint32_t value = 0) noexcept;
size_t snapshot_trace(TraceRecord* output, size_t capacity) noexcept;
const char* trace_event_name(TraceEvent event) noexcept;
#else
inline void reset_trace_for_test() noexcept {}
inline void note_trace(TraceEvent, uint32_t = 0) noexcept {}
inline size_t snapshot_trace(TraceRecord*, size_t) noexcept { return 0u; }
inline const char* trace_event_name(TraceEvent) noexcept { return "none"; }
#endif

} }
