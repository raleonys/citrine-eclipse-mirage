#pragma once
#include <stdint.h>
#include "preloader_l/runtime/event_trace.hpp"
namespace preloader_l { namespace runtime {
#if PRELOADER_L_TRACE_DIAGNOSTICS
void windows_debug_trace(TraceEvent event, uint32_t value = 0) noexcept;
void windows_debug_identity() noexcept;
void windows_debug_reconstructed_identity() noexcept;
#else
inline void windows_debug_trace(TraceEvent, uint32_t = 0) noexcept {}
inline void windows_debug_identity() noexcept {}
inline void windows_debug_reconstructed_identity() noexcept {}
#endif
} }
