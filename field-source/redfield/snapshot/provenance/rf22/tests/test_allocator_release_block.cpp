// SPDX-License-Identifier: GPL-3.0-only
#include "allocator_ptd_support.hpp"
#include <cassert>
#include <cstdint>
using namespace redfield::runtime;

int main(){
 d2test::PtdFake p; d2test::start_ptd(p);
 alignas(32) unsigned char storage_small[0x1100]{};
 alignas(32) unsigned char storage_large_a[0x1200]{};
 alignas(32) unsigned char storage_large_b[0x1200]{};

 d2test::HeapFake h;
 h.scripted_allocs={storage_small+3,storage_large_a+5,storage_large_b+7};

 auto a=allocate_block(0x0fffu,AllocationRetryPolicy::outer_retry_only,d2test::ops(h));
 assert(a.pointer==storage_small+3 && !a.aligned32);
 release_block(a.pointer,0x0fffu,d2test::ops(h));
 assert(h.frees.size()==1 && h.frees.back()==storage_small+3);

 auto b=allocate_block(0x1000u,AllocationRetryPolicy::outer_retry_only,d2test::ops(h));
 assert(b.pointer && b.aligned32 && reinterpret_cast<void**>(b.pointer)[-1]==storage_large_a+5);
 release_block(b.pointer,0x1000u,d2test::ops(h));
 assert(h.frees.size()==2 && h.frees.back()==storage_large_a+5);

 auto c=allocate_block(0x1001u,AllocationRetryPolicy::outer_retry_only,d2test::ops(h));
 assert(c.pointer && c.aligned32 && reinterpret_cast<void**>(c.pointer)[-1]==storage_large_b+7);
 release_block(c.pointer,0x1001u,d2test::ops(h));
 assert(h.frees.size()==3 && h.frees.back()==storage_large_b+7);

 release_block(nullptr,0x1000u,d2test::ops(h));
 assert(h.frees.size()==3);
 d2test::stop_ptd(p); return 0;
}
