#include "preloader_l/runtime/event_trace.hpp"
#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#endif

namespace preloader_l { namespace runtime {
namespace {
struct Slot {
    // Internal ownership word. It is deliberately not part of TraceRecord ABI.
    // Writers own the whole payload publication interval and snapshots own the
    // whole payload copy interval, eliminating races on event/reserved/value.
    uint32_t ownership;
    uint32_t sequence;
    TraceEvent event;
    uint16_t reserved;
    uint32_t value;
};
uint32_t g_next = 0;
Slot g_slots[trace_capacity]{};

inline uint32_t load_acquire(const uint32_t* p) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    return static_cast<uint32_t>(_InterlockedCompareExchange(
        reinterpret_cast<volatile long*>(const_cast<uint32_t*>(p)), 0, 0));
#elif defined(__clang__) || defined(__GNUC__)
    return __atomic_load_n(p, __ATOMIC_ACQUIRE);
#else
    return *p;
#endif
}
inline void store_release(uint32_t* p, uint32_t v) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    (void)_InterlockedExchange(reinterpret_cast<volatile long*>(p),
                              static_cast<long>(v));
#elif defined(__clang__) || defined(__GNUC__)
    __atomic_store_n(p, v, __ATOMIC_RELEASE);
#else
    *p = v;
#endif
}
inline uint32_t fetch_add_relaxed(uint32_t* p, uint32_t v) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    return static_cast<uint32_t>(_InterlockedExchangeAdd(
        reinterpret_cast<volatile long*>(p), static_cast<long>(v)));
#elif defined(__clang__) || defined(__GNUC__)
    return __atomic_fetch_add(p, v, __ATOMIC_RELAXED);
#else
    const uint32_t old = *p; *p += v; return old;
#endif
}
inline bool try_lock_slot(uint32_t* p) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    return _InterlockedCompareExchange(
        reinterpret_cast<volatile long*>(p), 1, 0) == 0;
#elif defined(__clang__) || defined(__GNUC__)
    uint32_t expected = 0;
    return __atomic_compare_exchange_n(
        p, &expected, 1u, false, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED);
#else
    if (*p != 0) return false;
    *p = 1;
    return true;
#endif
}
inline void lock_slot(uint32_t* p) noexcept {
    while (!try_lock_slot(p)) {}
}
inline void unlock_slot(uint32_t* p) noexcept { store_release(p, 0u); }

// Sequence numbers that target the same slot differ by at most the ring size
// under ordinary concurrency. Signed modular ordering prevents a delayed old
// writer from overwriting a newer generation of the same slot.
inline bool sequence_is_newer(uint32_t candidate, uint32_t current) noexcept {
    return current == 0u || static_cast<int32_t>(candidate - current) > 0;
}
}

void reset_trace_for_test() noexcept
{
    store_release(&g_next, 0u);
    for (size_t i = 0; i < trace_capacity; ++i) {
        // reset_trace_for_test() is a quiescent test-only operation.
        store_release(&g_slots[i].ownership, 0u);
        g_slots[i].event = TraceEvent::none;
        g_slots[i].reserved = 0;
        g_slots[i].value = 0;
        store_release(&g_slots[i].sequence, 0u);
    }
}

void note_trace(TraceEvent event, uint32_t value) noexcept
{
    const uint32_t seq = fetch_add_relaxed(&g_next, 1u) + 1u;
    Slot& slot = g_slots[(seq - 1u) % trace_capacity];
    lock_slot(&slot.ownership);
    const uint32_t current = load_acquire(&slot.sequence);
    if (sequence_is_newer(seq, current)) {
        slot.event = event;
        slot.reserved = 0;
        slot.value = value;
        store_release(&slot.sequence, seq);
    }
    unlock_slot(&slot.ownership);
}

