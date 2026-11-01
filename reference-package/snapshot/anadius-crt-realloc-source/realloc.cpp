#include "realloc.hpp"

namespace anadius::crt_realloc_recovered {
void* malloc_base_06a308(std::uint64_t bytes, State& s, Effects& e) {
    constexpr std::uint64_t maximum = 0xffffffffffffffe0ULL;
    if (bytes <= maximum) {
        if (bytes == 0) bytes = 1;
        do {
            if (auto* result = e.heap_alloc(s.heap_0a5a10, 0, bytes)) return result;
        } while (crt_heap_recovered::query_new_mode_07301c(s) != 0 &&
                 crt_heap_recovered::call_new_handler_0634b0(bytes, s, e));
    }
    e.write_error_slot_05d724(12);
    return nullptr;
}
void* realloc_base_06e3a0(void* pointer, std::uint64_t bytes, State& s, Effects& e) {
    if (!pointer) return malloc_base_06a308(bytes, s, e);
    if (bytes == 0) {
        crt_heap_recovered::free_0668c0(pointer, s, e);
        return nullptr;
    }
    constexpr std::uint64_t maximum = 0xffffffffffffffe0ULL;
    if (bytes <= maximum) {
        do {
            if (auto* result = e.heap_realloc(s.heap_0a5a10, 0, pointer, bytes)) return result;
        } while (crt_heap_recovered::query_new_mode_07301c(s) != 0 &&
                 crt_heap_recovered::call_new_handler_0634b0(bytes, s, e));
    }
    e.write_error_slot_05d724(12);
    return nullptr;
}
}
