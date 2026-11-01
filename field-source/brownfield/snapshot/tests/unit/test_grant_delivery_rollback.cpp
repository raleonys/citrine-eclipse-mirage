#include "preloader_l/runtime/service_state.hpp"
#include "preloader_l/runtime/event_queue.hpp"
#include <cstdlib>
#include <cstring>

extern "C" void* GetProcessHeap(){ return reinterpret_cast<void*>(1); }
extern "C" void* HeapAlloc(void*,unsigned long,size_t n){ return std::malloc(n); }
extern "C" int HeapFree(void*,unsigned long,void* p){ std::free(p); return 1; }

namespace {
using namespace preloader_l;
bool same(runtime::ServiceTextView v,const char* s){const size_t n=std::strlen(s);if(v.size!=n)return false;for(size_t i=0;i<n;++i)if(v.data[i]!=s[i])return false;return true;}
bool reset_state(){
    runtime::clear_queued_events_for_test();runtime::clear_behavioral_service_state_for_test();
    constexpr char cfg[]="\"Config2\" { \"Achievements\" { \"AchievementTotals\" { \"A1\" \"1\" } } }";
    runtime::BehavioralServiceStateCandidate* c=nullptr;
    if(!runtime::prepare_behavioral_service_state_cfg(cfg,sizeof(cfg)-1u,&c)||!c)return false;
    runtime::commit_behavioral_service_state_candidate(c);return true;
}
bool stored_is(int count,int progress){
    runtime::acquire_behavioral_service_state();runtime::AchievementRecordView r{};
    const bool ok=runtime::achievement_record_count_locked()==1u&&runtime::achievement_record_at_locked(0,&r)&&same(r.id,"A1")&&r.count==count&&r.progress==progress;
    runtime::release_behavioral_service_state();return ok;
}
bool prepare_tx(runtime::GrantAchievementTransaction** tx,runtime::PreparedQueuedEvent** ev){
    *tx=nullptr;*ev=nullptr;
    constexpr char stamp[]="2026-09-28T00:00:00Z";
    runtime::AchievementRecordView rec{};bool mutated=false,completed=false,inserted=false;
    runtime::acquire_behavioral_service_state();
    const bool ok=runtime::prepare_grant_achievement_locked({"A1",2},{1},{stamp,sizeof(stamp)-1u},tx,&rec,&mutated,&completed,&inserted);
    runtime::release_behavioral_service_state();
    if(!ok||!*tx||!mutated||!completed||inserted)return false;
    constexpr char event[]="<LSX><Event sender=\"EbisuSDK\"><Achievement Id=\"A1\"/></Event></LSX>";
    return runtime::prepare_achievement_event(event,sizeof(event)-1u,ev)&&*ev;
}
}

int main(){
    using namespace preloader_l;
    if(!reset_state())return 1;

    // Failure path with a pre-existing FIFO head: candidate is provisional,
    // selected head is copied but neither head nor candidate is consumed on rollback.
    if(!runtime::publish_show_igo_down_event())return 2;
    if(runtime::queued_event_count_for_test()!=1u)return 3;
    runtime::acquire_grant_achievement_delivery();
    runtime::GrantAchievementTransaction* tx=nullptr;runtime::PreparedQueuedEvent* ev=nullptr;
    if(!prepare_tx(&tx,&ev))return 4;
    char out[1024]{};size_t n=0;runtime::QueuedEventDelivery q{};
    if(!runtime::begin_queued_event_delivery(ev,out,sizeof(out),&n,&q)||n==0u||!q.active)return 5;
    ev=nullptr;
    runtime::rollback_queued_event_delivery(&q);
    runtime::release_grant_achievement_transaction(tx);tx=nullptr;
    runtime::release_grant_achievement_delivery();
    if(!stored_is(0,0))return 6;
    if(runtime::queued_event_count_for_test()!=1u)return 7;
    size_t drained=0;if(!runtime::drain_one_queued_event(out,sizeof(out),&drained)||drained==0u)return 8;
    if(runtime::queued_event_count_for_test()!=0u)return 9;

    // Retry/success path: prepare the same logical mutation again, reserve its
    // completion event, then commit only after the modeled send succeeds.
    runtime::acquire_grant_achievement_delivery();
    if(!prepare_tx(&tx,&ev))return 10;
    n=0;q=runtime::QueuedEventDelivery{};
    if(!runtime::begin_queued_event_delivery(ev,out,sizeof(out),&n,&q)||n==0u||!q.active)return 11;
    ev=nullptr;
    runtime::acquire_behavioral_service_state();runtime::commit_grant_achievement_locked(tx);tx=nullptr;runtime::release_behavioral_service_state();
    runtime::commit_queued_event_delivery(&q);
    runtime::release_grant_achievement_delivery();
    if(!stored_is(1,1))return 12;
    if(runtime::queued_event_count_for_test()!=0u)return 13;
    return 0;
}
