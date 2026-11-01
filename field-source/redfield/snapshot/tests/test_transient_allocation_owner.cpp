// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/runtime/transient_allocator.hpp"
#include <cassert>
#include <cstdlib>
#include <cstddef>
using namespace preloader_l::runtime;
struct Harness { int fail_free=0; int frees=0; };
static void* a(void*,size_t n) noexcept { return std::malloc(n); }
static int f(void* c,void* p) noexcept {
    auto* h=static_cast<Harness*>(c); ++h->frees;
    if(h->fail_free) return 0;
    std::free(p); return 1;
}
struct StorageHeap {
    alignas(std::max_align_t) unsigned char storage[512]{};
    unsigned allocations=0,frees=0;
    bool fail=true;
    static void* allocate(void* p,size_t n) noexcept {
        auto& heap=*static_cast<StorageHeap*>(p);assert(n<=sizeof heap.storage);
        ++heap.allocations;return heap.storage;
    }
    static int release(void* p,void* raw) noexcept {
        auto& heap=*static_cast<StorageHeap*>(p);assert(raw==heap.storage);
        if(heap.fail)return 0;
        ++heap.frees;return 1;
    }
};
int main(){
    Harness h{}; TransientPlatformOps ops{&h,&a,&f};
    void* p1=allocate_transient_bytes(3,ops);
    void* p2=allocate_transient_bytes(8192,ops);
    assert(p1&&p2);
    h.fail_free=1;
    assert(!release_transient_bytes(p1));
    assert(!release_transient_bytes(p2));
    assert(transient_release_pending());
    assert(transient_pending_release_count()==2);
    assert(transient_pending_release_bytes()==8195);
    assert(!retry_pending_transient_releases());
    assert(transient_pending_release_count()==2);
    h.fail_free=0;
    assert(retry_pending_transient_releases());
    assert(!transient_release_pending());
    assert(transient_pending_release_count()==0);
    assert(transient_pending_release_bytes()==0);
    StorageHeap storage;
    TransientPlatformOps captured{&storage,StorageHeap::allocate,StorageHeap::release};
    auto* bytes=static_cast<unsigned char*>(allocate_transient_bytes(16,captured));assert(bytes);
    for(unsigned i=0;i<16;++i)bytes[i]=static_cast<unsigned char>(i);
    // Only the descriptor is borrowed at allocation. Deferred release uses its
    // captured callbacks/context even after the original descriptor changes.
    captured={};
    assert(!release_transient_bytes(bytes) && transient_pending_release_bytes()==16);
    storage.fail=false;assert(drain_transient_releases(0));
    assert(storage.allocations==1 && storage.frees==1 && !transient_release_pending());
    return 0;
}
