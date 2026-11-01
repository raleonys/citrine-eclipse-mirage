#pragma once
#include "../anadius-crt-heap-source/heap.hpp"

namespace anadius::crt_realloc_recovered {
using crt_heap_recovered::Heap;
using crt_heap_recovered::State;
struct Effects : crt_heap_recovered::Effects {
    // Required explicit provider: no Win32 heap call or original address dispatch.
    // On failure, the provider retains the original allocation and its contents.
    virtual void* heap_realloc(Heap, std::uint32_t flags, void* old_pointer,
                               std::uint64_t bytes) = 0;
};
void* malloc_base_06a308(std::uint64_t bytes, State&, Effects&);
void* realloc_base_06e3a0(void* old_pointer, std::uint64_t bytes, State&, Effects&);
}
