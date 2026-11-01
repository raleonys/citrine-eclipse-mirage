#if !defined(_WIN32)
#error preloader_l fatal-policy Windows backend is Windows-only.
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#if defined(_MSC_VER)
#include <intrin.h>
#endif
#include "preloader_l/runtime/fatal_termination.hpp"

namespace preloader_l { namespace runtime {
namespace {
bool fast_fail_available(void*) noexcept { return ::IsProcessorFeaturePresent(23) != FALSE; }
void fast_fail(void*, uint32_t code) {
#if defined(_MSC_VER)
    __fastfail(code);
#else
    (void)code;
#endif
}
void clear_uef(void*) noexcept { (void)::SetUnhandledExceptionFilter(nullptr); }

// The generalized reportfault path captures a live CONTEXT, unwinds the
// reportfault frame when unwind metadata is available, then publishes that
// context through EXCEPTION_POINTERS. An earlier provisional model incorrectly
// zero-initialized CONTEXT; the curio contract uses runtime capture. Supplied security-failure
// pointers are forwarded unchanged and never replaced by a synthetic object.
void report_uef(void*, uint32_t status, uint32_t flags,
                const void* supplied_exception_pointers) noexcept {
    if (supplied_exception_pointers) {
        auto* supplied = const_cast<EXCEPTION_POINTERS*>(
            static_cast<const EXCEPTION_POINTERS*>(supplied_exception_pointers));
        (void)::UnhandledExceptionFilter(supplied);
        return;
    }

    CONTEXT context{};
    ::RtlCaptureContext(&context);

#if defined(_M_X64) || defined(__x86_64__)
    // The curio reportfault helper performs a virtual unwind after capture
    // so the published address/context describe its caller rather than the
    // interior of the capture helper. Preserve that behavior when pdata/xdata
    // can resolve the current RIP; otherwise the captured context remains a
    // valid live fallback instead of the old all-zero structure.
    DWORD64 image_base = 0;
    PRUNTIME_FUNCTION function_entry =
        ::RtlLookupFunctionEntry(context.Rip, &image_base, nullptr);
    if (function_entry) {
        PVOID handler_data = nullptr;
        DWORD64 establisher_frame = 0;
        KNONVOLATILE_CONTEXT_POINTERS nonvolatile{};
        (void)::RtlVirtualUnwind(UNW_FLAG_NHANDLER,
                                 image_base,
                                 context.Rip,
                                 function_entry,
                                 &context,
                                 &handler_data,
                                 &establisher_frame,
                                 &nonvolatile);
    }
#endif

    EXCEPTION_RECORD record{};
    record.ExceptionCode = status;
    record.ExceptionFlags = flags;
#if defined(_M_X64) || defined(__x86_64__)
    record.ExceptionAddress = reinterpret_cast<PVOID>(context.Rip);
#endif

    EXCEPTION_POINTERS pointers{};
    pointers.ExceptionRecord = &record;
    pointers.ContextRecord = &context;
    (void)::IsDebuggerPresent();
    (void)::UnhandledExceptionFilter(&pointers);
}
void terminate_process(void*, uint32_t code) noexcept {
    (void)::TerminateProcess(::GetCurrentProcess(), code);
}
void exit_process(void*, uint32_t code) noexcept { ::ExitProcess(code); }
const FatalPlatformOps g_ops{nullptr,&fast_fail_available,&fast_fail,&clear_uef,&report_uef,&terminate_process,&exit_process};
}
const FatalPlatformOps& windows_fatal_platform_ops() noexcept { return g_ops; }

void dispatch_terminate_then_abort_windows() noexcept {
    dispatch_terminate_then_abort(windows_fatal_platform_ops());
}

void dispatch_invalid_parameter_windows(const wchar_t* expression,
                                        const wchar_t* function_name,
                                        const wchar_t* file_name,
                                        unsigned int line_number,
                                        uintptr_t reserved) noexcept {
    dispatch_invalid_parameter(expression, function_name, file_name, line_number,
                               reserved, windows_fatal_platform_ops());
}

void dispatch_security_failure_windows(const void* exception_pointers) noexcept {
    execute_fatal_recipe(FatalFamily::security_failure,
                         windows_fatal_platform_ops(),
                         {exception_pointers});
}

} }
