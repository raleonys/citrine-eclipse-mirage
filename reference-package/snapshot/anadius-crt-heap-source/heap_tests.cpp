#include "heap.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

using namespace anadius::crt_heap_recovered;
namespace {
int checks = 0;
void check(bool condition) { ++checks; if (!condition) throw std::runtime_error("heap check " + std::to_string(checks)); }
struct Request { Heap heap; std::uint32_t flags; std::uint64_t bytes; };
struct Fake final : Effects {
    State& state;
    explicit Fake(State& s) : state(s) { storage.fill(std::byte{0xa5}); }
    std::vector<std::string> events;
    std::vector<Request> allocations;
    std::vector<std::uint64_t> handler_sizes;
    std::array<std::byte, 1024> storage;
    Heap process_heap = 0x2233;
    unsigned failures = 0;
    bool allocation_always_fails = false, free_result = true, locked = false;
    bool change_heap_on_handler = false, clear_handler_on_unlock = false;
    bool throw_handler = false, throw_alloc = false, throw_free = false;
    std::int32_t handler_result = 1;
    std::uintptr_t expected_handler = 0x9876;
    std::uint32_t error_slot = 101, os_error = 87, mapped = 22;
    void* freed = nullptr;
    Heap freed_heap = 0;
    std::uint32_t freed_flags = 99;
    Heap get_process_heap() override { events.push_back("GetProcessHeap"); return process_heap; }
    void* heap_alloc(Heap heap, std::uint32_t flags, std::uint64_t bytes) override {
        events.push_back("HeapAlloc"); allocations.push_back({heap, flags, bytes});
        if (throw_alloc) throw 5;
        if (allocation_always_fails) return nullptr;
        if (failures > 0) { --failures; return nullptr; }
        // Only fixture storage, no Win32 or CRT heap provider operation.
        if (bytes <= storage.size() && (flags & 8))
            std::fill_n(storage.begin(), static_cast<std::size_t>(bytes), std::byte{});
        return storage.data();
    }
    bool heap_free(Heap heap, std::uint32_t flags, void* p) override {
        events.push_back("HeapFree"); freed_heap = heap; freed_flags = flags; freed = p;
        if (throw_free) throw 6;
        return free_result;
    }
    std::uint32_t last_error() override { events.push_back("GetLastError"); return os_error; }
    std::uint32_t errno_from_os_error_05d650(std::uint32_t value) override {
        events.push_back("MapError"); check(value == os_error); return mapped;
    }
    void write_error_slot_05d724(std::uint32_t value) override { events.push_back("WriteError"); error_slot = value; }
    void lock_0625f8(std::uint32_t index) override {
        events.push_back("Lock"); check(index == 0 && !locked); locked = true;
    }
    void unlock_06264c(std::uint32_t index) override {
        events.push_back("Unlock"); check(index == 0 && locked); locked = false;
        if (clear_handler_on_unlock) { state.cookie_0a1580 = 23; state.encoded_handler_0a5178 = 23; }
    }
    std::int32_t invoke_new_handler(std::uintptr_t handler, std::uint64_t bytes) override {
        events.push_back("Handler"); check(!locked && handler == expected_handler);
        handler_sizes.push_back(bytes);
        if (throw_handler) throw 7;
        if (change_heap_on_handler) state.heap_0a5a10 = 0x4455;
        return handler_result;
    }
};
State state(std::uintptr_t handler = 0x9876, std::uint64_t cookie = 17) {
    return {0x1122, 1, cookie, std::rotl(static_cast<std::uint64_t>(handler), static_cast<int>(cookie & 63)) ^ cookie};
}
void sizes_and_overflow() {
    constexpr auto max = 0xffffffffffffffe0ULL;
    struct Case { std::uint64_t count, size; bool accepted; std::uint64_t bytes; };
    const Case cases[] = {{0, 0, true, 1}, {0, UINT64_MAX, true, 1}, {UINT64_MAX, 0, true, 1},
        {1, max, true, max}, {1, max + 1, false, 0}, {1, UINT64_MAX, false, 0},
        {max, 1, true, max}, {max + 1, 1, false, 0}, {UINT64_MAX, 1, false, 0},
        {2, max / 2, true, max}, {2, max / 2 + 1, false, 0},
        {UINT64_MAX, 2, false, 0}, {1ULL << 32, 1ULL << 32, false, 0},
        {1, 40, true, 40}, {1, 968, true, 968}, {121, 8, true, 968}};
    for (const auto& c : cases) {
        auto s = state(); Fake f(s);
        auto* result = calloc_base_066848(c.count, c.size, s, f);
        check((result != nullptr) == c.accepted);
        if (c.accepted) {
            check(f.allocations.size() == 1 && f.allocations[0].heap == s.heap_0a5a10);
            check(f.allocations[0].flags == 8 && f.allocations[0].bytes == c.bytes);
            check(f.error_slot == 101 && f.events == std::vector<std::string>{"HeapAlloc"});
            if (c.bytes <= f.storage.size()) {
                check(std::all_of(f.storage.begin(), f.storage.begin() + static_cast<std::size_t>(c.bytes),
                                  [](auto b) { return b == std::byte{}; }));
                if (c.bytes < f.storage.size()) check(f.storage[static_cast<std::size_t>(c.bytes)] == std::byte{0xa5});
            }
        } else check(f.allocations.empty() && f.error_slot == 12 && f.events == std::vector<std::string>{"WriteError"});
    }
    // Boundary samples on both sides of the exact division threshold, avoiding
    // reliance on a mirrored multiply-overflow implementation as the oracle.
    for (const auto count : {3ULL, 7ULL, 31ULL, 255ULL, 65537ULL, 0x100000001ULL}) {
        const auto threshold = max / count;
        auto s = state(); Fake f(s);
        check(calloc_base_066848(count, threshold, s, f) != nullptr);
        check(f.allocations[0].bytes <= max);
        f.allocations.clear(); f.events.clear();
        check(calloc_base_066848(count, threshold + 1, s, f) == nullptr);
        check(f.allocations.empty() && f.events == std::vector<std::string>{"WriteError"});
    }
}
void retry_and_handlers() {
    for (const auto mode : {0, 1, -1, std::numeric_limits<std::int32_t>::min()}) {
        auto s = state(); s.new_mode_0a5a20 = mode; Fake f(s); f.failures = 1;
        check(query_new_mode_07301c(s) == mode);
        auto* result = calloc_base_066848(1, 40, s, f);
        check((result != nullptr) == (mode != 0));
        check(f.allocations.size() == (mode == 0 ? 1U : 2U));
        check(f.handler_sizes.size() == (mode == 0 ? 0U : 1U));
        check(f.error_slot == (mode == 0 ? 12U : 101U));
    }
    for (const auto result : {0, -1, 2, std::numeric_limits<std::int32_t>::min()}) {
        auto s = state(); Fake f(s); f.failures = 1; f.handler_result = result;
        check((calloc_base_066848(0, UINT64_MAX, s, f) != nullptr) == (result != 0));
        check(f.handler_sizes == std::vector<std::uint64_t>{1});
        check(f.allocations.size() == (result == 0 ? 1U : 2U));
    }
    {
        auto s = state(0); Fake f(s); f.failures = 1;
        check(calloc_base_066848(1, 968, s, f) == nullptr);
        check(f.events == std::vector<std::string>{"HeapAlloc", "Lock", "Unlock", "WriteError"});
        check(f.handler_sizes.empty() && f.error_slot == 12);
    }
    {
        auto s = state(); Fake f(s); f.failures = 3; f.change_heap_on_handler = true;
        check(calloc_base_066848(2, 20, s, f) == f.storage.data());
        check(f.allocations.size() == 4 && f.handler_sizes == std::vector<std::uint64_t>{40, 40, 40});
        check(f.allocations.front().heap == 0x1122 && f.allocations.back().heap == 0x4455);
        check(f.error_slot == 101);
    }
    {
        auto s = state(); Fake f(s); f.clear_handler_on_unlock = true;
        check(call_new_handler_0634b0(123, s, f));
        check(f.handler_sizes == std::vector<std::uint64_t>{123});
        check(!call_new_handler_0634b0(124, s, f));
        check(f.handler_sizes.size() == 1);
    }
    // Every shift count, including 0, must use the x64 rotate interpretation.
    for (std::uint64_t shift = 0; shift != 64; ++shift) {
        for (const auto pointer : {0ULL, 1ULL, 0x9876ULL, 0x8000000000000000ULL, UINT64_MAX}) {
            const auto cookie = 0x8734a5b600000000ULL | shift;
            auto s = state(pointer, cookie); Fake f(s);
            check(get_new_handler_0634f0(s, f) == pointer);
            check(f.events == std::vector<std::string>{"Lock", "Unlock"});
        }
    }
}
void release_and_initialization() {
    for (const auto heap : {0ULL, 0x6677ULL, UINT64_MAX}) {
        auto s = state(); Fake f(s); f.process_heap = heap;
        check(initialize_heap_0704cc(s, f) == (heap != 0));
        check(s.heap_0a5a10 == heap && f.events == std::vector<std::string>{"GetProcessHeap"});
        check(f.error_slot == 101);
    }
    {
        auto s = state(); Fake f(s); f.free_result = false;
        free_0668c0(nullptr, s, f);
        check(f.events.empty() && f.error_slot == 101);
    }
    for (bool success : {false, true}) {
        auto s = state(); Fake f(s); f.free_result = success;
        auto* pointer = f.storage.data() + 7;
        free_0668c0(pointer, s, f);
        check(f.freed == pointer && f.freed_heap == 0x1122 && f.freed_flags == 0);
        if (success) check(f.events == std::vector<std::string>{"HeapFree"} && f.error_slot == 101);
        else {
            check(f.events == std::vector<std::string>{"HeapFree", "GetLastError", "MapError", "WriteError"});
            check(f.error_slot == 22);
        }
    }
    for (const auto mapped : {0U, 12U, 22U, UINT32_MAX}) {
        auto s = state(); Fake f(s); f.free_result = false; f.mapped = mapped;
        free_0668c0(f.storage.data(), s, f);
        check(f.error_slot == mapped);
    }
}
void exceptions_remain_explicit() {
    for (int kind : {0, 1, 2}) {
        auto s = state(); Fake f(s);
        f.throw_alloc = kind == 0; f.throw_free = kind == 1;
        f.throw_handler = kind == 2; f.failures = kind == 2 ? 1U : 0U;
        int caught = 0;
        try {
            if (kind == 1) free_0668c0(f.storage.data(), s, f);
            else (void)calloc_base_066848(1, 40, s, f);
        } catch (int value) { caught = value; }
        check(caught == kind + 5);
        check(f.error_slot == 101 && !f.locked);
        check(std::find(f.events.begin(), f.events.end(), "WriteError") == f.events.end());
    }
}
}
int main() {
    try {
        sizes_and_overflow(); retry_and_handlers(); release_and_initialization(); exceptions_remain_explicit();
        std::cout << "PASS " << checks << " checks; fake heap and callback providers only\n";
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
