#include "preloader_l/config/store_vectors.hpp"
#include "preloader_l/config/store_runtime.hpp"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

using namespace preloader_l::config;
using namespace preloader_l::runtime;
namespace {
alignas(64) uint8_t arena[0x10000]{};
size_t cursor = 3u;
size_t frees = 0u;
void* heap_alloc(void*, size_t n) noexcept {
    if (cursor + n + 64u > sizeof(arena)) return nullptr;
    void* p = arena + cursor;
    cursor += n + 19u;
    return p;
}
int heap_free(void*, void*) noexcept { ++frees; return 1; }
int new_handler(void*, size_t) { return 0; }
}

int main() {
    clear_runtime_store();
    const char xml[] =
        "<R><Store PreambleB64=\"AQID\" TrailerB64=\"BAUG\">"
        "<Entitlement NumericId=\"7\"/></Store></R>";
    assert(set_runtime_store_xml(xml, sizeof(xml)-1) == StoreRuntimeStatus::ok);

    AllocatorPlatformOps ops{nullptr, &heap_alloc, &heap_free, &new_handler};
    ModelStoreVectorBindings bindings{
        {&ops, AllocationRetryPolicy::outer_retry_only},
        {&ops, AllocationRetryPolicy::outer_retry_only},
    };
    ModelStoreVectorSet vectors{};
    assert(materialize_model_store_vectors(bindings, &vectors) == ModelStoreVectorStatus::ok);
    assert(msvc_byte_vector_size(vectors.preamble) == 3u);
    assert(msvc_byte_vector_size(vectors.trailer) == 3u);
    assert(msvc_byte_vector_size(vectors.payload) == 8u);
    const uint8_t expected[]{1,2,3,0x08,0x07,4,5,6};
    for (size_t i = 0; i < sizeof(expected); ++i) assert(vectors.payload.begin[i] == expected[i]);
    assert(release_model_store_vectors(bindings, &vectors) == ModelStoreVectorStatus::ok);
    assert(model_byte_vector_empty(vectors.preamble));
    assert(model_byte_vector_empty(vectors.trailer));
    assert(model_byte_vector_empty(vectors.payload));
    assert(frees == 3u);
    return 0;
}
