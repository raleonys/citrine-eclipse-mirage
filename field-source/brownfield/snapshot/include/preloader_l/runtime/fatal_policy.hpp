#pragma once
#include <stdint.h>

namespace preloader_l { namespace runtime {

enum class FatalFamily : uint32_t {
    security_failure = 0,
    invalid_parameter = 1,
    fatal_app_exit = 2,
};

struct FatalRecipe {
    uint32_t exception_status;
    uint32_t exception_flags;
    uint32_t fast_fail_code;
    uint32_t process_exit_code;
    bool uses_fast_fail_when_available;
    bool clears_top_level_filter;
    bool reports_unhandled_exception;
    bool hard_terminate_process;
    bool enters_common_exit_engine;
};

constexpr uint32_t status_stack_buffer_overrun = 0xC0000409u;
constexpr uint32_t status_invalid_crt_parameter = 0xC0000417u;
constexpr uint32_t status_fatal_app_exit = 0x40000015u;
constexpr uint32_t exception_noncontinuable = 0x1u;
constexpr uint32_t fast_fail_invalid_arg = 5u;
constexpr uint32_t fast_fail_fatal_app_exit = 7u;

FatalRecipe fatal_recipe(FatalFamily family) noexcept;

// Opaque hand-off for the security-failure terminal. The curio binary's
// 0x48AA0 terminal does not manufacture a replacement EXCEPTION_POINTERS: it
// preserves the structure supplied by its upstream builder. Windows code owns
// the concrete EXCEPTION_POINTERS type; portable policy keeps only identity.
struct FatalExceptionInput {
    const void* supplied_exception_pointers;
};

struct FatalPlatformOps {
    void* context;
    bool (*fast_fail_available)(void*) noexcept;
    // Test/platform seam: production implementation does not return from the
    // actual fail-fast instruction, but the callback type permits mocks.
    void (*fast_fail)(void*, uint32_t code);
    void (*clear_top_level_filter)(void*) noexcept;
    // When supplied_exception_pointers is non-null, platform code must forward
    // that exact object rather than replacing it. A null pointer requests the
    // generalized reportfault path, including a captured runtime CONTEXT.
    void (*report_unhandled)(void*, uint32_t status, uint32_t flags,
                             const void* supplied_exception_pointers) noexcept;
    void (*terminate_process)(void*, uint32_t code) noexcept;
    void (*exit_process)(void*, uint32_t code) noexcept;
};

void execute_fatal_recipe(FatalFamily family,
                          const FatalPlatformOps& ops,
                          FatalExceptionInput input = {nullptr}) noexcept;
void dispatch_terminate_then_abort(const FatalPlatformOps& ops) noexcept;
void dispatch_invalid_parameter(const wchar_t* expression,
                                const wchar_t* function_name,
                                const wchar_t* file_name,
                                unsigned int line_number,
                                uintptr_t reserved,
                                const FatalPlatformOps& ops) noexcept;

} }
