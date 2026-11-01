// SPDX-License-Identifier: GPL-3.0-only
#include "allocator_ptd_support.hpp"
#include <cassert>
using namespace redfield::runtime;
int main(){
 d2test::PtdFake p; d2test::start_ptd(p); int* err=current_errno_slot(); assert(err);
 alignas(32) unsigned char a[64]{}; *err=0; d2test::HeapFake outer; outer.scripted_allocs={nullptr,a}; outer.handler_results={1};
 auto r0=allocate_block(32,AllocationRetryPolicy::outer_retry_only,d2test::ops(outer));
 assert(r0.pointer==a && r0.new_handler_calls==1 && *err==allocation_errno_enomem);
 alignas(32) unsigned char b[64]{}; *err=0; d2test::HeapFake inner; inner.scripted_allocs={nullptr,b}; inner.handler_results={1};
 auto r1=allocate_block(32,AllocationRetryPolicy::inner_then_outer,d2test::ops(inner));
 assert(r1.pointer==b && r1.new_handler_calls==1 && *err==0);
 alignas(32) unsigned char c[64]{}; *err=0; d2test::HeapFake split; split.scripted_allocs={nullptr,c}; split.handler_results={0,1};
 auto r2=allocate_block(32,AllocationRetryPolicy::inner_then_outer,d2test::ops(split));
 assert(r2.pointer==c && r2.new_handler_calls==2 && *err==allocation_errno_enomem);
 *err=0; d2test::HeapFake fail; fail.scripted_allocs={nullptr}; fail.handler_results={0,0};
 auto rf=allocate_block(32,AllocationRetryPolicy::inner_then_outer,d2test::ops(fail));
 assert(!rf.pointer && rf.terminal==AllocationTerminal::bad_alloc && rf.new_handler_calls==2 && *err==allocation_errno_enomem);
 d2test::stop_ptd(p); return 0;
}
