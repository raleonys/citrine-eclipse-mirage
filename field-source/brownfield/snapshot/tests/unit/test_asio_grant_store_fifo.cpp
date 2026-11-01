// Stage04D portable: actual persistent Grant state and Event FIFO through real
// transaction owner; callback test scheduler is NOT Windows Asio IOCP.
#include "socket_exchange_transaction_owner_private.hpp"
#include "socket_exchange_async_transaction.hpp"
#include "preloader_l/runtime/event_queue.hpp"
#include "preloader_l/runtime/service_state.hpp"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <new>
#include <stdexcept>

// GCC/Clang fixture for Windows-only heap entrypoints. Event FIFO and
// transaction completion implementations below are real production sources.
extern "C" void* GetProcessHeap(){return reinterpret_cast<void*>(1);}
extern "C" void* HeapAlloc(void*,unsigned long,size_t n){return std::malloc(n);}
extern "C" int HeapFree(void*,unsigned long,void* p){std::free(p);return 1;}

namespace p=preloader_l;
namespace d=preloader_l::protocol::detail;
constexpr char kResponse[]="RESPONSE";
constexpr size_t kResponseN=sizeof(kResponse)-1;
static_assert(kResponseN==8,"Full fixture Response must be eight bytes");
static int terminal=0,launches=0,checks=0;
constexpr char kCfg[]="\"Config2\" { \"Achievements\" { \"AchievementTotals\" { \"A1\" \"2\" } } }";
constexpr char kStamp[]="2026-10-14T00:00:00";
// A narrow adapter invokes the genuine service-state implementation, not
// a recorder. The full public Grant handler and dispatcher remain unlinked.
namespace preloader_l { namespace handlers {
void commit_grant_achievement_delivery_store(GrantAchievementDelivery* g) noexcept {
 if(!g||!g->active||!g->transaction)return;
 p::runtime::acquire_behavioral_service_state();
 p::runtime::commit_grant_achievement_locked(g->transaction);
 p::runtime::release_behavioral_service_state();g->transaction=nullptr;
}
void finish_grant_achievement_delivery(GrantAchievementDelivery* g) noexcept {
 if(!g||!g->active)return;
 if(g->event){p::runtime::commit_prepared_achievement_event(g->event);g->event=nullptr;}
 g->active=false;p::runtime::release_grant_achievement_delivery();
}
void rollback_grant_achievement_delivery(GrantAchievementDelivery* g) noexcept {
 if(!g||!g->active)return;
 if(g->transaction){p::runtime::release_grant_achievement_transaction(g->transaction);g->transaction=nullptr;}
 if(g->event){p::runtime::release_prepared_achievement_event(g->event);g->event=nullptr;}
 g->active=false;p::runtime::release_grant_achievement_delivery();
}
}}
namespace preloader_l { namespace runtime {
LocalSocialResult commit_local_social_mutation(const LocalSocialSession&,LocalSocialPending* p) noexcept {p->active=false;return LocalSocialResult::ok;}
void rollback_local_social_mutation(LocalSocialPending* p) noexcept {p->active=false;}
}}
static void check(bool x,const char* what){++checks;if(!x)throw std::runtime_error(what);}
static void* a(void*,size_t n) noexcept {return std::malloc(n);}
static int f(void*,void* x) noexcept {std::free(x);return 1;}
struct Record {
 d::PreparedAsyncTransaction* driver=nullptr;
 bool reject_event=false;
 const uint8_t* bytes=nullptr;
 size_t len=0;
 d::DispatchTransactionResult result{};
};
static bool schedule(void* v,const uint8_t* bytes,size_t length) noexcept {
 auto* r=static_cast<Record*>(v);++launches;
 if(r->reject_event && launches==2)return false;
 r->bytes=bytes;r->len=length;
 return true;
}
static void finished(void* v,const d::DispatchTransactionResult& t) noexcept {
 auto* r=static_cast<Record*>(v);++terminal;r->result=t;
}
static void init_store(){
 p::runtime::clear_behavioral_service_state_for_test();
 p::runtime::BehavioralServiceStateCandidate* candidate=nullptr;
 check(p::runtime::prepare_behavioral_service_state_cfg(kCfg,sizeof(kCfg)-1u,&candidate)&&candidate,"real cfg");
 p::runtime::commit_behavioral_service_state_candidate(candidate);
}
static int progress(){
 p::runtime::acquire_behavioral_service_state();
 p::runtime::AchievementRecordView rec{};
 bool good=p::runtime::achievement_record_count_locked()==1u
       &&p::runtime::achievement_record_at_locked(0,&rec)
       &&rec.total==2&&rec.count==0;
 int result=rec.progress;
 p::runtime::release_behavioral_service_state();
 check(good,"snapshot persistent achievement");
 return result;
}
static d::PreparedResponseTransaction* prepare(const char* event,size_t event_length){
 auto ops=p::runtime::AllocatorPlatformOps{GetProcessHeap(),a,f,nullptr};
 void* raw=p::runtime::allocate_transient_bytes(sizeof(d::PreparedResponseTransaction),ops);
 check(raw!=nullptr,"allocate transaction");
 auto* tx=new(raw)d::PreparedResponseTransaction{};
 tx->wire.data=static_cast<char*>(p::runtime::allocate_transient_bytes(kResponseN+event_length+1u,ops));
 check(tx->wire.data!=nullptr,"allocate wire");
 tx->wire.capacity=kResponseN+event_length+1u;
 std::memcpy(tx->wire.data,kResponse,kResponseN);
 tx->response_required=kResponseN;
 tx->event_required=event_length;
 p::runtime::acquire_grant_achievement_delivery();
 p::runtime::GrantAchievementTransaction* mutation=nullptr;
 p::runtime::AchievementRecordView projected{};
 bool changed=false,completed=false,inserted=false;
 p::runtime::acquire_behavioral_service_state();
 bool ok=p::runtime::prepare_grant_achievement_locked({"A1",2},1,{kStamp,sizeof(kStamp)-1u},
       &mutation,&projected,&changed,&completed,&inserted);
 p::runtime::release_behavioral_service_state();
 check(ok&&mutation&&changed&&!completed&&!inserted&&projected.progress==1,"real mutation reservation");
 tx->grant_delivery.active=true;tx->grant_delivery.grant={mutation,nullptr,true};
 if(event_length){
   size_t written=0;
   check(p::runtime::begin_queued_event_delivery(nullptr,tx->wire.data+kResponseN,event_length+1,&written,
         &tx->generic_event_delivery.queue),"genuine FIFO reserve");
   check(written==event_length,"FIFO length");
   check(std::memcmp(tx->wire.data+kResponseN,event,event_length)==0,"genuine FIFO payload");
 }
 check(d::begin_delivery_writes(&tx->cursor,kResponseN,event_length),"cursor init");
 return tx;
}
static void one(bool event,bool stop_after_response,bool reject_event){
 check(p::runtime::clear_queued_events(),"start clean");
 init_store();check(progress()==0,"fresh state");
 constexpr char payload[]="<LSX><Event sender=\"EbisuSDK\"><IGOEvent State=\"DOWN\"/></Event></LSX>";
 constexpr size_t n=sizeof(payload)-1;
 if(event)check(p::runtime::publish_achievement_event(payload,n),"publish FIFO");
 check(p::runtime::queued_event_count_for_test()==(event?1u:0u),"pre-reserve FIFO count");
 Record rec{};rec.reject_event=reject_event;
 terminal=launches=0;
 auto* tx=prepare(payload,event?n:0u);
 check(p::runtime::queued_event_count_for_test()==(event?1u:0u),"FIFO retained on reserve");
 if(event)check(!p::runtime::clear_queued_events(),"must refuse clear while IO pending");
 d::PreparedAsyncTransaction driver(tx,&rec,&schedule,&finished);rec.driver=&driver;
 check(driver.begin(),"async driver begin");
 check(launches==1 && rec.len==kResponseN,"response first");
 if(stop_after_response)driver.request_stop();
 driver.completed(true,kResponseN);
 const bool event_success=event&&!stop_after_response&&!reject_event;
 if(event_success){
   check(launches==2 && rec.len==n,"event after response");
   driver.completed(true,n);
 }
 check(driver.is_finished()&&terminal==1,"terminal once");
 check(rec.result.response_sent && progress()==1,"real Grant commit after complete Response");
 check(p::runtime::queued_event_count_for_test()==(event&&!event_success?1u:0u),"real FIFO post completion");
 if(event&&!event_success){
   char restored[256]{};size_t count=0;
   check(p::runtime::drain_one_queued_event(restored,sizeof(restored),&count),"retry drains FIFO");
   check(count==n&&std::memcmp(restored,payload,n)==0,"exact retry payload and FIFO order");
 }
 driver.completed(true,n);check(terminal==1&&progress()==1,"duplicate ignored; store unchanged");
 check(p::runtime::clear_queued_events(),"final FIFO clear");
 p::runtime::clear_behavioral_service_state_for_test();
}
static void rollback_without_response(){
 check(p::runtime::clear_queued_events(),"abort setup FIFO clean");
 init_store();check(progress()==0,"abort initially zero");
 constexpr char message[]="<LSX><Event sender=\"EbisuSDK\"><IGOEvent State=\"DOWN\"/></Event></LSX>";
 constexpr size_t length=sizeof(message)-1;
 check(p::runtime::publish_achievement_event(message,length),"abort publish FIFO");
 auto* tx=prepare(message,length);
 check(progress()==0,"prepare must not persist before Response");
 d::destroy_prepared_response_transaction(tx);
 check(progress()==0,"abort rollback keeps store pristine");
 check(p::runtime::queued_event_count_for_test()==1u,"abort FIFO retry retained");
 char output[256]{};size_t n=0;
 check(p::runtime::drain_one_queued_event(output,sizeof(output),&n),"abort drain");
 check(n==length && std::memcmp(output,message,n)==0,"abort exact FIFO data");
 check(p::runtime::clear_queued_events(),"abort finish FIFO clean");
 p::runtime::clear_behavioral_service_state_for_test();
}
int main(){
 try{
  for(int i=0;i<20;++i){one(false,false,false);one(true,false,false);
     one(true,true,false);one(true,false,true);rollback_without_response();}
  std::printf("HF12_STAGE04D_REAL_STORE_FIFO_OWNER PASS checks=%d\n",checks);
 }catch(const std::exception& ex){std::fprintf(stderr,"FAIL %s at %d\n",ex.what(),checks);return 1;}
 return 0;
}
