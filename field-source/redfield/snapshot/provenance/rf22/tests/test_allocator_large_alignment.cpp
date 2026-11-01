// SPDX-License-Identifier: GPL-3.0-only
#include "allocator_ptd_support.hpp"
#include <cassert>
#include <cstdint>
using namespace redfield::runtime;
int main(){
 d2test::PtdFake p; d2test::start_ptd(p); alignas(32) unsigned char storage[0x1200]{}; void* raw=storage+3;
 d2test::HeapFake h; h.scripted_allocs={raw}; auto r=allocate_block(0x1000,AllocationRetryPolicy::outer_retry_only,d2test::ops(h));
 assert(r.pointer && r.aligned32 && (reinterpret_cast<uintptr_t>(r.pointer)&31u)==0 && r.heap_request_size==0x1027);
 assert(reinterpret_cast<void**>(r.pointer)[-1]==raw); assert(free_aligned32(r.pointer,0x1000u,d2test::ops(h))==AllocationReleaseStatus::ok);
 assert(h.frees.size()==1 && h.frees[0]==raw); d2test::stop_ptd(p); return 0;
}
