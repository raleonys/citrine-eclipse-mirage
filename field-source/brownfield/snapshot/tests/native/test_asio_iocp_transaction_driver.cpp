// Stage04B genuine Windows IOCP write callbacks invoke the REAL shipping
// prepared owner + source-shared async driver. Store and FIFO side effects
// are narrow TEST recorders, not the production achievement/event services.
#include <asio.hpp>
#if !defined(_WIN32) || !defined(_WIN64) || !defined(ASIO_HAS_IOCP) || defined(ASIO_DISABLE_IOCP)
#error HF12 Stage04B native must use Windows x64 Asio IOCP.
#endif
// Include exact production TU to expose its intentionally opaque owner for
// deterministic native fixtures; do not introduce any shipping test exports.
#include "socket_exchange_transaction_owner_private.hpp"
#include "socket_exchange_async_transaction.hpp"
#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
using asio::ip::tcp;
using namespace std::chrono_literals;
namespace w=preloader_l::protocol::detail;
static int store_commits=0,event_commits=0,event_retries=0,event_rollbacks=0,social_commits=0;
namespace preloader_l {namespace handlers {
void commit_grant_achievement_delivery_store(GrantAchievementDelivery* g) noexcept {++store_commits;g->active=false;}
void finish_grant_achievement_delivery(GrantAchievementDelivery*) noexcept {}
void rollback_grant_achievement_delivery(GrantAchievementDelivery* g) noexcept {g->active=false;}
}}
namespace preloader_l {namespace runtime {
void commit_queued_event_delivery(QueuedEventDelivery* e) noexcept {if(e->active){++event_commits;e->active=false;}}
void rollback_queued_event_delivery(QueuedEventDelivery* e) noexcept {if(e->active){++event_rollbacks;e->active=false;}}
void resolve_queued_event_delivery_for_retry(QueuedEventDelivery* e) noexcept {if(e->active){++event_retries;e->active=false;}}
LocalSocialResult commit_local_social_mutation(const LocalSocialSession&,LocalSocialPending* p) noexcept {++social_commits;p->active=false;return LocalSocialResult::ok;}
void rollback_local_social_mutation(LocalSocialPending* p) noexcept {p->active=false;}
}}
static int assertions=0;
static void must(bool x,const char* label) {++assertions;if(!x)throw std::runtime_error(label);}
static void* heap_alloc(void* heap,size_t n) noexcept {return HeapAlloc(heap,0,n);}
static int heap_free(void* heap,void* p) noexcept {return HeapFree(heap,0,p)?1:0;}
static w::PreparedResponseTransaction* make_real_owner(bool event) {
 auto* heap=GetProcessHeap();
 const preloader_l::runtime::AllocatorPlatformOps ops{heap,&heap_alloc,&heap_free,nullptr};
 void* storage=preloader_l::runtime::allocate_transient_bytes(sizeof(w::PreparedResponseTransaction),ops);
 if(!storage)return nullptr;
 auto* tx=new(storage) w::PreparedResponseTransaction{};
 tx->wire.data=static_cast<char*>(preloader_l::runtime::allocate_transient_bytes(event?12u:7u,ops));
 if(!tx->wire.data){w::destroy_prepared_response_transaction(tx);return nullptr;}
 tx->wire.capacity=event?12u:7u;
 tx->response_required=7u;tx->event_required=event?5u:0u;
 for(size_t i=0;i<7;++i)tx->wire.data[i]='R';
 if(event)for(size_t i=0;i<5;++i)tx->wire.data[i+7]='E';
 if(!w::begin_delivery_writes(&tx->cursor,7u,event?5u:0u)) {
  w::destroy_prepared_response_transaction(tx);return nullptr;
 }
 tx->grant_delivery.active=true;tx->grant_delivery.grant.active=true;
 tx->generic_event_delivery.queue.active=event;
 return tx;
}
struct Server:std::enable_shared_from_this<Server> {
 tcp::socket socket;
 std::unique_ptr<w::PreparedAsyncTransaction> driver;
 bool stop_after_response{false};
 bool reject_event_launch{false};
 int initiated=0,terminal=0;
 w::DispatchTransactionResult finished{};
 explicit Server(tcp::socket s):socket(std::move(s)){}
 static bool send(void* ctx,const uint8_t* bytes,size_t n) noexcept {
  auto* self=static_cast<Server*>(ctx);
  ++self->initiated;
  if(self->reject_event_launch && self->initiated==2)return false;
  auto held=self->shared_from_this();
  asio::async_write(self->socket,asio::buffer(bytes,n),
     [held](const asio::error_code& ec,size_t transferred) {
      if(held->stop_after_response && held->initiated==1)
          held->driver->request_stop();
      held->driver->completed(!ec,transferred);
     });
  return true;
 }
 static void end(void* ctx,const w::DispatchTransactionResult& result) noexcept {
  auto* self=static_cast<Server*>(ctx);++self->terminal;self->finished=result;
 }
 void start(bool event) {
  auto* prepared=make_real_owner(event);
  if(!prepared)throw std::runtime_error("true allocator preparation failed");
  driver.reset(new w::PreparedAsyncTransaction(prepared,this,&Server::send,&Server::end));
  if(!driver->begin())throw std::runtime_error("initial native async write start failed");
 }
};
static void one_round(bool event,bool stop,bool reject_event) {
 store_commits=event_commits=event_retries=event_rollbacks=social_commits=0;
 asio::io_context io;
 tcp::acceptor listener(io,tcp::endpoint(asio::ip::address_v4::loopback(),0));
 tcp::socket client(io);
 std::shared_ptr<Server> server;
 bool accepted=false,received=false,io_failure=false;
 listener.async_accept([&](const asio::error_code& ec,tcp::socket accepted_sock){
   if(ec){io_failure=true;return;}
   accepted=true;server=std::make_shared<Server>(std::move(accepted_sock));
   server->stop_after_response=stop;server->reject_event_launch=reject_event;
   server->start(event);
 });
 asio::error_code ec;
 client.connect(listener.local_endpoint(),ec);must(!ec,"connect");
 const bool complete_event=event&&!stop&&!reject_event;
 const std::string expected=std::string(7,'R')+(complete_event?std::string(5,'E'):std::string());
 std::string buffer(expected.size(),'\0');
 asio::async_read(client,asio::buffer(buffer),[&](const asio::error_code& e,size_t n){
   if(e||n!=expected.size()||buffer!=expected)io_failure=true;
   received=true;
 });
 io.run_for(1900ms);
 must(accepted&&!io_failure&&received,"true IOCP response/event wire");
 must(server&&server->driver&&server->driver->is_finished(),"one callback lifecycle");
 must(server->terminal==1&&store_commits==1,"full Response granted once");
 must(event_commits==(complete_event?1:0),"FIFO full Event gate");
 must(event_retries==(event&&!complete_event?1:0),"FIFO retry after cancelled/rejected Event");
 must(server->finished.response_sent,"response commit boundary");
 must(server->initiated==(event&&!stop?2:1),"serialized write launch");
 server->driver->completed(true,5);
 must(server->terminal==1,"duplicate callback ignored");
 server->socket.cancel(ec);must(!ec,"cancel");
 server->socket.close(ec);must(!ec,"close without release");
 listener.close();client.close();io.restart();io.poll();
 must(server.use_count()==1,"handlers fully drained");
}
int main(){
 try {
  for(int i=0;i<6;++i){one_round(false,false,false);one_round(true,false,false);
     one_round(true,true,false);one_round(true,false,true);}
  std::cout<<"HF12_STAGE04B_REAL_WINDOWS_IOCP_ASYNC_OWNER PASS assertions="<<assertions<<"\n";
 }catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<" after "<<assertions<<" assertions\n";return 1;}
}
