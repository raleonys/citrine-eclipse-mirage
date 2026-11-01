#include "preloader_l/xml/value_codec.hpp"
#include "preloader_l/runtime/memory_allocator.hpp"
#include <cassert>
#include <cstdlib>
#include <cstring>
static int g_fail_free=0;
extern "C" void* GetProcessHeap(){ return reinterpret_cast<void*>(0x1234); }
extern "C" void* HeapAlloc(void*,unsigned long,size_t n){ return std::malloc(n); }
extern "C" int HeapFree(void*,unsigned long,void* p){ if(g_fail_free)return 0; std::free(p); return 1; }
int main(){
    using namespace preloader_l;
    {
        xml::SemanticValue v;
        const char encoded[]="A&amp;B";
        assert(v.assign_source({encoded,sizeof(encoded)-1})==xml::ValueCodecStatus::ok);
        auto view=v.view();
        assert(view.size==3 && std::memcmp(view.data,"A&B",3)==0);
        // Owned full-view self-assignment never frees its own source.
        assert(v.assign_source(v.view())==xml::ValueCodecStatus::ok);
        assert(v.view().data==view.data && v.view().size==view.size);
        // Partial internal source aliases are rejected without UAF or state loss.
        assert(v.assign_source({view.data+1,1})==xml::ValueCodecStatus::invalid_argument);
        assert(v.view().data==view.data && v.view().size==view.size);
        // S0291 regression: the owned terminator/end address is an alias too.
        assert(v.assign_source({view.data+view.size,1})==xml::ValueCodecStatus::invalid_argument);
        assert(v.assign_source({view.data+view.size,0})==xml::ValueCodecStatus::invalid_argument);
        assert(v.view().data==view.data && v.view().size==view.size);
        g_fail_free=1;
        v.clear();
        assert(v.view().data==nullptr && v.view().size==0);
        assert(runtime::transient_pending_release_count()==1);
        const char plain[]="plain";
        assert(v.assign_source({plain,sizeof(plain)-1})==xml::ValueCodecStatus::ok);
        assert(v.view().data==plain);
    }
    assert(runtime::transient_pending_release_count()==1);
    g_fail_free=0;
    assert(runtime::retry_pending_transient_releases());
    assert(runtime::transient_pending_release_count()==0);
    {
        xml::SemanticValue v;
        const char encoded[]="C&amp;D";
        assert(v.assign_source({encoded,sizeof(encoded)-1})==xml::ValueCodecStatus::ok);
        g_fail_free=1;
    }
    assert(runtime::transient_pending_release_count()==1);
    g_fail_free=0;
    assert(runtime::retry_pending_transient_releases());
    return 0;
}
