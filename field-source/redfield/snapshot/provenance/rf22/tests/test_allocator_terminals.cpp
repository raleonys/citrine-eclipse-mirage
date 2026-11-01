// SPDX-License-Identifier: GPL-3.0-only
#include "allocator_ptd_support.hpp"
#include <cassert>
#include <limits>
using namespace redfield::runtime;
struct Escaped{int marker;};
static int throwing_handler(void*,size_t){throw Escaped{77};}
int main(){
 d2test::PtdFake p; d2test::start_ptd(p); d2test::HeapFake over;
 auto ov=allocate_block(std::numeric_limits<size_t>::max()-0x20,AllocationRetryPolicy::outer_retry_only,d2test::ops(over));
 assert(!ov.pointer && ov.terminal==AllocationTerminal::bad_array_new_length && over.requests.empty() && over.handler_sizes.empty());
 d2test::HeapFake e; e.scripted_allocs={nullptr}; AllocatorPlatformOps o{&e,d2test::heap_alloc,d2test::heap_free,throwing_handler}; bool escaped=false;
 try{(void)allocate_block(32,AllocationRetryPolicy::outer_retry_only,o);}catch(const Escaped& x){escaped=x.marker==77;}
 assert(escaped); d2test::stop_ptd(p); return 0;
}
