#pragma once
#ifndef PRELOADER_L_TRACE_DIAGNOSTICS
#define PRELOADER_L_TRACE_DIAGNOSTICS 0
#endif

#if PRELOADER_L_TRACE_DIAGNOSTICS
#include "preloader_l/runtime/event_trace.hpp"
#include "preloader_l/runtime/debug_trace.hpp"
#define PRELOADER_L_TRACE_NOTE(event,value) \
    ::preloader_l::runtime::note_trace((event),(value))
#define PRELOADER_L_WINDOWS_TRACE(event,value) \
    ::preloader_l::runtime::windows_debug_trace((event),(value))
#define PRELOADER_L_WINDOWS_IDENTITY() \
    ::preloader_l::runtime::windows_debug_identity()
#define PRELOADER_L_WINDOWS_RECONSTRUCTED_IDENTITY() \
    ::preloader_l::runtime::windows_debug_reconstructed_identity()
#else
#define PRELOADER_L_TRACE_NOTE(event,value) ((void)0)
#define PRELOADER_L_WINDOWS_TRACE(event,value) ((void)0)
#define PRELOADER_L_WINDOWS_IDENTITY() ((void)0)
#define PRELOADER_L_WINDOWS_RECONSTRUCTED_IDENTITY() ((void)0)
#endif
