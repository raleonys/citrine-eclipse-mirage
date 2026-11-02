// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <windows.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace fixture {
// Data objects only. The program never calls any pointer into these arrays.
alignas(4096) extern std::array<unsigned char, 65536> targets;
alignas(4096) extern std::array<unsigned char, 65536> buffers;
struct Thread {
    DWORD id;
    DWORD process;
    std::uint64_t ip;
    bool open_ok = true;
    bool suspend_ok = true;
    bool context_ok = true;
    unsigned suspended = 0;
    unsigned resumed = 0;
    unsigned writes = 0;
};
struct State {
    bool heap_create_fail = false;
    bool heap_alloc_fail = false;
    bool virtual_alloc_fail = false;
    bool protect_fail = false;
    bool snapshot_fail = false;
    bool enumeration_fail = false;
    bool module_found = false;
    bool proc_found = false;
    unsigned protects = 0;
    unsigned flushes = 0;
    unsigned snapshots = 0;
    unsigned allocations = 0;
    unsigned frees = 0;
    unsigned closes = 0;
    std::size_t last_patch_size = 0;
    void* last_patch_address = nullptr;
    DWORD last_error = 0;
    std::vector<Thread> threads;
};
extern State state;
void reset();
bool owned(const void*, std::size_t);
std::size_t live_heap_allocations();
std::size_t live_buffer_blocks();
}
