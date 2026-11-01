#include "preloader_l/runtime/exception_policy.hpp"
#include "allocator_ptd_support.hpp"
#include <cassert>
#include <limits>
#include <new>
using namespace preloader_l::runtime;
int main(){
 d2test::PtdFake p; d2test::start_ptd(p);
 alignas(32) unsigned char okmem[64]{};
 d2test::HeapFake ok; ok.scripted_allocs={okmem};
 assert(allocate_block_or_throw(32,AllocationRetryPolicy::outer_retry_only,d2test::ops(ok))==okmem);

 d2test::HeapFake fail; fail.scripted_allocs={nullptr}; fail.handler_results={0};
 bool ba=false; try { (void)allocate_block_or_throw(32,AllocationRetryPolicy::outer_retry_only,d2test::ops(fail)); }
 catch(const std::bad_alloc&){ba=true;} assert(ba);

 d2test::HeapFake over; bool banl=false;
 try { (void)allocate_block_or_throw(std::numeric_limits<size_t>::max()-0x20,AllocationRetryPolicy::outer_retry_only,d2test::ops(over)); }
 catch(const std::bad_array_new_length&){banl=true;} assert(banl);
 d2test::stop_ptd(p); return 0;
}
