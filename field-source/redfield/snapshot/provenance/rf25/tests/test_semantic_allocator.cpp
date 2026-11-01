// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/xml/value_codec.hpp"
#include "redfield/runtime/transient_allocator.hpp"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <limits>

// Used only by the Linux branch-selection QA driver. Normal Windows builds
// use the real imported process-heap functions from value_codec_semantic.cpp.
#if defined(REDFIELD_HEAP_SHIM)
#if !defined(_WIN32)
#error REDFIELD_HEAP_SHIM requires the Windows SemanticValue layout
#endif
static bool no_heap = false, fail_alloc = false, fail_free = false;
static size_t heap_allocs = 0, heap_frees = 0;
extern "C" void* GetProcessHeap() { return no_heap ? nullptr : &heap_allocs; }
extern "C" void* HeapAlloc(void* heap, unsigned long flags, size_t n) {
    assert(heap == &heap_allocs && flags == 0);
    if (fail_alloc) return nullptr;
    ++heap_allocs;
    return std::malloc(n);
}
extern "C" int HeapFree(void* heap, unsigned long flags, void* p) {
    assert(heap == &heap_allocs && flags == 0);
    if (fail_free) return 0;
    ++heap_frees;
    std::free(p);
    return 1;
}
#endif

namespace {
struct Heap { bool fail_alloc = false, fail_free = false; size_t allocs = 0, frees = 0; };
void* allocate(void* context, size_t n) noexcept {
    auto& h = *static_cast<Heap*>(context);
    if (h.fail_alloc) return nullptr;
    ++h.allocs;
    return std::malloc(n);
}
int release(void* context, void* p) noexcept {
    auto& h = *static_cast<Heap*>(context);
    if (h.fail_free) return 0;
    ++h.frees;
    std::free(p);
    return 1;
}
}

int main() {
    using namespace redfield;
    using namespace runtime;
    assert(!transient_release_pending());
    Heap h{};
    // SemanticValue uses only physical heap allocation/release.
    TransientPlatformOps ops{&h, allocate, release};
    assert(!allocate_transient_bytes(0, ops));
    assert(!allocate_transient_bytes(std::numeric_limits<size_t>::max(), ops));
    assert(h.allocs == 0);
    assert(!allocate_transient_bytes(1, TransientPlatformOps{}));
    h.fail_alloc = true;
    assert(!allocate_transient_bytes(4, ops));
    h.fail_alloc = false;
    void* p = allocate_transient_bytes(4, ops);
    assert(p);
    std::memcpy(p, "abc", 4);
    assert(std::strcmp(static_cast<char*>(p), "abc") == 0);
    h.fail_free = true;
    assert(!release_transient_bytes(p));
    assert(transient_pending_release_count() == 1 && transient_pending_release_bytes() == 4);
    assert(!retry_pending_transient_releases());
    h.fail_free = false;
    assert(retry_pending_transient_releases() && h.allocs == h.frees);
    assert(release_transient_bytes(nullptr));

    xml::SemanticValue value;
    const char plain[] = "plain";
    assert(value.assign_source({plain, 5}) == xml::ValueCodecStatus::ok);
    assert(value.view().data == plain);
    assert(value.assign_source({"a&amp;b", 7}) == xml::ValueCodecStatus::ok);
    auto view = value.view();
    assert(view.size == 3 && std::memcmp(view.data, "a&b", 3) == 0);
    assert(value.assign_source({view.data, view.size}) == xml::ValueCodecStatus::ok);
    assert(value.assign_source({view.data + 1, view.size - 1}) == xml::ValueCodecStatus::invalid_argument);
    value.clear();
    assert(value.view().data == nullptr && !transient_release_pending());
    assert(value.assign_source({"&bogus;", 7}) == xml::ValueCodecStatus::malformed_entity);

#if defined(REDFIELD_HEAP_SHIM)
    no_heap = true;
    assert(value.assign_source({"&amp;", 5}) == xml::ValueCodecStatus::allocation_failed);
    no_heap = false;
    fail_alloc = true;
    assert(value.assign_source({"&amp;", 5}) == xml::ValueCodecStatus::allocation_failed);
    fail_alloc = false;
    assert(value.assign_source({"&amp;", 5}) == xml::ValueCodecStatus::ok);
    fail_free = true;
    value.clear();
    assert(value.view().data == nullptr && transient_pending_release_count() == 1);
    assert(!retry_pending_transient_releases());
    fail_free = false;
    assert(retry_pending_transient_releases() && heap_allocs == heap_frees);
#endif
    assert(!transient_release_pending());
    return 0;
}
