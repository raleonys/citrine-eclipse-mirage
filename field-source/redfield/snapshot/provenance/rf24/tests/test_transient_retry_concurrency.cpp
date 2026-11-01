// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/runtime/transient_allocator.hpp"
#include <cstdlib>
#include <iostream>
#include <latch>
#include <thread>
using namespace redfield::runtime;
#define CHECK(x) do {if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
struct Heap {
    bool fail=true;
    std::latch entered{1},release{1};
    static void* alloc(void*,size_t n) noexcept {return std::malloc(n);}
    static int free(void* p,void* memory) noexcept {
        auto& h=*static_cast<Heap*>(p);
        if(h.fail)return 0;
        h.entered.count_down();h.release.wait();std::free(memory);return 1;
    }
};
int main() {
    Heap heap;void* memory=allocate_transient_bytes(16,{&heap,Heap::alloc,Heap::free});CHECK(memory);
    CHECK(!release_transient_bytes(memory));heap.fail=false;
    bool done=false;std::thread retry([&]{done=retry_pending_transient_releases();});
    heap.entered.wait();
    const bool retained=transient_release_pending();
    const bool competing=drain_transient_releases(0);
    heap.release.count_down();retry.join();
    CHECK(retained && !competing && done && !transient_release_pending());
    CHECK(drain_transient_releases(0));
    std::cout<<"TRANSIENT_RETRY_DETACHED_LIST_STILL_OWNED_CONCURRENT_FENCE_PASS\n";
}
