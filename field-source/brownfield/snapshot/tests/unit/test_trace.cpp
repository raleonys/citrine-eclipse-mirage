#ifndef PRELOADER_L_TRACE_DIAGNOSTICS
#define PRELOADER_L_TRACE_DIAGNOSTICS 1
#endif
#include "preloader_l/runtime/event_trace.hpp"
#include <atomic>
#include <cstring>
#include <thread>

namespace {
bool valid_pair(const preloader_l::runtime::TraceRecord& record) {
    const bool high = (record.value & 0x80000000u) != 0;
    return high
        ? record.event == preloader_l::runtime::TraceEvent::response_sent
        : record.event == preloader_l::runtime::TraceEvent::bytes_received;
}
}

int main() {
    using namespace preloader_l::runtime;
    reset_trace_for_test();
    note_trace(TraceEvent::process_attach, 1);
    note_trace(TraceEvent::listener_active, 3216);
    TraceRecord records[4]{};
    const size_t count = snapshot_trace(records, 4);
    if (count != 2) return 1;
    if (records[0].sequence != 1 || records[0].event != TraceEvent::process_attach || records[0].value != 1) return 2;
    if (records[1].sequence != 2 || records[1].event != TraceEvent::listener_active || records[1].value != 3216) return 3;
    if (std::strcmp(trace_event_name(records[1].event), "listener_active") != 0) return 4;
    for (unsigned i=0;i<80;++i) note_trace(TraceEvent::bytes_received, i);
    TraceRecord wrap[64]{};
    if (snapshot_trace(wrap, 64) != 64) return 5;
    if (wrap[63].value != 79) return 6;

    // F323-S0718-001: writers and snapshots may touch the same reused ring
    // slot concurrently, but every returned payload must be one coherent
    // publication. The event/value relation makes torn payloads observable.
    reset_trace_for_test();
    std::atomic<unsigned> active{4};
    std::atomic<bool> invalid{false};
    auto writer = [&](unsigned id) {
        const bool high = (id & 1u) != 0;
        const TraceEvent event = high ? TraceEvent::response_sent : TraceEvent::bytes_received;
        const uint32_t prefix = high ? 0x80000000u : 0u;
        for (uint32_t i=0; i<6000u; ++i)
            note_trace(event, prefix | ((id & 0x7fu) << 20) | (i & 0xfffffu));
        --active;
    };
    std::thread a(writer,0u), b(writer,1u), c(writer,2u), d(writer,3u);
    while (active.load(std::memory_order_acquire) != 0u) {
        TraceRecord concurrent[64]{};
        const size_t n = snapshot_trace(concurrent, 64);
        for (size_t i=0; i<n; ++i) {
            if (!valid_pair(concurrent[i])) invalid.store(true, std::memory_order_release);
        }
    }
    a.join(); b.join(); c.join(); d.join();
    if (invalid.load(std::memory_order_acquire)) return 7;
    TraceRecord final_records[64]{};
    const size_t final_count = snapshot_trace(final_records, 64);
    if (final_count == 0 || final_count > 64) return 8;
    for (size_t i=0; i<final_count; ++i) if (!valid_pair(final_records[i])) return 9;
    return 0;
}
