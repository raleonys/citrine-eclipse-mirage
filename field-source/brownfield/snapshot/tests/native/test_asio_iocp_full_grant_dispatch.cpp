// Stage04E native acceptance CANDIDATE: complete real XML->handler->store->
// AES/hex+NUL response/event preparation and IOCP callback completion.
// No fake handler, service state, FIFO or codec. NOT a shipping Asio session.
#include <asio.hpp>
#if !defined(_WIN32) || !defined(_WIN64) || !defined(ASIO_HAS_IOCP) || defined(ASIO_DISABLE_IOCP)
#error Genuine Windows x64 IOCP mandatory for Stage04E.
#endif
#include "socket_exchange_transaction_owner_private.hpp"
#include "socket_exchange_async_transaction.hpp"
#include "preloader_l/codec/message_codec.hpp"
#include "preloader_l/runtime/service_state.hpp"
#include "preloader_l/runtime/event_queue.hpp"
#include <chrono>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std::chrono_literals;
using asio::ip::tcp;
namespace p=preloader_l;
namespace w=preloader_l::protocol::detail;
namespace rt=preloader_l::runtime;
// Unrelated image-route host surface only. No mock XML dispatcher, Grant
// handler, persistent state, event queue, codec or socket completion.
namespace preloader_l {namespace runtime {
bool normalized_avatar_path(char*,size_t,size_t* written) noexcept {if(written)*written=0;return false;}
int32_t query_image_size_bucket(int32_t width) noexcept {return width/208;}
}}
static int checks=0;
static void must(bool condition,const char* what){++checks;if(!condition)throw std::runtime_error(what);}
constexpr char kCfg[]="\"Config2\" { \"Achievements\" { \"AchievementTotals\" { \"A1\" \"2\" } } }";
constexpr char kRequestOne[]="<LSX><Request id=\"E1\"><GrantAchievement><AchievementId>A1</AchievementId><Progress>1</Progress></GrantAchievement></Request></LSX>";
constexpr char kRequestComplete[]="<LSX><Request id=\"E2\"><GrantAchievement><AchievementId>A1</AchievementId><Progress>2</Progress></GrantAchievement></Request></LSX>";
static void init_store(){
 must(rt::clear_queued_events(),"FIFO clean at start");
 rt::clear_behavioral_service_state_for_test();
 rt::BehavioralServiceStateCandidate* c=nullptr;
 must(rt::prepare_behavioral_service_state_cfg(kCfg,sizeof(kCfg)-1,&c)&&c,"Config2 real state parse");
 rt::commit_behavioral_service_state_candidate(c);
}
static int progress(){
 rt::acquire_behavioral_service_state();
 rt::AchievementRecordView v{};
 bool ok=rt::achievement_record_count_locked()==1u && rt::achievement_record_at_locked(0,&v)
  && v.total==2 && (v.count==0 || (v.count==1 && v.progress==2));
 const int value=v.progress;
 rt::release_behavioral_service_state();
 must(ok,"real persistent service record");
 return value;
}
static p::protocol::ExchangeState new_exchange(){
 p::protocol::ExchangeState state{};
 state.established=true;
 for(size_t i=0;i<16;++i)state.session_key[i]=static_cast<uint8_t>(i);
 return state;
}
static void wire_contains(const std::string& wire,const uint8_t key[16],const char* fragment){
 must(!wire.empty()&&wire.back()=='\0',"encrypted LSX terminal NUL");
 std::vector<uint8_t> plain(wire.size());size_t count=0;
 must(p::codec::decode_payload(key,16,wire.data(),wire.size(),plain.data(),plain.size(),&count)==p::codec::MessageCodecStatus::ok,"actual AES/hex decode");
 must(std::string(reinterpret_cast<const char*>(plain.data()),count).find(fragment)!=std::string::npos,"decrypted real LSX fragment");
}
struct Session final:std::enable_shared_from_this<Session>{
 tcp::socket socket;
 p::protocol::ExchangeState state{};
 std::unique_ptr<w::PreparedAsyncTransaction> driver;
 bool stop_on_response{false},reject_event{false};
 int launched{0},finished{0};
 w::DispatchTransactionResult outcome{};
 size_t response_n=0,event_n=0;
 explicit Session(tcp::socket sock):socket(std::move(sock)),state(new_exchange()){}
 static bool start(void* context,const uint8_t* data,size_t n) noexcept{
  auto* s=static_cast<Session*>(context);++s->launched;
  if(s->reject_event&&s->launched==2)return false;
  auto retain=s->shared_from_this();
  asio::async_write(s->socket,asio::buffer(data,n),[retain](const asio::error_code& ec,size_t count){
    if(retain->stop_on_response&&retain->launched==1)retain->driver->request_stop();
    retain->driver->completed(!ec,count);
  });
  return true;
 }
 static void end(void* context,const w::DispatchTransactionResult& result) noexcept{
  auto* s=static_cast<Session*>(context);s->outcome=result;++s->finished;
 }
 void begin(const char* xml,bool stop,bool reject){
  stop_on_response=stop;reject_event=reject;
  w::DispatchTransactionResult prepared{};
  auto* tx=w::prepare_response_transaction(&state,xml,std::strlen(xml),nullptr,&prepared);
  must(tx!=nullptr,"real XML dispatcher+GrantAchievement+AES prepare");
  response_n=tx->response_required;event_n=tx->event_required;
  must(response_n>0,"real encoded Response available");
  if(std::strcmp(xml,kRequestComplete)==0)must(event_n>0,"completion Event has separate ciphertext frame");
  must(progress()==0,"prepare must not commit persistent Grant");
  driver=std::make_unique<w::PreparedAsyncTransaction>(tx,this,&Session::start,&Session::end);
  must(driver->begin(),"Asio first write scheduled");
 }
};
static void round(bool complete,bool stop,bool reject){
 init_store();must(progress()==0,"baseline Progress=0");
 asio::io_context io;
 tcp::acceptor acceptor(io,tcp::endpoint(asio::ip::address_v4::loopback(),0));
 tcp::socket client(io);
 std::shared_ptr<Session> session;
 bool accepted=false,read_done=false,bad=false;
 std::string received,read_error;
 const char* request=complete?kRequestComplete:kRequestOne;
 acceptor.async_accept([&](const asio::error_code& ec,tcp::socket peer){
  if(ec){bad=true;return;}
  accepted=true;session=std::make_shared<Session>(std::move(peer));
  session->begin(request,stop,reject);
  const auto wanted=session->response_n+((stop||reject)?0u:session->event_n);
  received.resize(wanted); // callback will not outlive this scope
  asio::async_read(client,asio::buffer(received),[&](const asio::error_code& error,size_t count){
   if(error||count!=received.size()){bad=true;read_error=error.message();}
   read_done=true;
  });
 });
 asio::error_code ec;
 client.connect(acceptor.local_endpoint(),ec);must(!ec,"loopback connect");
 io.run_for(2500ms);
 if(!accepted||!read_done||bad)
  std::cerr<<"DISPATCH_IOCP_DIAGNOSTIC accepted="<<accepted<<" read="<<read_done
   <<" bad="<<bad<<" wire_bytes="<<received.size()<<" error="<<read_error<<"\n";
 must(accepted&&read_done&&!bad,"full encrypted LSX frames arrived via IOCP");
 must(session&&session->driver&&session->driver->is_finished()&&session->finished==1,"callback owner drained");
 must(session->outcome.response_sent&&progress()==(complete?2:1),"true handler Grant commits after Response");
 const std::string response=received.substr(0,session->response_n);
 wire_contains(response,session->state.session_key,"<Achievement Id=\"A1\"");
 if(complete&&!stop&&!reject){
  const std::string event=received.substr(session->response_n);
  wire_contains(event,session->state.session_key,"<Event sender=\"EbisuSDK\"");
 }
 must(rt::queued_event_count_for_test()==(complete&&(stop||reject)?1u:0u),"real Event FIFO commit or retry");
 if(complete&&(stop||reject)){
  char retry[1024]{};size_t count=0;
  must(rt::drain_one_queued_event(retry,sizeof(retry),&count)&&count>0,"retry Event available");
  must(std::string(retry,count).find("<Achievement Id=\"A1\"")!=std::string::npos,"retry retains exact achievement payload");
 }
 session->driver->completed(true,42u);
 must(session->finished==1&&progress()==(complete?2:1),"late callback cannot duplicate Grant");
 session->socket.cancel(ec);must(!ec,"Asio-owned cancel");
 session->socket.close(ec);must(!ec,"Asio-owned close");
 acceptor.close();client.close();io.restart();io.poll();
 must(session.use_count()==1,"asynchronous callbacks quiesced");
 must(rt::clear_queued_events(),"FIFO clear after callbacks");
 rt::clear_behavioral_service_state_for_test();
}
static void abort_before_response(){
 init_store();
 auto state=new_exchange();w::DispatchTransactionResult out{};
 auto* prepared=w::prepare_response_transaction(&state,kRequestComplete,sizeof(kRequestComplete)-1,nullptr,&out);
 must(prepared!=nullptr&&prepared->event_required>0,"real completion transaction prepared");
 must(progress()==0,"no pre-send store mutation");
 w::destroy_prepared_response_transaction(prepared);
 must(progress()==0,"abort before Response leaves persistent state unchanged");
 must(rt::queued_event_count_for_test()==0,"prepared Event rolled back without publishing");
 must(rt::clear_queued_events(),"FIFO clear on aborted request");
 rt::clear_behavioral_service_state_for_test();
}
int main(){
 try{
  for(int i=0;i<3;++i){round(false,false,false);round(true,false,false);
   round(true,true,false);round(true,false,true);abort_before_response();}
  std::cout<<"HF12_STAGE04E_FULL_GRANT_DISPATCH_WINDOWS_IOCP PASS checks="<<checks<<"\n";
 }catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<" checks="<<checks<<"\n";return 1;}
}
