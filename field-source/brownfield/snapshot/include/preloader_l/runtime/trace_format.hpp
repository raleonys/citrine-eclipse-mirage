#pragma once
#include <stddef.h>
#include <stdint.h>
#include "preloader_l/runtime/event_trace.hpp"

namespace preloader_l { namespace runtime {

// Writes a terminal-NUL string when capacity is nonzero. Returns bytes written
// excluding the NUL; returns 0 on invalid/capacity failure.
#if PRELOADER_L_TRACE_DIAGNOSTICS
size_t format_trace_line(TraceEvent event, uint32_t value,
                         char* output, size_t capacity) noexcept;
#else
inline size_t format_trace_line(TraceEvent, uint32_t, char* output, size_t capacity) noexcept {
    if (output && capacity) output[0] = 0;
    return 0u;
}
#endif

} }
