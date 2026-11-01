// W7 permanent regression derived from EVENT_FIFO_OUTER_DISPATCH Phase02/Phase03.
#include "preloader_l/protocol/socket_exchange.hpp"
#include "preloader_l/runtime/service_state.hpp"
#include "preloader_l/runtime/event_queue.hpp"
#include "preloader_l/net/socket_transport.hpp"
#include "../../src/protocol/socket_exchange_internal.hpp"
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <thread>
extern "C" void* GetProcessHeap(){ return reinterpret_cast<void*>(1); }
extern "C" void* HeapAlloc(void*,unsigned long,size_t n){ return std::malloc(n); }
extern "C" int HeapFree(void*,unsigned long,void* p){ std::free(p); return 1; }
namespace preloader_l { namespace runtime { bool normalized_avatar_path(char*,size_t,size_t* w) noexcept {if(w)*w=0;return false;} int query_image_size_bucket(int x) noexcept{return x/208;} } }
namespace {
struct FakeSend{unsigned calls=0;};
preloader_l::net::TransportStatus send_fn(void* c,const uint8_t*,size_t n,size_t* sent) noexcept {++static_cast<FakeSend*>(c)->calls;if(sent)*sent=n;return preloader_l::net::TransportStatus::ok;}
preloader_l::net::TransportStatus recv_fn(void*,uint8_t*,size_t,size_t* r) noexcept {if(r)*r=0;return preloader_l::net::TransportStatus::closed;}
void close_fn(void*) noexcept {}
preloader_l::protocol::detail::DispatchTransactionResult run_tx(FakeSend& io,const char* x,size_t n){preloader_l::protocol::ExchangeState s{};preloader_l::net::TransportEndpoint ep{&io,recv_fn,send_fn,close_fn};return preloader_l::protocol::detail::execute_response_transaction(ep,&s,x,n,nullptr);}
bool reset_state(){using namespace preloader_l;runtime::clear_queued_events_for_test();runtime::clear_behavioral_service_state_for_test();constexpr char cfg[]="\"Config2\" { \"Achievements\" { \"AchievementTotals\" { \"A1\" \"10\" } } }";runtime::BehavioralServiceStateCandidate* c=nullptr;if(!runtime::prepare_behavioral_service_state_cfg(cfg,sizeof(cfg)-1,&c)||!c)return false;runtime::commit_behavioral_service_state_candidate(c);constexpr char stamp[]="2026-10-04T19:00:00";runtime::AchievementRecordView rec{};bool m=false,d=false;runtime::acquire_behavioral_service_state();bool ok=runtime::grant_achievement_locked({"A1",2},8,{stamp,sizeof(stamp)-1},&rec,&m,&d);runtime::release_behavioral_service_state();return ok&&m&&!d&&rec.progress==8;}
}
int main(){using namespace preloader_l;constexpr char req[]="<LSX><Request id=\"B\"><GrantAchievement><AchievementId>A1</AchievementId><Progress>1</Progress></GrantAchievement></Request></LSX>";constexpr char stamp[]="2026-10-04T20:00:00";constexpr char ev[]="<LSX><Event sender=\"EbisuSDK\"><Achievement Id=\"A1\"/></Event></LSX>";int observed=0;
for(int i=0;i<100;++i){if(!reset_state())return 1;runtime::acquire_grant_achievement_delivery();runtime::GrantAchievementTransaction* tx=nullptr;runtime::AchievementRecordView rec{};bool m=false,c=false,ins=false;runtime::acquire_behavioral_service_state();bool ok=runtime::prepare_grant_achievement_locked({"A1",2},2,{stamp,sizeof(stamp)-1},&tx,&rec,&m,&c,&ins);runtime::release_behavioral_service_state();if(!ok||!tx||!m||!c||ins)return 2;runtime::PreparedQueuedEvent* cand=nullptr;if(!runtime::prepare_achievement_event(ev,sizeof(ev)-1,&cand)||!cand)return 3;char out[256]{};size_t n=0;runtime::QueuedEventDelivery q{};if(!runtime::begin_queued_event_delivery(cand,out,sizeof(out),&n,&q)||!q.active)return 4;cand=nullptr;FakeSend io{};protocol::detail::DispatchTransactionResult br{};std::atomic<bool> started{false};std::thread tb([&]{started.store(true,std::memory_order_release);br=run_tx(io,req,sizeof(req)-1);});while(!started.load(std::memory_order_acquire))std::this_thread::yield();std::this_thread::sleep_for(std::chrono::milliseconds(3));runtime::acquire_behavioral_service_state();runtime::commit_grant_achievement_locked(tx);tx=nullptr;runtime::release_behavioral_service_state();runtime::commit_queued_event_delivery(&q);runtime::release_grant_achievement_delivery();tb.join();if(br.status==protocol::ExchangeStatus::ok&&br.response_sent)++observed;}
fprintf(stderr,"W7_GRANT_GRANT_COHERENCE=%d/100\n",observed);return observed==100?0:5;}
