#include "preloader_l/runtime/loader_signals.hpp"

namespace preloader_l { namespace runtime {
namespace {
volatile uint32_t g_attach_count = 0;
volatile uint32_t g_detach_count = 0;
volatile uint32_t g_surge_count = 0;
volatile uint32_t g_last_event = static_cast<uint32_t>(LoaderEvent::none);
}

void reset_signals_for_test() noexcept {
    g_attach_count = 0;
    g_detach_count = 0;
    g_surge_count = 0;
    g_last_event = static_cast<uint32_t>(LoaderEvent::none);
}

void note_process_attach() noexcept {
    ++g_attach_count;
    g_last_event = static_cast<uint32_t>(LoaderEvent::process_attach);
}

void note_process_detach() noexcept {
    ++g_detach_count;
    g_last_event = static_cast<uint32_t>(LoaderEvent::process_detach);
}

void note_surge_call() noexcept { ++g_surge_count; }

SignalSnapshot snapshot_signals() noexcept {
    SignalSnapshot s{};
    s.process_attach_count = g_attach_count;
    s.process_detach_count = g_detach_count;
    s.surge_call_count = g_surge_count;
    s.last_loader_event = static_cast<LoaderEvent>(g_last_event);
    return s;
}

} }
