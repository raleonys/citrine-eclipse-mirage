// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#ifndef REDFIELD_TRACE_DIAGNOSTICS
#define REDFIELD_TRACE_DIAGNOSTICS 0
#endif

#if REDFIELD_TRACE_DIAGNOSTICS
#include "redfield/runtime/event_trace.hpp"
#include "redfield/runtime/debug_trace.hpp"
#define REDFIELD_TRACE_NOTE(event,value) \
    ::redfield::runtime::note_trace((event),(value))
#define REDFIELD_WINDOWS_TRACE(event,value) \
    ::redfield::runtime::windows_debug_trace((event),(value))
#define REDFIELD_WINDOWS_IDENTITY() \
    ::redfield::runtime::windows_debug_identity()
#define REDFIELD_WINDOWS_RECONSTRUCTED_IDENTITY() \
    ::redfield::runtime::windows_debug_reconstructed_identity()
#else
#define REDFIELD_TRACE_NOTE(event,value) ((void)0)
#define REDFIELD_WINDOWS_TRACE(event,value) ((void)0)
#define REDFIELD_WINDOWS_IDENTITY() ((void)0)
#define REDFIELD_WINDOWS_RECONSTRUCTED_IDENTITY() ((void)0)
#endif
