// Stage04C true Windows IOCP async_write with real production Event FIFO.
// Grant store remains narrowly observed; this test does NOT claim the real
// GrantAchievement store or dispatcher is linked to the IOCP callback.
#include <asio.hpp>
#if !defined(_WIN32) || !defined(_WIN64) || !defined(ASIO_HAS_IOCP) || defined(ASIO_DISABLE_IOCP)
#error HF12 native FIFO must execute genuine Windows x64 Asio IOCP.
#endif
#include "socket_exchange_transaction_owner_private.hpp"
#include "socket_exchange_async_transaction.hpp"
#include "preloader_l/runtime/event_queue.hpp"
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
static int grants=0,checks=0;
namespace preloader_l {namespace handlers {
void commit_grant_achievement_delivery_store(GrantAchievementDelivery* g) noexcept {++grants;g->active=false;}
void finish_grant_achievement_delivery(GrantAchievementDelivery*) noexcept {}
void rollback_grant_achievement_delivery(GrantAchievementDelivery* g) noexcept {g->active=false;}
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
 tx->grant_delivery.active=true;tx->grant_delivery.grant.active=true;
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
 if(event)must(rt::publish_achievement_event(kEvent,kEventN),"publish real FIFO");
 grants=0;
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
 must(grants==1&&server->last.response_sent,"Response store boundary (grant recorder)");
 must(rt::queued_event_count_for_test()==(event&&!event_success?1u:0u),"true FIFO commit or retry");
 if(event&&!event_success){
  char restored[256]{};size_t n=0;
  must(rt::drain_one_queued_event(restored,sizeof(restored),&n),"drain retry");
  must(n==kEventN&&std::memcmp(restored,kEvent,n)==0,"exact retry event");
 }
 must(rt::clear_queued_events(),"safe FIFO shutdown after callback");
 server->driver->completed(true,9);
 must(server->ended==1,"no duplicate terminal");
 server->socket.cancel(ec);must(!ec,"Asio cancel without release");
 server->socket.close(ec);must(!ec,"Asio close without release");
 acceptor.close();client.close();io.restart();io.poll();
 must(server.use_count()==1,"callbacks drained");
}
int main(){
 try{
  for(int i=0;i<6;++i){round(false,false,false);round(true,false,false);
   round(true,true,false);round(true,false,true);}
  std::cout<<"HF12_STAGE04C_REAL_FIFO_WINDOWS_IOCP PASS checks="<<checks<<"\n";
 }catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<" checks="<<checks<<"\n";return 1;}
}
