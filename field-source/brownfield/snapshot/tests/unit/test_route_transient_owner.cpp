#include "preloader_l/runtime/memory_allocator.hpp"
#include <cassert>
#include <cstdlib>
static int g_fail_free=0;
extern "C" void* GetProcessHeap(){ return reinterpret_cast<void*>(0x4567); }
extern "C" void* HeapAlloc(void*,unsigned long,size_t n){ return std::malloc(n); }
extern "C" int HeapFree(void*,unsigned long,void* p){ if(g_fail_free)return 0; std::free(p); return 1; }
#include "../../src/handlers/route_response_support.hpp"
int main(){
    using namespace preloader_l;
    {
        handlers::HeapBytes bytes;
        assert(bytes.allocate(64));
        g_fail_free=1;
    }
    assert(runtime::transient_pending_release_count()==1);
    g_fail_free=0;
    assert(runtime::retry_pending_transient_releases());
    assert(runtime::transient_pending_release_count()==0);
    return 0;
}
