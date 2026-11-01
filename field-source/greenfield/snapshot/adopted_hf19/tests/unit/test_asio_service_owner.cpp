// HF13 internal owner transport regression. Uses REAL Asio TCP but stubbed
// classifier/dispatcher: this is NOT LSX parity, Windows IOCP, or game proof.
#include "asio_session_owner.hpp"
#include "asio_service_owner.hpp"
#include <vector>
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
 std::shared_ptr<runtime::AsioServiceOwner> owner;
 std::atomic<unsigned> sessions{0}, quiesced{0}, failed{0}, bad_status{0}, admitted{0};
 static void finished(void* ptr,protocol::ExchangeStatus st) noexcept {
  auto* p=static_cast<Probe*>(ptr);
  if(st!=protocol::ExchangeStatus::frame_error && st!=protocol::ExchangeStatus::peer_closed)
    ++p->bad_status;
  ++p->sessions;
 }
 static void stopped(void* ptr) noexcept {++static_cast<Probe*>(ptr)->quiesced;}
 static void error(void* ptr,const asio::error_code&) noexcept {++static_cast<Probe*>(ptr)->failed;}
 static bool accepted(void* ptr) noexcept {
  ++static_cast<Probe*>(ptr)->admitted;return true;
 }
 runtime::AsioServiceObserver observer() noexcept {
  return {this,&stopped,&error,&finished,&accepted};
 }
};
static void connect_and_finish(unsigned clients) {
 asio::io_context io;
 Probe p;
 p.owner=std::make_shared<runtime::AsioServiceOwner>(io);
 asio::error_code ec;
 if(!p.owner->bind_loopback(0,ec)) throw std::runtime_error("bind");
 auto port=p.owner->bound_port(ec);
 if(ec||!port||!p.owner->start(p.observer())) throw std::runtime_error("start");
 std::thread worker([&]{io.run_for(3000ms);});
 for(unsigned i=0;i<clients;++i) {
  asio::ip::tcp::socket client(io);
  client.connect({asio::ip::address_v4::loopback(),port},ec);
  if(ec)throw std::runtime_error("connect");
  char greeting[6]{};
  asio::read(client,asio::buffer(greeting),ec);
  if(ec||std::memcmp(greeting,"HELLO",6)!=0) throw std::runtime_error("greeting");
  asio::write(client,asio::buffer("?",1),ec);
  if(ec)throw std::runtime_error("write");
  char x='a';
  const auto n=client.read_some(asio::buffer(&x,1),ec);
  if(n!=0||ec!=asio::error::eof) throw std::runtime_error("termination");
 }
 if(!p.owner->request_stop())throw std::runtime_error("stop post");
 worker.join();
 if(p.sessions!=clients||p.admitted!=clients||p.quiesced!=1||p.failed||p.bad_status)
  throw std::runtime_error("completion and quiescence");
 auto weak=std::weak_ptr<runtime::AsioServiceOwner>(p.owner);
 p.owner.reset();
 io.restart();io.poll();
 if(!weak.expired())throw std::runtime_error("owner leaked");
}
static void cancel_open_sessions() {
 asio::io_context io;
 Probe p;
 p.owner=std::make_shared<runtime::AsioServiceOwner>(io);
 asio::error_code ec;
 if(!p.owner->bind_loopback(0,ec))throw std::runtime_error("bind2");
 auto port=p.owner->bound_port(ec);
 if(!p.owner->start(p.observer()))throw std::runtime_error("start2");
 std::thread worker([&]{io.run_for(3000ms);});
 std::vector<std::unique_ptr<asio::ip::tcp::socket>> clients;
 for(unsigned i=0;i<3;++i) {
  auto client=std::make_unique<asio::ip::tcp::socket>(io);
  client->connect({asio::ip::address_v4::loopback(),port},ec);
  if(ec)throw std::runtime_error("connect2");
  char greeting[6]{};
  asio::read(*client,asio::buffer(greeting),ec);
  if(ec||std::memcmp(greeting,"HELLO",6)!=0)throw std::runtime_error("greeting2");
  clients.emplace_back(std::move(client));
 }
 if(!p.owner->request_stop())throw std::runtime_error("stop2");
 worker.join();
 if(p.sessions!=3||p.admitted!=3||p.quiesced!=1||p.failed||p.bad_status)
  throw std::runtime_error("cancel session quiescence");
 clients.clear();
 auto weak=std::weak_ptr<runtime::AsioServiceOwner>(p.owner);
 p.owner.reset();io.restart();io.poll();
 if(!weak.expired())throw std::runtime_error("cancel owner leaked");
}
static void aborted_unstarted_bind() {
 asio::io_context io;
 auto owner=std::make_shared<runtime::AsioServiceOwner>(io);
 asio::error_code ec;
 if(!owner->bind_loopback(0,ec)) throw std::runtime_error("prebind");
 if(!owner->abort_before_start(ec)||ec) throw std::runtime_error("abort prestart");
 auto weak=std::weak_ptr<runtime::AsioServiceOwner>(owner);
 owner.reset();
 if(!weak.expired()) throw std::runtime_error("prestart owner leaked");
}
int main() {
 try {
  aborted_unstarted_bind();
  connect_and_finish(1);
  connect_and_finish(4);
  cancel_open_sessions();
  std::cout << "HF13_ASIO_SERVICE_OWNER_REAL_TCP: PASS\n";
 } catch(const std::exception& e) {
  std::cerr<<"FAIL "<<e.what()<<"\n";return 1;
 }
}
