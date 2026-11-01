// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/runtime/transient_allocator.hpp"
#include <cstdlib>
#include <iostream>
#include <latch>
#include <thread>
#include <atomic>
using namespace preloader_l::runtime;
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
struct QueueHeap {
    std::atomic<bool> fail{true},block{false};
    std::atomic<unsigned> frees{0};
    std::latch entered{1},release{1};
    bool nested_rejected=false,pending_seen=false,fail_after_block=false;
    static void* alloc(void*,size_t n) noexcept {return std::malloc(n);}
    static int free(void* p,void* memory) noexcept {
        auto& heap=*static_cast<QueueHeap*>(p);
        if(heap.fail.load())return 0;
        if(heap.block.exchange(false)) {
            heap.entered.count_down();heap.release.wait();
            if(heap.fail_after_block)return 0;
        }
        heap.nested_rejected=!retry_pending_transient_releases();
        heap.pending_seen=transient_release_pending();
        std::free(memory);++heap.frees;return 1;
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
    for(unsigned round=0;round<16;++round) {
        QueueHeap first,second;
        void* a=allocate_transient_bytes(3,{&first,QueueHeap::alloc,QueueHeap::free});
        void* b=allocate_transient_bytes(5,{&second,QueueHeap::alloc,QueueHeap::free});
        CHECK(a && b && !release_transient_bytes(a));
        first.fail=false;first.block=true;first.fail_after_block=(round%2)!=0;
        bool result=true;std::thread batch([&]{result=retry_pending_transient_releases();});
        first.entered.wait();
        const bool detached_owned=transient_release_pending() && transient_pending_release_count()==0 && transient_pending_release_bytes()==0;
        const bool competing=retry_pending_transient_releases();
        const bool retained_new=!release_transient_bytes(b) && transient_pending_release_count()==1 && transient_pending_release_bytes()==5;
        first.release.count_down();batch.join();
        CHECK(detached_owned && !competing && retained_new && !result);
        if(first.fail_after_block) {
            CHECK(first.frees==0 && second.frees==0);
            CHECK(transient_pending_release_count()==2 && transient_pending_release_bytes()==8);
        } else {
            CHECK(first.frees==1 && second.frees==0 && first.nested_rejected && first.pending_seen);
            CHECK(transient_pending_release_count()==1 && transient_pending_release_bytes()==5);
        }
        CHECK(transient_release_pending());
        second.fail=false;
        CHECK(drain_transient_releases(0));
        CHECK(first.frees==1 && first.nested_rejected && first.pending_seen);
        CHECK(second.frees==1 && second.nested_rejected && second.pending_seen && !transient_release_pending());
    }
    std::cout<<"TRANSIENT_RETRY_DETACHED_LIST_STILL_OWNED_CONCURRENT_FENCE_PASS\n";
}
