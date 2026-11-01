#include "heap.hpp"
#include <bit>

namespace anadius::crt_heap_recovered {
bool initialize_heap_0704cc(State& s, Effects& e) {
    const auto heap = e.get_process_heap();
    s.heap_0a5a10 = heap;
    return heap != 0;
}
std::int32_t query_new_mode_07301c(const State& s) { return s.new_mode_0a5a20; }
std::uintptr_t get_new_handler_0634f0(State& s, Effects& e) {
    e.lock_0625f8(0);
    const auto cookie = s.cookie_0a1580;
    const auto handler = std::rotr(s.encoded_handler_0a5178 ^ cookie,
                                   static_cast<int>(cookie & 63));
    e.unlock_06264c(0);
    return handler;
}
bool call_new_handler_0634b0(std::uint64_t bytes, State& s, Effects& e) {
    const auto handler = get_new_handler_0634f0(s, e);
    return handler != 0 && e.invoke_new_handler(handler, bytes) != 0;
}
void* calloc_base_066848(std::uint64_t count, std::uint64_t size, State& s, Effects& e) {
    constexpr std::uint64_t maximum = 0xffffffffffffffe0ULL;
    if (count == 0 || size <= maximum / count) {
        auto bytes = count * size;
        if (bytes == 0) bytes = 1;
        do {
            if (auto* result = e.heap_alloc(s.heap_0a5a10, 8, bytes)) return result;
        } while (query_new_mode_07301c(s) != 0 && call_new_handler_0634b0(bytes, s, e));
    }
    e.write_error_slot_05d724(12);
    return nullptr;
}
void free_0668c0(void* pointer, State& s, Effects& e) {
    if (pointer && !e.heap_free(s.heap_0a5a10, 0, pointer)) {
        const auto error = e.last_error();
        const auto mapped = e.errno_from_os_error_05d650(error);
        e.write_error_slot_05d724(mapped);
    }
}
}
