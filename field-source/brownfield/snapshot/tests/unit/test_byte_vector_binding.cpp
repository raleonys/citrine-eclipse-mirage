#include "preloader_l/config/byte_vector_binding.hpp"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

using namespace preloader_l::config;
using namespace preloader_l::runtime;

namespace {
alignas(64) uint8_t arena[0x7000]{};
size_t cursor = 1u;
void* freed[8]{};
size_t free_count = 0u;
int handler_calls = 0;
bool fail_free = false;

void* heap_alloc(void*, size_t size) noexcept {
    if (cursor + size + 64u > sizeof(arena)) return nullptr;
    void* p = arena + cursor;
    cursor += size + 17u;
    return p;
}
int heap_free(void*, void* p) noexcept {
    if (fail_free) return 0;
    if (free_count < 8u) freed[free_count++] = p;
    return 1;
}
int new_handler(void*, size_t) {
    ++handler_calls;
    return 0;
}
}

int main() {
    AllocatorPlatformOps ops{nullptr, &heap_alloc, &heap_free, &new_handler};
    ModelByteVectorAllocatorBinding binding{&ops, AllocationRetryPolicy::outer_retry_only};

    MsvcByteVectorLayout small{};
    assert(model_byte_vector_allocate_exact(31u, binding, &small) == ModelByteVectorStatus::ok);
    assert(small.end - small.begin == 31);
    assert(small.capacity == small.end);
    void* small_raw = small.begin;
    assert(model_byte_vector_release(binding, &small) == ModelByteVectorStatus::ok);
    assert(free_count == 1u && freed[0] == small_raw);
    assert(model_byte_vector_empty(small));

    MsvcByteVectorLayout large{};
    assert(model_byte_vector_allocate_exact(0x1000u, binding, &large) == ModelByteVectorStatus::ok);
    assert((reinterpret_cast<uintptr_t>(large.begin) & 31u) == 0u);
    assert(large.end - large.begin == 0x1000);
    void* large_raw = *(reinterpret_cast<void**>(large.begin) - 1);
    assert(large_raw != nullptr && large_raw != large.begin);
    assert(model_byte_vector_release(binding, &large) == ModelByteVectorStatus::ok);
    assert(free_count == 2u && freed[1] == large_raw);

    const uint8_t bytes[]{1,2,3,4,5};
    MsvcByteVectorLayout copy{};
    assert(model_byte_vector_copy_exact(bytes, sizeof(bytes), binding, &copy) == ModelByteVectorStatus::ok);
    for (size_t i = 0; i < sizeof(bytes); ++i) assert(copy.begin[i] == bytes[i]);
    fail_free = true;
    MsvcByteVectorLayout retained{};
    assert(model_byte_vector_copy_exact(bytes, sizeof(bytes), binding, &retained) == ModelByteVectorStatus::ok);
    auto* retained_begin = retained.begin;
    assert(model_byte_vector_release(binding, &retained) == ModelByteVectorStatus::release_failed);
    assert(retained.begin == retained_begin);
    fail_free = false;
    assert(model_byte_vector_release(binding, &retained) == ModelByteVectorStatus::ok);
    assert(model_byte_vector_empty(retained));

    assert(model_byte_vector_release(binding, &copy) == ModelByteVectorStatus::ok);

    MsvcByteVectorLayout occupied{reinterpret_cast<uint8_t*>(1), reinterpret_cast<uint8_t*>(1), reinterpret_cast<uint8_t*>(1)};
    assert(model_byte_vector_allocate_exact(1u, binding, &occupied) == ModelByteVectorStatus::destination_not_empty);
    assert(handler_calls == 0);
    return 0;
}
