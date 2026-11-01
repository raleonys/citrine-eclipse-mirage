#include "preloader_l/runtime/exception_runtime.hpp"
#include "preloader_l/runtime/ptd_runtime.hpp"

namespace preloader_l { namespace runtime {
namespace {

template <typename T>
T& ptd_field(void* ptd, size_t offset) noexcept {
    return *reinterpret_cast<T*>(static_cast<unsigned char*>(ptd) + offset);
}

RuntimeInvalidParameterHandler g_invalid_parameter_handler = nullptr;

} // namespace

SehFilterDecision evaluate_crt_exception_filter(uint32_t exception_code,
                                                 const CrtActionView* table,
                                                 size_t count) {
    // The curio 0x59EFC gate is unconditional: every non-MSVC-C++ code
    // continues search before the action table is consulted.
    if (exception_code != msvc_cpp_exception_code || !table || !count)
        return SehFilterDecision::continue_search;
    for (size_t i = 0; i < count; ++i) {
        const CrtActionView& row = table[i];
        if (row.exception_number != exception_code) continue;
        if (row.action == 0) return SehFilterDecision::continue_search;
        if (row.action == 5) return SehFilterDecision::execute_handler;
        if (row.action == 1) return SehFilterDecision::continue_execution;
        reinterpret_cast<CrtActionCallback>(row.action)(row.signal_number);
        return SehFilterDecision::continue_execution;
    }
    return SehFilterDecision::continue_search;
}

SehFilterDecision evaluate_current_crt_exception_filter(uint32_t exception_code) {
    void* ptd = current_ptd(true);
    if (!ptd) return SehFilterDecision::continue_search;
    const auto* table = ptd_field<const CrtActionView*>(ptd, ptd_offset::action_table);
    return evaluate_crt_exception_filter(exception_code, table,
                                         default_exception_action_count());
}

NewHandlerEscapeContract new_handler_escape_contract() noexcept {
    return {18u, 2u, 36u, 24u, 12u, 0u, 0x60u, 0x90u};
}

StreamExceptionOutcome evaluate_stream_exception_policy(StreamExceptionPolicy policy,
                                                         uint32_t current_state,
                                                         uint32_t badbit,
                                                         uint32_t exception_mask) noexcept {
    if (policy == StreamExceptionPolicy::catch_and_swallow)
        return {current_state, false};
    const uint32_t state = current_state | badbit;
    return {state, (state & exception_mask) != 0};
}

TrackedAvDecision evaluate_tracked_av(uint32_t exception_code,
                                      uintptr_t accessed_address,
                                      uintptr_t rip,
                                      const TrackedAvRange* ranges,
                                      size_t count) noexcept {
    TrackedAvDecision out{SehFilterDecision::continue_search, false, false, false, rip};
    if (exception_code != access_violation_code || !ranges) return out;
    for (size_t i = 0; i < count; ++i) {
        const auto& r = ranges[i];
        const bool exact = r.span && accessed_address >= r.source_base &&
                           accessed_address < r.source_base + r.span;
        const bool source_page = accessed_address >= r.source_page_begin &&
                                 accessed_address < r.source_page_end;
        const bool mapped_page = accessed_address >= r.mapped_page_begin &&
                                 accessed_address < r.mapped_page_end;
        if (!exact && !source_page && !mapped_page) continue;
        out.decision = SehFilterDecision::continue_execution;
        out.tracked = true;
        out.exact_source_span = exact;
        if (exact && rip >= r.source_base && rip < r.source_base + r.span) {
            out.rewrite_rip = true;
            out.mapped_rip = r.mapped_base + (rip - r.source_base);
        }
        return out;
    }
    return out;
}

RuntimeTerminateHandler current_terminate_handler() noexcept {
    void* ptd = current_ptd(false);
    if (!ptd) return nullptr;
    return ptd_field<RuntimeTerminateHandler>(ptd, ptd_offset::terminate_handler);
}

RuntimeInvalidParameterHandler current_invalid_parameter_handler() noexcept {
    if (void* ptd = current_ptd(false)) {
        auto local = ptd_field<RuntimeInvalidParameterHandler>(ptd, ptd_offset::invalid_parameter_handler);
        if (local) return local;
    }
    return g_invalid_parameter_handler;
}

RuntimeInvalidParameterHandler set_runtime_global_invalid_parameter_handler(
    RuntimeInvalidParameterHandler handler) noexcept {
    RuntimeInvalidParameterHandler previous = g_invalid_parameter_handler;
    g_invalid_parameter_handler = handler;
    return previous;
}

void forward_exception_record(const ForwardExceptionRecord& record,
                              RaiseExceptionCallback raise,
                              void* context) {
    if (!raise) return;
    raise(record.code, record.flags, record.parameter_count, record.information, context);
}

} }
