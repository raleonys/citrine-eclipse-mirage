// W7 permanent regression derived from EVENT_FIFO_OUTER_DISPATCH Phase02/Phase03.
#include "preloader_l/protocol/socket_exchange.hpp"
#include "preloader_l/runtime/service_state.hpp"
#include "preloader_l/runtime/event_queue.hpp"
#include "preloader_l/net/socket_transport.hpp"
#include "../../src/protocol/socket_exchange_internal.hpp"
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <cstdio>

extern "C" void* GetProcessHeap(){ return reinterpret_cast<void*>(1); }
extern "C" void* HeapAlloc(void*,unsigned long,size_t n){ return std::malloc(n); }
extern "C" int HeapFree(void*,unsigned long,void* p){ std::free(p); return 1; }
namespace preloader_l { namespace runtime {
bool normalized_avatar_path(char*,size_t,size_t* written) noexcept { if(written)*written=0; return false; }
int query_image_size_bucket(int width) noexcept { return width/208; }
} }
namespace {
struct FakeSend{unsigned calls=0;};
preloader_l::net::TransportStatus send_fn(void* c,const uint8_t*,size_t n,size_t* sent) noexcept { ++static_cast<FakeSend*>(c)->calls; if(sent)*sent=n; return preloader_l::net::TransportStatus::ok; }
preloader_l::net::TransportStatus recv_fn(void*,uint8_t*,size_t,size_t* received) noexcept { if(received)*received=0; return preloader_l::net::TransportStatus::closed; }
void close_fn(void*) noexcept {}
preloader_l::protocol::detail::DispatchTransactionResult run_tx(FakeSend& io,const char* xml,size_t n){ preloader_l::protocol::ExchangeState state{}; preloader_l::net::TransportEndpoint ep{&io,recv_fn,send_fn,close_fn}; return preloader_l::protocol::detail::execute_response_transaction(ep,&state,xml,n,nullptr); }
bool reset_state(){ preloader_l::runtime::clear_queued_events_for_test(); preloader_l::runtime::clear_behavioral_service_state_for_test(); constexpr char cfg[]="\"Config2\" { \"Achievements\" { \"AchievementTotals\" { \"A1\" \"1\" } } }"; preloader_l::runtime::BehavioralServiceStateCandidate* c=nullptr; if(!preloader_l::runtime::prepare_behavioral_service_state_cfg(cfg,sizeof(cfg)-1u,&c)||!c)return false; preloader_l::runtime::commit_behavioral_service_state_candidate(c); return true; }
}
int main(){
    constexpr char query[]="<LSX><Request id=\"B\"><QueryAchievements/></Request></LSX>";
    constexpr char stamp[]="2026-10-04T20:00:00";
    constexpr char dummy[]="<LSX><Event sender=\"EbisuSDK\"><Dummy/></Event></LSX>";
    int observed=0;
    for(int iter=0;iter<100;++iter){
        if(!reset_state()) return 1;
        preloader_l::runtime::acquire_grant_achievement_delivery();
        preloader_l::runtime::GrantAchievementTransaction* tx=nullptr;
        preloader_l::runtime::AchievementRecordView projected{}; bool mutated=false,completed=false,inserted=false;
        preloader_l::runtime::acquire_behavioral_service_state();
        const bool prep=preloader_l::runtime::prepare_grant_achievement_locked({"A1",2},1,{stamp,sizeof(stamp)-1u},&tx,&projected,&mutated,&completed,&inserted);
        preloader_l::runtime::release_behavioral_service_state();
        if(!prep||!tx||!mutated||!completed||inserted) return 2;

        preloader_l::runtime::PreparedQueuedEvent* candidate=nullptr;
        if(!preloader_l::runtime::prepare_achievement_event(dummy,sizeof(dummy)-1u,&candidate)||!candidate) return 3;
        char event_out[256]{}; size_t event_n=0; preloader_l::runtime::QueuedEventDelivery delivery{};
        if(!preloader_l::runtime::begin_queued_event_delivery(candidate,event_out,sizeof(event_out),&event_n,&delivery)||!delivery.active) return 4;
        candidate=nullptr; // owned by queue lease

        FakeSend io{}; preloader_l::protocol::detail::DispatchTransactionResult br{}; std::atomic<bool> started{false};
        std::thread tb([&]{ started.store(true,std::memory_order_release); br=run_tx(io,query,sizeof(query)-1u); });
        while(!started.load(std::memory_order_acquire)) std::this_thread::yield();
        // QueryAchievements measure obtains/releases the behavioral-state lock, then
        // blocks in queued_event_preview_size() on the delivery lease above.
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
        preloader_l::runtime::acquire_behavioral_service_state();
        preloader_l::runtime::commit_grant_achievement_locked(tx); tx=nullptr;
        preloader_l::runtime::release_behavioral_service_state();
        // Shipping GrantSocketDelivery::commit uses store-first then queue commit.
        preloader_l::runtime::commit_queued_event_delivery(&delivery);
        preloader_l::runtime::release_grant_achievement_delivery();
        tb.join();
        if(br.status==preloader_l::protocol::ExchangeStatus::ok && br.response_sent) ++observed;
    }
    fprintf(stderr,"W7_GRANT_QUERY_COHERENCE=%d/100\n", observed); return observed==100 ? 0 : 5;
}
