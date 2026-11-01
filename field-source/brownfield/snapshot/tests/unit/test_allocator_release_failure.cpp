#include "preloader_l/runtime/memory_allocator.hpp"
#include <cassert>
#include <cstdlib>
using namespace preloader_l::runtime;
namespace {
struct F { int free_calls=0; bool fail=true; };
void* alloc(void*,size_t n) noexcept { return std::malloc(n); }
int release(void* c,void* p) noexcept { auto*f=static_cast<F*>(c); ++f->free_calls; if(f->fail)return 0; std::free(p); return 1; }
int handler(void*,size_t){return 0;}
}
int main(){
 F f{}; AllocatorPlatformOps ops{&f,alloc,release,handler};
 auto r=allocate_block(64,AllocationRetryPolicy::outer_retry_only,ops); assert(r.pointer);
 void* owner=r.pointer;
 assert(release_block(owner,64,ops)==AllocationReleaseStatus::release_failed);
 assert(owner==r.pointer && f.free_calls==1);
 f.fail=false;
 assert(release_block(owner,64,ops)==AllocationReleaseStatus::ok);
 owner=nullptr;
 assert(f.free_calls==2 && owner==nullptr);
 return 0;
}
