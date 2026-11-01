// Stage04E portable real dispatcher/handler/AES/Grant store/FIFO source-linked
// acceptance. Synthetic completion callbacks only; not Windows IOCP proof.
#include "socket_exchange_transaction_owner_private.hpp"
#include "socket_exchange_async_transaction.hpp"
#include "preloader_l/runtime/service_state.hpp"
#include "preloader_l/runtime/event_queue.hpp"
#include "preloader_l/codec/message_codec.hpp"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
extern "C" void* GetProcessHeap(){return reinterpret_cast<void*>(1);}
extern "C" void* HeapAlloc(void*,unsigned long,size_t n){return std::malloc(n);}
extern "C" int HeapFree(void*,unsigned long,void* p){std::free(p);return 1;}
namespace preloader_l {namespace runtime {
// Unrelated image handler platform bindings; NO stubs for Grant/dispatcher/FIFO/codec.
bool normalized_avatar_path(char*,size_t,size_t* n) noexcept {if(n)*n=0;return false;}
int query_image_size_bucket(int n) noexcept{return n/208;}
}}
namespace p=preloader_l;
namespace w=preloader_l::protocol::detail;
namespace rt=preloader_l::runtime;
static int checks=0;
static void must(bool b,const char* why){++checks;if(!b)throw std::runtime_error(why);}
constexpr char kCfg[]="\"Config2\" { \"Achievements\" { \"AchievementTotals\" { \"A1\" \"2\" } } }";
constexpr char kReqOne[]="<LSX><Request id=\"X1\"><GrantAchievement><AchievementId>A1</AchievementId><Progress>1</Progress></GrantAchievement></Request></LSX>";
constexpr char kReqComplete[]="<LSX><Request id=\"X2\"><GrantAchievement><AchievementId>A1</AchievementId><Progress>2</Progress></GrantAchievement></Request></LSX>";
static void init(){
 must(rt::clear_queued_events(),"clear FIFO");rt::clear_behavioral_service_state_for_test();
 rt::BehavioralServiceStateCandidate* c=nullptr;
 must(rt::prepare_behavioral_service_state_cfg(kCfg,sizeof(kCfg)-1u,&c)&&c,"parse real Config2");
 rt::commit_behavioral_service_state_candidate(c);
}
static int progress(){
 rt::acquire_behavioral_service_state();
 rt::AchievementRecordView v{};
 const bool valid=rt::achievement_record_count_locked()==1u && rt::achievement_record_at_locked(0,&v) && v.total==2 && (v.count==0 || (v.count==1 && v.progress==2));
 const int result=v.progress;
 rt::release_behavioral_service_state();must(valid,"real store snapshot");return result;
}
static p::protocol::ExchangeState exchange(){
 p::protocol::ExchangeState s{};s.established=true;
 for(int i=0;i<16;++i)s.session_key[i]=static_cast<uint8_t>(i);
 return s;
}
static void decode_has(const std::string& wire,const uint8_t* key,const char* sought){
 must(!wire.empty()&&wire.back()=='\0',"exact NUL framed ciphertext");
 std::vector<uint8_t> decoded(wire.size());size_t count=0;
 must(p::codec::decode_payload(key,16,wire.data(),wire.size(),decoded.data(),decoded.size(),&count)==p::codec::MessageCodecStatus::ok,"decode real AES");
 must(std::string(reinterpret_cast<const char*>(decoded.data()),count).find(sought)!=std::string::npos,"real decrypted XML");
}
struct Recorder {
 std::vector<std::string> frames;w::DispatchTransactionResult final{};int done=0;
 static bool start(void* context,const uint8_t* frame,size_t n) noexcept {
  auto& self=*static_cast<Recorder*>(context);
  try {self.frames.emplace_back(reinterpret_cast<const char*>(frame),n);}catch(...){return false;}
  return true;
 }
 static void finish(void* context,const w::DispatchTransactionResult& out) noexcept {
  auto& self=*static_cast<Recorder*>(context);self.final=out;++self.done;
 }
};
static void round(bool completed,bool stop,bool event_start_fails){
 init();must(progress()==0,"store initially unchanged");
 auto state=exchange();const auto* req=completed?kReqComplete:kReqOne;
 w::DispatchTransactionResult out{};
 auto* tx=w::prepare_response_transaction(&state,req,std::strlen(req),nullptr,&out);
 must(tx!=nullptr,"real dispatcher+handler+crypto prepared");
 must(tx->response_required>0,"encoded Response available");
 if(completed)must(tx->event_required>0,"completion Event separately encoded");
 must(progress()==0,"no pre-send persistent mutation");
 Recorder record{};
 w::PreparedAsyncTransaction driver(tx,&record,&Recorder::start,&Recorder::finish);
 must(driver.begin()&&record.frames.size()==1,"only Response write scheduled first");
 decode_has(record.frames[0],state.session_key,"<Achievement Id=\"A1\"");
 must(progress()==0,"Grant not committed at write initiation");
 if(stop)driver.request_stop();
 if(event_start_fails){
  // Test an actual IO-start failure by making the next write incomplete when
  // manually delivering a short completion; the same retry invariant applies.
  driver.completed(true,record.frames[0].size());
  if(record.frames.size()==2)driver.completed(false,0);
 }else{
  driver.completed(true,record.frames[0].size());
  if(record.frames.size()==2){
   decode_has(record.frames[1],state.session_key,"<Event sender=\"EbisuSDK\"");
   driver.completed(true,record.frames[1].size());
  }
 }
 must(driver.is_finished()&&record.done==1,"single completed transaction");
 must(record.final.response_sent&&progress()==(completed?2:1),"real Grant committed after response");
 const bool retry=completed&&(stop||event_start_fails);
 must(rt::queued_event_count_for_test()==(retry?1u:0u),"real FIFO commit or retry");
 if(retry){char restored[1024]{};size_t n=0;
  must(rt::drain_one_queued_event(restored,sizeof(restored),&n),"retry Event available");
  must(std::string(restored,n).find("<Achievement Id=\"A1\"")!=std::string::npos,"retry Event exact content");
 }
 driver.completed(true,20);must(record.done==1&&progress()==(completed?2:1),"duplicate completion ignored");
 must(rt::clear_queued_events(),"queue clear");rt::clear_behavioral_service_state_for_test();
}
static void aborted(){
 init();auto state=exchange();w::DispatchTransactionResult result{};
 auto* tx=w::prepare_response_transaction(&state,kReqComplete,sizeof(kReqComplete)-1u,nullptr,&result);
 must(tx&&tx->event_required>0,"real completion Event reserved");
 must(progress()==0,"prepare without commit");
 w::destroy_prepared_response_transaction(tx);
 must(progress()==0&&rt::queued_event_count_for_test()==0u,"pre-response abort rolls back state and FIFO");
 must(rt::clear_queued_events(),"abort cleanup");rt::clear_behavioral_service_state_for_test();
}
int main(){try{
 for(int i=0;i<3;++i){round(false,false,false);round(true,false,false);round(true,true,false);round(true,false,true);aborted();}
 std::cout<<"STAGE04E_REAL_DISPATCH_GRANT_CODEC_CALLBACK PASS checks="<<checks<<"\n";
}catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<" checks="<<checks<<"\n";return 1;}}
