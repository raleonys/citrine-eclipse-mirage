// HF13 internal owner transport regression. Uses REAL Asio TCP but stubbed
// classifier/dispatcher: this is NOT LSX parity, Windows IOCP, or game proof.
#include "asio_session_owner.hpp"
#include "asio_listener_owner.hpp"
#include <asio.hpp>
#include <atomic>
#include <chrono>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
using namespace preloader_l;
using namespace std::chrono_literals;
namespace preloader_l { namespace protocol {
void reset_exchange(ExchangeState* s) noexcept {if(s)std::memset(s,0,sizeof(*s));}
GreetingStatus build_challenge_greeting(uint32_t,char* p,size_t cap,size_t* n) noexcept {
 if(!p||cap<6||!n)return GreetingStatus::capacity;
 std::memcpy(p,"HELLO",6);*n=5;return GreetingStatus::ok;
}
} }
namespace preloader_l { namespace protocol { namespace detail {
uint32_t current_greeting_seed() noexcept {return 0;}
bool scan_available_wire_frame(ExchangeState* s,WireFrameView* f) noexcept {
 if(!s||!f||!s->wire_size)return false;*f={1,1};return true;
}
ReceivedWireStep classify_received_wire_frame(ExchangeState*,WireFrameView,uint32_t*) noexcept {
 return {WireStepKind::terminal,ExchangeStatus::frame_error,HandshakeResult{},0};
}
bool tolerate_malformed_frame(uint32_t*) noexcept {return false;}
void consume_wire_frame(ExchangeState*,size_t) noexcept {}
bool commit_sent_handshake(ExchangeState*,WireFrameView,const HandshakeResult&,uint32_t*) noexcept {return false;}
PreparedResponseTransaction* prepare_response_transaction(ExchangeState*,const char*,size_t,
 const core::DispatchEnvironment*,DispatchTransactionResult*) noexcept {return nullptr;}
bool prepared_response_frame(PreparedResponseTransaction*,PreparedWireFrame*) noexcept {return false;}
PreparedWriteDisposition complete_prepared_transaction_write(PreparedResponseTransaction*,bool,size_t,
 PreparedWireFrame*,DispatchTransactionResult*) noexcept {return PreparedWriteDisposition::rejected;}
void destroy_prepared_response_transaction(PreparedResponseTransaction*) noexcept {}
} } }
struct Probe {
 std::shared_ptr<runtime::AsioListenerOwner> listener;
 std::shared_ptr<runtime::AsioSessionOwner> session;
 std::atomic<unsigned> finished{0},quiesced{0},failed{0};
 static void done(void* p,protocol::ExchangeStatus status) noexcept {
  auto* self=static_cast<Probe*>(p);
  if(status!=protocol::ExchangeStatus::frame_error)++self->failed;
  ++self->finished;
  if(!self->listener->session_finished())++self->failed;
  if(!self->listener->request_stop())++self->failed;
 }
 static void accept(void* p,asio::ip::tcp::socket&& socket,const asio::ip::tcp::endpoint& addr) noexcept {
  auto* self=static_cast<Probe*>(p);
  try {
   self->session=std::make_shared<runtime::AsioSessionOwner>(std::move(socket));
   if(!self->session->start(addr.address().to_string(),{self,&done}))++self->failed;
  } catch (...) {++self->failed;}
 }
 static void stopped(void* p) noexcept {++static_cast<Probe*>(p)->quiesced;}
 static void error(void* p,const asio::error_code&) noexcept {++static_cast<Probe*>(p)->failed;}
};
int main() {
 try {
  asio::io_context io; Probe p;
  p.listener=std::make_shared<runtime::AsioListenerOwner>(io);
  asio::error_code ec;
  if(!p.listener->bind_loopback(0,ec))throw std::runtime_error("bind");
  auto port=p.listener->bound_port(ec);
  if(!port||!p.listener->start({&p,&Probe::accept,&Probe::stopped,&Probe::error}))
   throw std::runtime_error("start");
  asio::ip::tcp::socket client(io);
  client.connect({asio::ip::address_v4::loopback(),port},ec);
  if(ec)throw std::runtime_error("connect");
  std::thread worker([&]{io.run_for(1500ms);});
  char greeting[6]{};
  asio::read(client,asio::buffer(greeting),ec);
  if(ec||std::memcmp(greeting,"HELLO",6)!=0)throw std::runtime_error("greeting");
  asio::write(client,asio::buffer("?",1),ec);
  if(ec)throw std::runtime_error("send");
  worker.join();
  if(p.failed||p.finished!=1u||p.quiesced!=1u)throw std::runtime_error("lifecycle");
  client.close(ec);
  p.session.reset();p.listener.reset();io.restart();io.poll();
  std::cout<<"HF13_ASIO_SESSION_OWNER_TRANSPORT: PASS\n";
 }catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<"\n";return 1;}
}
