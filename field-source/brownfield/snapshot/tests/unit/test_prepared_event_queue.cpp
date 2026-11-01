#include "preloader_l/runtime/event_queue.hpp"
#include <cstdlib>
#include <cstring>

static bool g_fail_alloc=false;
static size_t g_allocs=0;
static size_t g_frees=0;
extern "C" void* GetProcessHeap(){ return reinterpret_cast<void*>(1); }
extern "C" void* HeapAlloc(void*,unsigned long,size_t n){
    if(g_fail_alloc) return nullptr;
    void* p=std::malloc(n); if(p) ++g_allocs; return p;
}
extern "C" int HeapFree(void*,unsigned long,void* p){ if(p) ++g_frees; std::free(p); return 1; }

int main(){
    using namespace preloader_l::runtime;
    clear_queued_events_for_test();
    constexpr char xml[]="<LSX><Event/></LSX>";
    PreparedQueuedEvent* p=nullptr;

    g_fail_alloc=true;
    if(prepare_achievement_event(xml,sizeof(xml)-1u,&p)||p) return 1;
    if(queued_event_count_for_test()!=0u) return 2;
    g_fail_alloc=false;

    if(!prepare_achievement_event(xml,sizeof(xml)-1u,&p)||!p) return 3;
    if(queued_event_count_for_test()!=0u) return 4;
    release_prepared_achievement_event(p); p=nullptr;
    if(queued_event_count_for_test()!=0u) return 5;

    if(!prepare_achievement_event(xml,sizeof(xml)-1u,&p)||!p) return 6;
    if(queued_event_count_for_test()!=0u) return 7;
    commit_prepared_achievement_event(p); p=nullptr;
    if(queued_event_count_for_test()!=1u) return 8;
    // D03: a too-small generic copy is non-destructive and preserves FIFO.
    char tiny[4]{}; size_t tiny_n=99u;
    if(drain_one_queued_event(tiny,sizeof(tiny),&tiny_n)) return 18;
    if(tiny_n!=0u||queued_event_count_for_test()!=1u) return 19;
    char out[64]{}; size_t n=0;
    if(!drain_one_queued_event(out,sizeof(out),&n)) return 9;
    if(n!=sizeof(xml)-1u||std::strcmp(out,xml)!=0) return 10;
    if(queued_event_count_for_test()!=0u) return 11;

    // HF17 / F1519-001: production lifecycle clear detaches the full FIFO under
    // lock, then releases every process-heap node. The legacy test alias remains
    // only as a compatibility wrapper around this production API.
    const size_t frees_before=g_frees;
    if(!publish_show_igo_down_event()) return 12;
    if(!publish_achievement_event(xml,sizeof(xml)-1u)) return 13;
    if(queued_event_count_for_test()!=2u) return 14;
    clear_queued_events();
    if(queued_event_count_for_test()!=0u) return 15;
    if(g_frees-frees_before!=2u) return 16;
    if(g_allocs!=g_frees) return 17;
    return 0;
}
