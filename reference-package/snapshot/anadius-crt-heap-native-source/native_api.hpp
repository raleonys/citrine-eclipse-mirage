#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include "../anadius-crt-heap-source/heap.hpp"

namespace anadius::crt_heap_native {
using Heap = crt_heap_recovered::Heap;
struct Api {
    decltype(&::GetProcessHeap) get_process_heap;
    decltype(&::HeapAlloc) heap_alloc;
    decltype(&::HeapReAlloc) heap_realloc;
    decltype(&::HeapFree) heap_free;
    decltype(&::HeapSize) heap_size;
    decltype(&::GetLastError) get_last_error;
};
const Api& native_api() noexcept;
// Copies the typed function table. Referenced code must remain loaded for the
// Calls lifetime. Heap and allocation ownership always remains with the caller.
class Calls {
public:
    explicit Calls(const Api& api = native_api());
    Heap get_process_heap();
    void* alloc(Heap, std::uint32_t flags, std::uint64_t bytes);
    void* realloc(Heap, std::uint32_t flags, void*, std::uint64_t bytes);
    bool free(Heap, std::uint32_t flags, void*);
    std::uint64_t size(Heap, std::uint32_t flags, const void*);
    std::uint32_t last_error();
private:
    Api api_;
};
}
