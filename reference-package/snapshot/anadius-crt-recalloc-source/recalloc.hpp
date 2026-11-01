#pragma once
#include "../anadius-crt-realloc-source/realloc.hpp"

namespace anadius::crt_recalloc_recovered {
using State = crt_heap_recovered::State;
using Heap = crt_heap_recovered::Heap;
struct Effects : crt_realloc_recovered::Effects {
    virtual std::uint64_t heap_size(Heap, std::uint32_t flags, void*) = 0;
    virtual void invalid_parameter_05c954() = 0;
    // Provider must zero exactly the indicated live preserved byte storage.
    virtual void zero_bytes_0793d0(void* begin, std::uint64_t bytes) = 0;
};
std::uint64_t msize_base_0773bc(void*, State&, Effects&);
// Returned non-null storage must support requested bytes in one live byte array.
// Growth requiring an extent above PTRDIFF_MAX is an explicit unsupported source
// boundary; that guard is not claimed to be present in the original assembly.
void* recalloc_base_070434(void*, std::uint64_t count, std::uint64_t size,
                         State&, Effects&);
}
