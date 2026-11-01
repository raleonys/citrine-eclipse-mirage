// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/runtime/memory_allocator.hpp"
#include <cassert>
#include <cstdlib>
#include <cstddef>
using namespace redfield::runtime;
struct Harness { int fail_free=0; int frees=0; };
static void* a(void*,size_t n) noexcept { return std::malloc(n); }
static int f(void* c,void* p) noexcept {
    auto* h=static_cast<Harness*>(c); ++h->frees;
    if(h->fail_free) return 0;
    std::free(p); return 1;
}
static int nh(void*,size_t){return 0;}
int main(){
    Harness h{}; AllocatorPlatformOps ops{&h,&a,&f,&nh};
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
    return 0;
}
