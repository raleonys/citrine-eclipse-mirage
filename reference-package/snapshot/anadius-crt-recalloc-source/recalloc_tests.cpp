#include "recalloc.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace r = anadius::crt_recalloc_recovered;
namespace h = anadius::crt_heap_recovered;
namespace {
int checks = 0;
void check(bool condition) {
    ++checks;
    if (!condition) throw std::runtime_error("recalloc check " + std::to_string(checks));
}
struct Fake final : r::Effects {
    h::State& state;
    std::array<std::byte, 256> old_storage{}, new_storage{};
    std::vector<std::string> events;
    std::uint64_t capacity = 16, request = 0, zero_length = 0;
    std::uint32_t error = 99, os_error = 8;
    std::ptrdiff_t zero_offset = -1;
    void* old_pointer = nullptr;
    bool allocation_failure = false, free_success = true, throw_invalid = false;
    bool throw_size = false, throw_zero = false, mutate_heap = false;
    bool locked = false, handler_retry = false;
    unsigned failures = 0;
    h::Heap queried_heap = 0, allocation_heap = 0;
    explicit Fake(h::State& s) : state(s) {
        old_storage.fill(std::byte{0xa5}); new_storage.fill(std::byte{0xcc});
    }
    h::Heap get_process_heap() override { throw std::runtime_error("unexpected process heap"); }
    void* heap_alloc(h::Heap heap, std::uint32_t flags, std::uint64_t bytes) override {
        check(flags == 0); events.push_back("Alloc"); request = bytes; allocation_heap = heap;
        if (allocation_failure) return nullptr;
        if (failures > 0) { --failures; return nullptr; }
        return new_storage.data();
    }
    void* heap_realloc(h::Heap heap, std::uint32_t flags, void* pointer,
                       std::uint64_t bytes) override {
        check(flags == 0); events.push_back("Realloc"); old_pointer = pointer;
        request = bytes; allocation_heap = heap;
        if (allocation_failure) return nullptr;
        if (failures > 0) { --failures; return nullptr; }
        if (capacity <= old_storage.size() && bytes <= new_storage.size())
            std::copy_n(old_storage.begin(), static_cast<std::size_t>(std::min(capacity, bytes)),
                        new_storage.begin());
        return new_storage.data();
    }
    bool heap_free(h::Heap heap, std::uint32_t flags, void* pointer) override {
        check(heap == state.heap_0a5a10 && flags == 0 && pointer == old_storage.data());
        events.push_back("Free"); return free_success;
    }
    std::uint32_t last_error() override { events.push_back("LastError"); return os_error; }
    std::uint32_t errno_from_os_error_05d650(std::uint32_t value) override {
        check(value == os_error); events.push_back("Map"); return 23;
    }
    void write_error_slot_05d724(std::uint32_t value) override {
        events.push_back("Error"); error = value;
    }
    void lock_0625f8(std::uint32_t index) override {
        check(index == 0 && !locked); locked = true; events.push_back("Lock");
    }
    void unlock_06264c(std::uint32_t index) override {
        check(index == 0 && locked); locked = false; events.push_back("Unlock");
    }
    std::int32_t invoke_new_handler(std::uintptr_t callback, std::uint64_t bytes) override {
        check(callback == 0x9876 && !locked && bytes == request);
        events.push_back("Handler"); return handler_retry ? 1 : 0;
    }
    std::uint64_t heap_size(h::Heap heap, std::uint32_t flags, void* pointer) override {
        check(flags == 0 && pointer == old_storage.data()); queried_heap = heap;
        events.push_back("Size");
        if (throw_size) throw 31;
        if (mutate_heap) state.heap_0a5a10 = 0x3344;
        return capacity;
    }
    void invalid_parameter_05c954() override {
        check(error == 22); events.push_back("Invalid"); if (throw_invalid) throw 32;
    }
    void zero_bytes_0793d0(void* pointer, std::uint64_t bytes) override {
        events.push_back("Zero"); if (throw_zero) throw 33;
        zero_offset = static_cast<std::byte*>(pointer) - new_storage.data(); zero_length = bytes;
        check(zero_offset >= 0 && static_cast<std::uint64_t>(zero_offset) + bytes <= new_storage.size());
        std::fill_n(static_cast<std::byte*>(pointer), static_cast<std::size_t>(bytes), std::byte{});
    }
};
h::State state(bool mode = false) {
    constexpr std::uint64_t cookie = 17;
    return {0x1122, mode ? 1 : 0, cookie,
            std::rotl(std::uint64_t{0x9876}, 17) ^ cookie};
}
void msize_cases() {
    auto s = state(); Fake f(s);
    check(r::msize_base_0773bc(nullptr, s, f) == UINT64_MAX);
    check(f.error == 22 && f.events == std::vector<std::string>{"Error", "Invalid"});
    f.events.clear(); f.capacity = UINT64_MAX;
    check(r::msize_base_0773bc(f.old_storage.data(), s, f) == UINT64_MAX);
    check(f.events == std::vector<std::string>{"Size"} && f.error == 22 && f.queried_heap == 0x1122);
    f.events.clear(); f.throw_invalid = true;
    try { r::msize_base_0773bc(nullptr, s, f); check(false); } catch (int value) { check(value == 32); }
    check(f.events == std::vector<std::string>{"Error", "Invalid"});
}
void growth_matrix() {
    for (std::uint64_t capacity = 0; capacity <= 64; capacity += 4) {
        for (std::uint64_t bytes = 1; bytes <= 80; ++bytes) {
            auto s = state(); Fake f(s); f.capacity = capacity;
            auto* result = r::recalloc_base_070434(f.old_storage.data(), bytes, 1, s, f);
            check(result == f.new_storage.data() && f.old_pointer == f.old_storage.data());
            check(f.error == 99 && f.request == bytes);
            check(f.events == (capacity < bytes ? std::vector<std::string>{"Size", "Realloc", "Zero"}
                                              : std::vector<std::string>{"Size", "Realloc"}));
            for (std::uint64_t i = 0; i < bytes; ++i)
                check(f.new_storage[static_cast<std::size_t>(i)] ==
                      (i < capacity ? std::byte{0xa5} : std::byte{}));
            check(f.new_storage[static_cast<std::size_t>(bytes)] == std::byte{0xcc});
            check(f.old_storage[0] == std::byte{0xa5});
        }
    }
    auto s = state(); Fake f(s);
    check(r::recalloc_base_070434(nullptr, 3, 7, s, f) == f.new_storage.data());
    check(f.events == std::vector<std::string>{"Alloc", "Zero"});
    check(f.request == 21 && f.zero_offset == 0 && f.zero_length == 21 && f.error == 99);
}
void boundary_cases() {
    constexpr auto maximum = 0xffffffffffffffe0ULL;
    struct Case { std::uint64_t count, size; bool allowed; };
    const Case cases[] = {{1, maximum, true}, {1, maximum + 1, false},
        {2, maximum / 2, true}, {2, maximum / 2 + 1, false},
        {3, maximum / 3, true}, {3, maximum / 3 + 1, false},
        {maximum, 1, true}, {maximum + 1, 1, false}, {UINT64_MAX, 2, false},
        {UINT64_MAX, 1, false}, {0x100000000ULL, 0x100000000ULL, false}};
    for (const auto& c : cases) {
        auto s = state(); Fake f(s); f.allocation_failure = true;
        check(r::recalloc_base_070434(f.old_storage.data(), c.count, c.size, s, f) == nullptr);
        check(f.events.front() == (c.allowed ? "Size" : "Error"));
        check(std::count(f.events.begin(), f.events.end(), "Realloc") == (c.allowed ? 1 : 0));
        check(f.zero_length == 0 && f.error == 12);
    }
    for (const auto count : {0ULL, 1ULL, UINT64_MAX}) {
        auto s = state(); Fake f(s);
        check(r::recalloc_base_070434(f.old_storage.data(), count, 0, s, f) == nullptr);
        check(f.events == std::vector<std::string>{"Size", "Free"});
        check(f.error == 99);
    }
    auto s = state(); Fake f(s);
    check(r::recalloc_base_070434(nullptr, 0, UINT64_MAX, s, f) == f.new_storage.data());
    check(f.request == 1 && f.events == std::vector<std::string>{"Alloc"});
    check(f.new_storage[0] == std::byte{0xcc}); // malloc normalizes zero; recalloc zeros no bytes.
}
void failures_and_order() {
    for (const bool null_pointer : {false, true}) {
        auto failed_state = state(); Fake failed(failed_state); failed.allocation_failure = true;
        check(r::recalloc_base_070434(null_pointer ? nullptr : failed.old_storage.data(),
                                     32, 1, failed_state, failed) == nullptr);
        check(failed.error == 12 && failed.zero_length == 0);
        check(failed.old_storage[0] == std::byte{0xa5} && failed.old_storage[31] == std::byte{0xa5});
        check(failed.new_storage[0] == std::byte{0xcc});
        check(std::find(failed.events.begin(), failed.events.end(), "Zero") == failed.events.end());
    }
    auto s = state(); Fake f(s); f.capacity = UINT64_MAX;
    check(r::recalloc_base_070434(f.old_storage.data(), 80, 1, s, f) == f.new_storage.data());
    check(f.events == std::vector<std::string>{"Size", "Realloc"} && f.error == 99);
    f.events.clear(); f.free_success = false;
    check(r::recalloc_base_070434(f.old_storage.data(), 0, 9, s, f) == nullptr);
    check(f.events == std::vector<std::string>{"Size", "Free", "LastError", "Map", "Error"});
    check(f.error == 23);
    auto s2 = state(); Fake f2(s2); f2.mutate_heap = true;
    check(r::recalloc_base_070434(f2.old_storage.data(), 32, 1, s2, f2) == f2.new_storage.data());
    check(f2.queried_heap == 0x1122 && f2.allocation_heap == 0x3344);
    auto s3 = state(); Fake f3(s3); f3.throw_size = true;
    try { r::recalloc_base_070434(f3.old_storage.data(), 0, 0, s3, f3); check(false); }
    catch (int value) { check(value == 31); }
    check(f3.events == std::vector<std::string>{"Size"});
    auto s4 = state(); Fake f4(s4); f4.throw_zero = true;
    try { r::recalloc_base_070434(f4.old_storage.data(), 32, 1, s4, f4); check(false); }
    catch (int value) { check(value == 33); }
    check(f4.events == std::vector<std::string>{"Size", "Realloc", "Zero"});
    auto s5 = state(true); Fake f5(s5); f5.failures = 1; f5.handler_retry = true;
    check(r::recalloc_base_070434(f5.old_storage.data(), 32, 1, s5, f5) == f5.new_storage.data());
    check(f5.events == std::vector<std::string>{"Size", "Realloc", "Lock", "Unlock", "Handler", "Realloc", "Zero"});
    check(std::count(f5.events.begin(), f5.events.end(), "Size") == 1);
    auto s6 = state(); Fake f6(s6);
    try { r::recalloc_base_070434(nullptr, 1, 0x8000000000000000ULL, s6, f6); check(false); }
    catch (const std::domain_error&) { check(true); }
    check(f6.events == std::vector<std::string>{"Alloc"});
}
}
int main() {
    try {
        msize_cases(); growth_matrix(); boundary_cases(); failures_and_order();
        std::cout << "recalloc: " << checks << " checks PASS\n";
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
      catch (...) { std::cerr << "unexpected provider exception\n"; return 1; }
}
