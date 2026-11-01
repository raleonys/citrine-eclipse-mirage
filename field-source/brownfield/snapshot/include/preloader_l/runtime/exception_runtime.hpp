#pragma once
#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace runtime {

constexpr uint32_t msvc_cpp_exception_code = 0xE06D7363u;
constexpr uint32_t access_violation_code = 0xC0000005u;

enum class SehFilterDecision : int32_t {
    continue_execution = -1,
    continue_search = 0,
    execute_handler = 1,
};

using CrtActionCallback = void (*)(int32_t signal_number);

struct CrtActionView {
    uint32_t exception_number;
    int32_t signal_number;
    uintptr_t action;
};

// Curio filter semantics. The default PTD table contains twelve
// zero-action records and no 0xE06D7363 row, so normal module-owned state
// resolves to CONTINUE_SEARCH.  Special values 1 and 5 reproduce the curio
// action policy; any other non-zero value is treated as an installed callback.
SehFilterDecision evaluate_crt_exception_filter(uint32_t exception_code,
                                                 const CrtActionView* table,
                                                 size_t count);
SehFilterDecision evaluate_current_crt_exception_filter(uint32_t exception_code);

struct NewHandlerEscapeContract {
    uint32_t active_roots;
    uint32_t callback_layers;
    uint32_t total_route_layers;
    uint32_t search_terminate;
    uint32_t target_unwind_terminate;
    uint32_t catch_funclet_executes;
    uint32_t outer_body_rsp_delta;
    uint32_t inner_body_rsp_delta;
};
NewHandlerEscapeContract new_handler_escape_contract() noexcept;

enum class StreamExceptionPolicy : uint32_t {
    catch_and_swallow = 0,
    set_badbit_conditional_rethrow = 1,
};
struct StreamExceptionOutcome {
    uint32_t resulting_state;
    bool rethrow_current_exception;
};
StreamExceptionOutcome evaluate_stream_exception_policy(StreamExceptionPolicy policy,
                                                         uint32_t current_state,
                                                         uint32_t badbit,
                                                         uint32_t exception_mask) noexcept;

// Source-level model of the curio AV-only continuation rule. The
// platform hook backend owns live CONTEXT mutation; this policy keeps the
// curio exact/page range semantics testable without copying machine code.
struct TrackedAvRange {
    uintptr_t source_page_begin;
    uintptr_t source_page_end;
    uintptr_t source_base;
    uintptr_t mapped_page_begin;
    uintptr_t mapped_page_end;
    uintptr_t mapped_base;
    size_t span;
};
struct TrackedAvDecision {
    SehFilterDecision decision;
    bool tracked;
    bool exact_source_span;
    bool rewrite_rip;
    uintptr_t mapped_rip;
};
TrackedAvDecision evaluate_tracked_av(uint32_t exception_code,
                                      uintptr_t accessed_address,
                                      uintptr_t rip,
                                      const TrackedAvRange* ranges,
                                      size_t count) noexcept;

// Curio handler-state consumers. The clean implementation does not invent
// image-local setters that were not recovered, but it preserves the consumer
// precedence and permits state installed through the runtime PTD/global seam.
using RuntimeTerminateHandler = void (*)();
using RuntimeInvalidParameterHandler = void (*)(const wchar_t* expression,
                                                 const wchar_t* function_name,
                                                 const wchar_t* file_name,
                                                 unsigned int line_number,
                                                 uintptr_t reserved);
RuntimeTerminateHandler current_terminate_handler() noexcept;
RuntimeInvalidParameterHandler current_invalid_parameter_handler() noexcept;
// Integration seam for the decoded process-global handler slot.  Normal
// module-owned state remains null because no image-local non-null installer was
// recovered; this is not exported as a feature surface.
RuntimeInvalidParameterHandler set_runtime_global_invalid_parameter_handler(
    RuntimeInvalidParameterHandler handler) noexcept;

struct ForwardExceptionRecord {
    uint32_t code;
    uint32_t flags;
    uint32_t parameter_count;
    const uintptr_t* information;
};
using RaiseExceptionCallback = void (*)(uint32_t code, uint32_t flags,
                                        uint32_t parameter_count,
                                        const uintptr_t* information,
                                        void* context);
void forward_exception_record(const ForwardExceptionRecord& record,
                              RaiseExceptionCallback raise,
                              void* context);

} }
