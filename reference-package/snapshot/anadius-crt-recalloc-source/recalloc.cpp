#include "recalloc.hpp"
#include <cstddef>
#include <limits>
#include <stdexcept>

namespace anadius::crt_recalloc_recovered {
std::uint64_t msize_base_0773bc(void* pointer, State& s, Effects& e) {
    if (!pointer) {
        e.write_error_slot_05d724(22);
        e.invalid_parameter_05c954();
        return UINT64_MAX;
    }
    return e.heap_size(s.heap_0a5a10, 0, pointer);
}
void* recalloc_base_070434(void* pointer, std::uint64_t count, std::uint64_t size,
                         State& s, Effects& e) {
    constexpr std::uint64_t maximum = 0xffffffffffffffe0ULL;
    if (count != 0 && size > maximum / count) {
        e.write_error_slot_05d724(12);
        return nullptr;
    }
    const auto old_capacity = pointer ? msize_base_0773bc(pointer, s, e) : 0;
    const auto bytes = count * size;
    auto* result = crt_realloc_recovered::realloc_base_06e3a0(pointer, bytes, s, e);
    if (result && old_capacity < bytes) {
        if (bytes > static_cast<std::uint64_t>(std::numeric_limits<std::ptrdiff_t>::max()))
            throw std::domain_error("recalloc growth exceeds supported live byte-array extent");
        e.zero_bytes_0793d0(static_cast<std::byte*>(result) +
                              static_cast<std::ptrdiff_t>(old_capacity),
                          bytes - old_capacity);
    }
    return result;
}
}
