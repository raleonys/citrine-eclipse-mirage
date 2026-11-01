#pragma once
#include "preloader_l/runtime/fatal_policy.hpp"
namespace preloader_l { namespace runtime {
const FatalPlatformOps& windows_fatal_platform_ops() noexcept;

// Windows recovery-model bindings for D5 fatal-policy evidence. 2.9.0 links
// this policy into the normalized DLL because the curio jthread teardown
// now has live terminal consumers. Normalized 3.1.8 shipping is /MD + /GS; this
// policy itself still binds only the curio Win32 terminal surface.
void dispatch_terminate_then_abort_windows() noexcept;
void dispatch_invalid_parameter_windows(const wchar_t* expression,
                                        const wchar_t* function_name,
                                        const wchar_t* file_name,
                                        unsigned int line_number,
                                        uintptr_t reserved) noexcept;

// Security-failure model binding. exception_pointers must point to the
// caller-supplied Windows EXCEPTION_POINTERS object recovered upstream.
void dispatch_security_failure_windows(const void* exception_pointers) noexcept;
} }
