#include "preloader_l/config/store_vectors.hpp"
#include "preloader_l/config/store_runtime.hpp"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

using namespace preloader_l::config;
using namespace preloader_l::runtime;
namespace {
struct Marker { int value; };
alignas(64) uint8_t arena[512]{};
size_t cursor = 0u;
size_t allocations = 0u;
size_t frees = 0u;
void* alloc_then_fail(void*, size_t n) noexcept {
    ++allocations;
    // Allow the first AE00 preamble allocation, fail subsequent allocation.
    if (allocations > 1u) return nullptr;
    if (cursor + n > sizeof(arena)) return nullptr;
    void* p = arena + cursor;
    cursor += n;
    return p;
}
int free_count(void*, void*) noexcept { ++frees; return 1; }
int throwing_handler(void*, size_t) { throw Marker{77}; }
}

int main() {
    clear_runtime_store();
    const char xml[] =
        "<R><Store PreambleB64=\"AQID\" TrailerB64=\"BAUG\">"
        "<Entitlement NumericId=\"7\"/></Store></R>";
    assert(set_runtime_store_xml(xml, sizeof(xml)-1) == StoreRuntimeStatus::ok);
    AllocatorPlatformOps ops{nullptr, &alloc_then_fail, &free_count, &throwing_handler};
    ModelStoreVectorBindings bindings{
        {&ops, AllocationRetryPolicy::outer_retry_only},
        {&ops, AllocationRetryPolicy::outer_retry_only},
    };
    ModelStoreVectorSet out{};
    bool caught = false;
    try {
        (void)materialize_model_store_vectors(bindings, &out);
    } catch (const Marker& m) {
        caught = (m.value == 77);
    }
    assert(caught);
    assert(frees == 1u); // preamble storage unwound before propagation
    assert(model_byte_vector_empty(out.preamble));
    assert(model_byte_vector_empty(out.trailer));
    assert(model_byte_vector_empty(out.payload));
    return 0;
}
