#include "allocator_ptd_support.hpp"
#include <cassert>
using namespace preloader_l::runtime;
int main(){
 d2test::PtdFake p; d2test::start_ptd(p);
 d2test::HeapFake z; auto zr=allocate_block(0,AllocationRetryPolicy::outer_retry_only,d2test::ops(z));
 assert(!zr.pointer && zr.terminal==AllocationTerminal::none && z.requests.empty() && z.handler_sizes.empty());
 alignas(32) unsigned char small[64]{}; d2test::HeapFake s; s.scripted_allocs={small};
 auto sr=allocate_block(255,AllocationRetryPolicy::outer_retry_only,d2test::ops(s));
 assert(sr.pointer==small && sr.heap_request_size==255 && !sr.aligned32);
 free_direct(sr.pointer,d2test::ops(s)); assert(s.frees.size()==1 && s.frees[0]==small);
 free_direct(nullptr,d2test::ops(s)); assert(s.frees.size()==1);
 d2test::stop_ptd(p); return 0;
}
