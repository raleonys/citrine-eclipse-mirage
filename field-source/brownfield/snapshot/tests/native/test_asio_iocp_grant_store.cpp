// Stage04D native: real IOCP, real FIFO, real persistent Grant service-state
// transaction. The test adapter invokes genuine service_state_grant.cpp but
// does not claim handler XML preparation or the complete LSX dispatcher.
#include <asio.hpp>
#if !defined(_WIN32) || !defined(_WIN64) || !defined(ASIO_HAS_IOCP) || defined(ASIO_DISABLE_IOCP)
#error HF12 native FIFO must execute genuine Windows x64 Asio IOCP.
#endif
#include "socket_exchange_transaction_owner_private.hpp"
#include "socket_exchange_async_transaction.hpp"
#include "preloader_l/runtime/event_queue.hpp"
#include "preloader_l/runtime/service_state.hpp"
#include <chrono>
#include <cstring>
#include <iostream>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>
using namespace std::chrono_literals;
using asio::ip::tcp;
namespace w=preloader_l::protocol::detail;
namespace rt=preloader_l::runtime;
static int checks=0;
constexpr char kCfg[]="\"Config2\" { \"Achievements\" { \"AchievementTotals\" { \"A1\" \"2\" } } }";
constexpr char kStamp[]="2026-10-14T00:00:00";
// This limited linkage adapter delegates persistent mutation to the REAL
// runtime implementation. It is not a substitute for linking the Grant handler.
namespace preloader_l {namespace handlers {
void commit_grant_achievement_delivery_store(GrantAchievementDelivery* g) noexcept {
 if(!g||!g->active||!g->transaction)return;
 rt::acquire_behavioral_service_state();
 rt::commit_grant_achievement_locked(g->transaction);
 rt::release_behavioral_service_state();g->transaction=nullptr;
}
void finish_grant_achievement_delivery(GrantAchievementDelivery* g) noexcept {
 if(!g||!g->active)return;
 if(g->event){rt::commit_prepared_achievement_event(g->event);g->event=nullptr;}
 g->active=false;rt::release_grant_achievement_delivery();
}
void rollback_grant_achievement_delivery(GrantAchievementDelivery* g) noexcept {
 if(!g||!g->active)return;
 if(g->transaction){rt::release_grant_achievement_transaction(g->transaction);g->transaction=nullptr;}
 if(g->event){rt::release_prepared_achievement_event(g->event);g->event=nullptr;}
 g->active=false;rt::release_grant_achievement_delivery();
}
}}
namespace preloader_l {namespace runtime {
LocalSocialResult commit_local_social_mutation(const LocalSocialSession&,LocalSocialPending* p) noexcept {p->active=false;return LocalSocialResult::ok;}
void rollback_local_social_mutation(LocalSocialPending* p) noexcept {p->active=false;}
}}
static void must(bool b,const char* s){++checks;if(!b)throw std::runtime_error(s);}
static void* alloc(void* h,size_t n) noexcept {return HeapAlloc(h,0,n);}
static int dealloc(void* h,void* p) noexcept {return HeapFree(h,0,p)?1:0;}
constexpr char kResponse[]="RESPONSE";
constexpr size_t kResponseN=sizeof(kResponse)-1;
static_assert(kResponseN==8,"Full fixture Response must be eight bytes");
constexpr char kEvent[]="<LSX><Event sender=\"EbisuSDK\"><IGOEvent State=\"DOWN\"/></Event></LSX>";
constexpr size_t kEventN=sizeof(kEvent)-1;
static void init_store(){
 rt::clear_behavioral_service_state_for_test();
 rt::BehavioralServiceStateCandidate* c=nullptr;
 must(rt::prepare_behavioral_service_state_cfg(kCfg,sizeof(kCfg)-1u,&c)&&c,"real store cfg");
 rt::commit_behavioral_service_state_candidate(c);
}
static int progress(){
 rt::acquire_behavioral_service_state();
 rt::AchievementRecordView view{};
 bool ok=rt::achievement_record_count_locked()==1u && rt::achievement_record_at_locked(0,&view)
         && view.total==2 && view.count==0;
 const int value=view.progress;
 rt::release_behavioral_service_state();
 must(ok,"real Grant store snapshot");
 return value;
}
static w::PreparedResponseTransaction* prepare(bool with_event){
 const preloader_l::runtime::AllocatorPlatformOps ops{GetProcessHeap(),alloc,dealloc,nullptr};
 void* p=rt::allocate_transient_bytes(sizeof(w::PreparedResponseTransaction),ops);
 must(p!=nullptr,"allocate owner");
 auto* tx=new(p) w::PreparedResponseTransaction{};
 const size_t size=kResponseN+(with_event?kEventN:0);
 tx->wire.data=static_cast<char*>(rt::allocate_transient_bytes(size+1u,ops));
 must(tx->wire.data!=nullptr,"allocate frame");
 tx->wire.capacity=size+1u;
 std::memcpy(tx->wire.data,kResponse,kResponseN);
 tx->response_required=kResponseN;
 tx->event_required=with_event?kEventN:0;
 if(with_event){
   size_t n=0;
   must(rt::begin_queued_event_delivery(nullptr,tx->wire.data+kResponseN,kEventN+1,&n,
        &tx->generic_event_delivery.queue),"real FIFO reserve");
   must(n==kEventN&&std::memcmp(tx->wire.data+kResponseN,kEvent,kEventN)==0,"real FIFO exact bytes");
 }
 // Stage and retain an actual service-state transaction, not a commit counter.
 rt::acquire_grant_achievement_delivery();
 rt::GrantAchievementTransaction* mutation=nullptr;
 rt::AchievementRecordView projected{};
 bool mutated=false,completed=false,inserted=false;
 rt::acquire_behavioral_service_state();
 bool prepared=rt::prepare_grant_achievement_locked({"A1",2},1,{kStamp,sizeof(kStamp)-1u},
        &mutation,&projected,&mutated,&completed,&inserted);
 rt::release_behavioral_service_state();
 must(prepared&&mutation&&mutated&&!completed&&!inserted&&projected.progress==1,"real Grant reserve");
 tx->grant_delivery.active=true;tx->grant_delivery.grant={mutation,nullptr,true};
 must(w::begin_delivery_writes(&tx->cursor,kResponseN,tx->event_required),"cursor");
 return tx;
}
struct Session:std::enable_shared_from_this<Session>{
 tcp::socket socket;
 std::unique_ptr<w::PreparedAsyncTransaction> driver;
 bool stop_after_response{false};
 bool reject_event_launch{false};
 int launched=0,ended=0;
 w::DispatchTransactionResult last{};
 explicit Session(tcp::socket s):socket(std::move(s)){}
 static bool start(void* ctx,const uint8_t* bytes,size_t length) noexcept {
  auto* s=static_cast<Session*>(ctx);++s->launched;
  if(s->reject_event_launch&&s->launched==2)return false;
  auto held=s->shared_from_this();
  asio::async_write(s->socket,asio::buffer(bytes,length),[held](const asio::error_code& ec,size_t n){
   if(held->stop_after_response&&held->launched==1)held->driver->request_stop();
   held->driver->completed(!ec,n);
  });
  return true;
 }
 static void finish(void* ctx,const w::DispatchTransactionResult& r) noexcept {
  auto* s=static_cast<Session*>(ctx);++s->ended;s->last=r;
 }
 void begin(bool with_event){
  driver.reset(new w::PreparedAsyncTransaction(prepare(with_event),this,&Session::start,&Session::finish));
  must(driver->begin(),"first callback queue");
 }
};
static void round(bool event,bool stop,bool reject){
 must(rt::clear_queued_events(),"clean queue at start");
 init_store();must(progress()==0,"initial persistent progress");
 if(event)must(rt::publish_achievement_event(kEvent,kEventN),"publish real FIFO");

 asio::io_context io;
 tcp::acceptor acceptor(io,tcp::endpoint(asio::ip::address_v4::loopback(),0));
 tcp::socket client(io);
 std::shared_ptr<Session> server;
 bool accepted=false,read_done=false,bad=false;
 size_t actual_bytes=0;
 std::string read_error;
 acceptor.async_accept([&](const asio::error_code& ec,tcp::socket sock){
  if(ec){bad=true;return;}
  accepted=true;server=std::make_shared<Session>(std::move(sock));
  server->stop_after_response=stop;server->reject_event_launch=reject;
  server->begin(event);
 });
 asio::error_code ec;
 client.connect(acceptor.local_endpoint(),ec);must(!ec,"client connects");
 const bool event_success=event&&!stop&&!reject;
 const std::string expected=std::string(kResponse,kResponseN)+(event_success?std::string(kEvent,kEventN):std::string());
 std::string actual(expected.size(),'\0');
 asio::async_read(client,asio::buffer(actual),[&](const asio::error_code& e,size_t n){
  actual_bytes=n;
  if(e)read_error=e.message();
  if(e||n!=expected.size()||actual!=expected)bad=true;
  read_done=true;
 });
 io.run_for(1900ms);
 if(!accepted||!read_done||bad) {
  std::cerr<<"FIFO_WIRE_DIAGNOSTIC accepted="<<accepted
           <<" read_done="<<read_done<<" bad="<<bad
           <<" expected_bytes="<<expected.size()
           <<" observed_bytes="<<actual_bytes
           <<" error="<<read_error<<"\n";
 }
 must(accepted&&read_done&&!bad,"real IOCP FIFO payload on wire");
 must(server&&server->driver&&server->driver->is_finished()&&server->ended==1,"driver finished");
 must(server->last.response_sent&&progress()==1,"real persistent Grant commit only after full Response");
 must(rt::queued_event_count_for_test()==(event&&!event_success?1u:0u),"true FIFO commit or retry");
 if(event&&!event_success){
  char restored[256]{};size_t n=0;
  must(rt::drain_one_queued_event(restored,sizeof(restored),&n),"drain retry");
  must(n==kEventN&&std::memcmp(restored,kEvent,n)==0,"exact retry event");
 }
 must(rt::clear_queued_events(),"safe FIFO shutdown after callback");
 server->driver->completed(true,9);
 must(server->ended==1&&progress()==1,"duplicate callback cannot commit Grant twice");
 server->socket.cancel(ec);must(!ec,"Asio cancel without release");
 server->socket.close(ec);must(!ec,"Asio close without release");
 acceptor.close();client.close();io.restart();io.poll();
 must(server.use_count()==1,"callbacks drained");
 rt::clear_behavioral_service_state_for_test();
}
static void rollback_without_response(){
 must(rt::clear_queued_events(),"abort clean FIFO");
 init_store();must(progress()==0,"abort pristine store");
 must(rt::publish_achievement_event(kEvent,kEventN),"abort publish FIFO");
 auto* tx=prepare(true);
 must(progress()==0,"no persistent commit in prepare");
 w::destroy_prepared_response_transaction(tx);
 must(progress()==0,"rollback before Response");
 must(rt::queued_event_count_for_test()==1u,"rollback leaves FIFO retryable");
 char restored[256]{};size_t n=0;
 must(rt::drain_one_queued_event(restored,sizeof(restored),&n),"rollback FIFO drain");
 must(n==kEventN&&std::memcmp(restored,kEvent,n)==0,"rollback exact Event data");
 must(rt::clear_queued_events(),"abort FIFO clear");
 rt::clear_behavioral_service_state_for_test();
}
int main(){
 try{
  for(int i=0;i<6;++i){round(false,false,false);round(true,false,false);
   round(true,true,false);round(true,false,true);rollback_without_response();}
  std::cout<<"HF12_STAGE04D_REAL_STORE_FIFO_WINDOWS_IOCP PASS checks="<<checks<<"\n";
 }catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<" checks="<<checks<<"\n";return 1;}
}