size_t snapshot_trace(TraceRecord* output, size_t capacity) noexcept
{
    if (!output || capacity == 0) return 0;
    const uint32_t next = load_acquire(&g_next);
    const uint32_t first = next > trace_capacity ? next - static_cast<uint32_t>(trace_capacity) + 1u : 1u;
    size_t written = 0;
    for (uint32_t seq = first; seq <= next && written < capacity; ++seq) {
        Slot& slot = g_slots[(seq - 1u) % trace_capacity];
        // Do not race a writer for diagnostics. A busy slot can be omitted from
        // this bounded snapshot and observed on the next one.
        if (!try_lock_slot(&slot.ownership)) continue;
        const uint32_t observed = load_acquire(&slot.sequence);
        if (observed == seq)
            output[written++] = TraceRecord{observed, slot.event, slot.reserved, slot.value};
        unlock_slot(&slot.ownership);
    }
    return written;
}

const char* trace_event_name(TraceEvent event) noexcept
{
    switch (event) {
        case TraceEvent::process_attach: return "process_attach";
        case TraceEvent::bootstrap_scheduled: return "bootstrap_scheduled";
        case TraceEvent::worker_started: return "worker_started";
        case TraceEvent::runtime_configuration_directory: return "runtime_configuration_directory";
        case TraceEvent::runtime_configuration_status: return "runtime_configuration_status";
        case TraceEvent::runtime_configuration_sections: return "runtime_configuration_sections";
        case TraceEvent::environment_policy_status: return "environment_policy_status";
        case TraceEvent::environment_written_mask: return "environment_written_mask";
        case TraceEvent::environment_preserved_mask: return "environment_preserved_mask";
        case TraceEvent::runtime_initialized: return "runtime_initialized";
        case TraceEvent::listener_active: return "listener_active";
        case TraceEvent::client_accepted: return "client_accepted";
        case TraceEvent::bytes_received: return "bytes_received";
        case TraceEvent::frame_dispatched: return "frame_dispatched";
        case TraceEvent::response_sent: return "response_sent";
        case TraceEvent::peer_closed: return "peer_closed";
        case TraceEvent::shutdown_started: return "shutdown_started";
        case TraceEvent::shutdown_complete: return "shutdown_complete";
        case TraceEvent::bootstrap_complete: return "bootstrap_complete";
        case TraceEvent::failure: return "failure";
        case TraceEvent::secondary_init_begin: return "secondary_init_begin";
        case TraceEvent::crypt_observer_status: return "crypt_observer_status";
        case TraceEvent::secondary_init_complete: return "secondary_init_complete";
        case TraceEvent::reconstructed_identity_ready: return "reconstructed_identity_ready";
        case TraceEvent::reconstructed_identity_branch: return "reconstructed_identity_branch";
        case TraceEvent::crypt_observer_sweep: return "crypt_observer_sweep";
        case TraceEvent::crypt_query_call: return "crypt_query_call";
        case TraceEvent::crypt_query_object_type: return "crypt_query_object_type";
        case TraceEvent::crypt_query_expected_content: return "crypt_query_expected_content";
        case TraceEvent::crypt_query_expected_format: return "crypt_query_expected_format";
        case TraceEvent::crypt_query_flags: return "crypt_query_flags";
        case TraceEvent::crypt_query_caller_rva: return "crypt_query_caller_rva";
        case TraceEvent::crypt_query_caller_is_main: return "crypt_query_caller_is_main";
        case TraceEvent::crypt_query_path_read: return "crypt_query_path_read";
        case TraceEvent::crypt_query_path_matches_model: return "crypt_query_path_matches_model";
        case TraceEvent::crypt_query_result: return "crypt_query_result";
        case TraceEvent::crypt_query_error: return "crypt_query_error";
        case TraceEvent::crypt_query_encoding_type: return "crypt_query_encoding_type";
        case TraceEvent::crypt_query_content_type: return "crypt_query_content_type";
        case TraceEvent::crypt_query_format_type: return "crypt_query_format_type";
        case TraceEvent::crypt_query_message_nonnull: return "crypt_query_message_nonnull";
        case TraceEvent::crypt_query_code_window_read: return "crypt_query_code_window_read";
        case TraceEvent::crypt_query_call_length: return "crypt_query_call_length";
        case TraceEvent::crypt_query_rdx_producer_kind: return "crypt_query_rdx_producer_kind";
        case TraceEvent::crypt_query_rdx_source_register: return "crypt_query_rdx_source_register";
        case TraceEvent::crypt_query_path_restore_status: return "crypt_query_path_restore_status";
        case TraceEvent::crypt_query_path_restore_length: return "crypt_query_path_restore_length";
        case TraceEvent::crypt_msg_observer_status: return "crypt_msg_observer_status";
        case TraceEvent::crypt_msg_observer_sweep: return "crypt_msg_observer_sweep";
        case TraceEvent::crypt_msg_call: return "crypt_msg_call";
        case TraceEvent::crypt_msg_param_type: return "crypt_msg_param_type";
        case TraceEvent::crypt_msg_index: return "crypt_msg_index";
        case TraceEvent::crypt_msg_message_nonnull: return "crypt_msg_message_nonnull";
        case TraceEvent::crypt_msg_size_before: return "crypt_msg_size_before";
        case TraceEvent::crypt_msg_size_after: return "crypt_msg_size_after";
        case TraceEvent::crypt_msg_caller_rva: return "crypt_msg_caller_rva";
        case TraceEvent::crypt_msg_caller_is_main: return "crypt_msg_caller_is_main";
        case TraceEvent::crypt_msg_result: return "crypt_msg_result";
        case TraceEvent::crypt_msg_error: return "crypt_msg_error";
        case TraceEvent::crypt_msg_null_guard: return "crypt_msg_null_guard";
        case TraceEvent::cert_find_observer_status: return "cert_find_observer_status";
        case TraceEvent::cert_find_observer_sweep: return "cert_find_observer_sweep";
        case TraceEvent::cert_find_call: return "cert_find_call";
        case TraceEvent::cert_find_encoding_type: return "cert_find_encoding_type";
        case TraceEvent::cert_find_flags: return "cert_find_flags";
        case TraceEvent::cert_find_type: return "cert_find_type";
        case TraceEvent::cert_find_caller_rva: return "cert_find_caller_rva";
        case TraceEvent::cert_find_caller_is_main: return "cert_find_caller_is_main";
        case TraceEvent::cert_find_result: return "cert_find_result";
        case TraceEvent::cert_find_error: return "cert_find_error";
        case TraceEvent::cert_find_context_nonnull: return "cert_find_context_nonnull";
        case TraceEvent::registry_iat_hook_status: return "registry_iat_hook_status";
        case TraceEvent::connect_redirect_status: return "connect_redirect_status";
        case TraceEvent::connect_selected_port: return "connect_selected_port";
        case TraceEvent::connect_redirect_mapped: return "connect_redirect_mapped";
        case TraceEvent::persistent_session_closed: return "persistent_session_closed";
        case TraceEvent::active_unresolved_mask: return "active_unresolved_mask";
        case TraceEvent::parallel_server_started: return "parallel_server_started";
        case TraceEvent::client_capacity_rejected: return "client_capacity_rejected";
        case TraceEvent::connect_redirect_sweep: return "connect_redirect_sweep";
        case TraceEvent::store_parse_status: return "store_parse_status";
        case TraceEvent::store_present: return "store_present";
        case TraceEvent::store_entitlement_count: return "store_entitlement_count";
        case TraceEvent::active_curio_mask: return "active_curio_mask";
        case TraceEvent::active_integrated_mask: return "active_integrated_mask";
        case TraceEvent::active_runtime_active_mask: return "active_runtime_active_mask";
        case TraceEvent::active_consistency_open_mask: return "active_consistency_open_mask";
        case TraceEvent::optional_host_compat_scan_status: return "optional_host_compat_scan_status";
        case TraceEvent::optional_host_compat_hits: return "optional_host_compat_hits";
        case TraceEvent::optional_host_compat_first_rva: return "optional_host_compat_first_rva";
        default: return "none";
    }
}

} }
