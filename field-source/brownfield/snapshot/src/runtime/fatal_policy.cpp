#include "preloader_l/runtime/fatal_policy.hpp"
#include "preloader_l/runtime/exception_runtime.hpp"

namespace preloader_l { namespace runtime {

FatalRecipe fatal_recipe(FatalFamily family) noexcept {
    switch (family) {
    case FatalFamily::security_failure:
        return {status_stack_buffer_overrun, exception_noncontinuable,
                0u, status_stack_buffer_overrun,
                false, true, true, true, false};
    case FatalFamily::invalid_parameter:
        return {status_invalid_crt_parameter, exception_noncontinuable,
                fast_fail_invalid_arg, status_invalid_crt_parameter,
                true, true, true, true, false};
    case FatalFamily::fatal_app_exit:
        return {status_fatal_app_exit, exception_noncontinuable,
                fast_fail_fatal_app_exit, 3u,
                true, true, true, false, true};
    }
    return {};
}

void execute_fatal_recipe(FatalFamily family,
                          const FatalPlatformOps& ops,
                          FatalExceptionInput input) noexcept {
    const FatalRecipe r = fatal_recipe(family);
    if (r.uses_fast_fail_when_available && ops.fast_fail_available &&
        ops.fast_fail_available(ops.context) && ops.fast_fail) {
        ops.fast_fail(ops.context, r.fast_fail_code);
        // A production fast-fail does not return. A mock may return so that the
        // deterministic fallback can still be validated.
    }
    if (r.clears_top_level_filter && ops.clear_top_level_filter)
        ops.clear_top_level_filter(ops.context);
    if (r.reports_unhandled_exception && ops.report_unhandled)
        ops.report_unhandled(ops.context, r.exception_status, r.exception_flags,
                             input.supplied_exception_pointers);
    if (r.hard_terminate_process && ops.terminate_process)
        ops.terminate_process(ops.context, r.process_exit_code);
    if (r.enters_common_exit_engine && ops.exit_process)
        ops.exit_process(ops.context, r.process_exit_code);
}

void dispatch_terminate_then_abort(const FatalPlatformOps& ops) noexcept {
    if (RuntimeTerminateHandler handler = current_terminate_handler()) handler();
    execute_fatal_recipe(FatalFamily::fatal_app_exit, ops);
}

void dispatch_invalid_parameter(const wchar_t* expression,
                                const wchar_t* function_name,
                                const wchar_t* file_name,
                                unsigned int line_number,
                                uintptr_t reserved,
                                const FatalPlatformOps& ops) noexcept {
    if (RuntimeInvalidParameterHandler handler = current_invalid_parameter_handler()) {
        handler(expression, function_name, file_name, line_number, reserved);
        return;
    }
    execute_fatal_recipe(FatalFamily::invalid_parameter, ops);
}

} }
