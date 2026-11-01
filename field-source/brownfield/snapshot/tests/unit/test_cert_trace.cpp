#ifndef PRELOADER_L_TRACE_DIAGNOSTICS
#define PRELOADER_L_TRACE_DIAGNOSTICS 1
#endif
#include "preloader_l/runtime/event_trace.hpp"
#include <string.h>
int main() {
    using preloader_l::runtime::TraceEvent;
    using preloader_l::runtime::trace_event_name;
    if (strcmp(trace_event_name(TraceEvent::cert_find_call), "cert_find_call") != 0) return 1;
    if (strcmp(trace_event_name(TraceEvent::cert_find_result), "cert_find_result") != 0) return 2;
    if (strcmp(trace_event_name(TraceEvent::cert_find_error), "cert_find_error") != 0) return 3;
    if (strcmp(trace_event_name(TraceEvent::cert_find_context_nonnull), "cert_find_context_nonnull") != 0) return 4;
    return 0;
}
