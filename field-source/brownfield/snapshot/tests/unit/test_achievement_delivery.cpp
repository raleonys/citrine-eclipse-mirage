#include "preloader_l/protocol/socket_exchange.hpp"
#include "preloader_l/net/socket_transport.hpp"
#include "preloader_l/runtime/event_queue.hpp"
#include "preloader_l/runtime/service_state.hpp"
#include "preloader_l/codec/message_codec.hpp"
#include "../../src/protocol/socket_exchange_internal.hpp"
#include <algorithm>
#include <cstdint>
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
bool normalized_avatar_path(char*,size_t,size_t* n) noexcept {if(n)*n=0;return false;}
int query_image_size_bucket(int width) noexcept {return width/208;}
}}
namespace {
size_t assertions=0;
void ck(bool b,const char* msg){++assertions;if(!b)throw std::runtime_error(msg);}
struct IO{std::string wire;size_t quota=static_cast<size_t>(-1);size_t chunk=11;unsigned calls=0;};
preloader_l::net::TransportStatus send(void* c,const uint8_t* b,size_t n,size_t* w) noexcept {
    auto* io=static_cast<IO*>(c);++io->calls;
    if(w)*w=0;
    if(io->wire.size()>=io->quota)return preloader_l::net::TransportStatus::io_error;
    const size_t k=std::min(n,std::min(io->chunk,io->quota-io->wire.size()));
    if(!k)return preloader_l::net::TransportStatus::io_error;
    io->wire.append(reinterpret_cast<const char*>(b),k);if(w)*w=k;
    return preloader_l::net::TransportStatus::ok;
}
preloader_l::net::TransportStatus recv(void*,uint8_t*,size_t,size_t* n) noexcept {if(n)*n=0;return preloader_l::net::TransportStatus::closed;}
void close(void*) noexcept {}
using TX=preloader_l::protocol::detail::DispatchTransactionResult;
TX tx(IO& io,preloader_l::protocol::ExchangeState& state,const char* req){
    preloader_l::net::TransportEndpoint ep{&io,recv,send,close};
    return preloader_l::protocol::detail::execute_response_transaction(ep,&state,req,std::strlen(req),nullptr);
}
std::vector<std::string> frames(const std::string& s){
    std::vector<std::string> result;size_t off=0;
    while(off<s.size()){
        const size_t end=s.find('\0',off);ck(end!=std::string::npos,"missing frame terminator");
        result.push_back(s.substr(off,end-off+1));off=end+1;
    }
    return result;
}
std::string plain(const std::string& s,const uint8_t* key){
    std::vector<uint8_t> decoded(s.size()+16);size_t n=0;
    ck(preloader_l::codec::decode_payload(key,16,s.data(),s.size(),decoded.data(),decoded.size(),&n)==preloader_l::codec::MessageCodecStatus::ok,"decrypt");
    ck(n>0&&n<=decoded.size(),"decrypted size");
    const size_t pad=decoded[n-1];ck(pad>=1&&pad<=16&&pad<=n,"pad");
    for(size_t i=n-pad;i<n;++i)ck(decoded[i]==pad,"pad byte");
    return std::string(reinterpret_cast<const char*>(decoded.data()),n-pad);
}
constexpr char SHOW[]="<LSX><Request id=\"301\"><ShowIGO/></Request></LSX>";
constexpr char QUERY[]="<LSX><Request id=\"302\"><GetConfig/></Request></LSX>";
constexpr char GRANT[]="<LSX><Request id=\"303\"><GrantAchievement><AchievementId>A1</AchievementId><Progress>2</Progress></GrantAchievement></Request></LSX>";
constexpr char OLDER[]="<LSX><Event sender=\"EbisuSDK\"><Older/></Event></LSX>";
void clear(){preloader_l::runtime::clear_queued_events_for_test();ck(preloader_l::runtime::queued_event_count_for_test()==0,"queue not clear");}
bool init_grant(){
    using namespace preloader_l::runtime;
    clear();clear_behavioral_service_state_for_test();
    constexpr char cfg[]="\"Config2\" { \"Achievements\" { \"AchievementTotals\" { \"A1\" \"10\" } } }";
    BehavioralServiceStateCandidate* c=nullptr;
    if(!prepare_behavioral_service_state_cfg(cfg,sizeof(cfg)-1,&c)||!c)return false;
    commit_behavioral_service_state_candidate(c);
    constexpr char stamp[]="2026-10-04T19:00:00";
    AchievementRecordView rec{};bool m=false,d=false;
    acquire_behavioral_service_state();
    bool ok=grant_achievement_locked({"A1",2},8,{stamp,sizeof(stamp)-1},&rec,&m,&d);
    release_behavioral_service_state();
    return ok&&m&&!d&&rec.progress==8;
}
int progress(){
    using namespace preloader_l::runtime;
    AchievementRecordView rec{};
    acquire_behavioral_service_state();
    const bool ok=achievement_record_at_locked(0,&rec);
    release_behavioral_service_state();
    ck(ok,"achievement record unavailable");
    return rec.progress;
}
void run(){
    using namespace preloader_l;
    clear();
    IO normal{};protocol::ExchangeState ns{};
    const auto success=tx(normal,ns,SHOW);
    ck(success.status==protocol::ExchangeStatus::ok&&success.response_sent,"show success");
    ck(success.bytes_sent==normal.wire.size(),"total wire count");
    const auto nf=frames(normal.wire);ck(nf.size()==2,"exact two independent frames");
    ck(plain(nf[0],ns.session_key).find("<Response id=\"301\"")!=std::string::npos,"response body");
    ck(plain(nf[0],ns.session_key).find("<Event")==std::string::npos,"event not fused into response");
    ck(plain(nf[1],ns.session_key).find("<IGOEvent State=\"DOWN\"/>")!=std::string::npos,"event frame");
    ck(runtime::queued_event_count_for_test()==0,"delivered event not consumed");
    const size_t response_wire=nf[0].size(),total=normal.wire.size();
    for(size_t cap:{size_t(0),size_t(1),response_wire-1,response_wire,response_wire+1,total-1,total}){
        clear();IO io{};io.quota=cap;protocol::ExchangeState s{};auto result=tx(io,s,SHOW);
        ck(io.wire==normal.wire.substr(0,cap),"wire prefix mismatch");
        ck(result.response_sent==(cap>=response_wire),"Response boundary mismatch");
        ck((result.status==protocol::ExchangeStatus::ok)==(cap==total),"status cutpoint");
        ck(runtime::queued_event_count_for_test()==((cap>=response_wire&&cap<total)?1u:0u),"retry or rollback event count");
        if(cap>=response_wire&&cap<total){
            IO recovery{};protocol::ExchangeState fresh{};auto rt=tx(recovery,fresh,QUERY);
            ck(rt.status==protocol::ExchangeStatus::ok,"retry response");
            auto rf=frames(recovery.wire);ck(rf.size()==2,"retry must carry event");
            ck(plain(rf[1],fresh.session_key).find("<IGOEvent State=\"DOWN\"/>")!=std::string::npos,"retry same event");
            ck(runtime::queued_event_count_for_test()==0,"retry consume once");
        }
    }
    clear();ck(runtime::publish_achievement_event(OLDER,sizeof(OLDER)-1),"older enqueue");
    IO fifo{};fifo.quota=response_wire;protocol::ExchangeState fs{};auto ft=tx(fifo,fs,SHOW);
    ck(ft.response_sent&&ft.status!=protocol::ExchangeStatus::ok,"FIFO Event failure status");
    ck(runtime::queued_event_count_for_test()==2u,"FIFO retry should retain selected and new event");
    char old[300]{};size_t len=0;ck(runtime::drain_one_queued_event(old,sizeof(old),&len)&&std::strcmp(old,OLDER)==0,"older FIFO head");
    char next[300]{};len=0;ck(runtime::drain_one_queued_event(next,sizeof(next),&len)&&std::strstr(next,"IGOEvent State=\"DOWN\""),"candidate FIFO second");
    clear();
    IO ordinary{};protocol::ExchangeState os{};auto noevent=tx(ordinary,os,QUERY);
    ck(noevent.status==protocol::ExchangeStatus::ok&&frames(ordinary.wire).size()==1,"response-only unchanged");

    ck(init_grant(),"initialize grant");
    IO grant_ok{};protocol::ExchangeState gs{};auto gt=tx(grant_ok,gs,GRANT);
    ck(gt.status==protocol::ExchangeStatus::ok&&gt.response_sent,"grant commit on complete response/event");
    ck(progress()==10,"grant not committed");
    ck(frames(grant_ok.wire).size()==2,"grant event not split");
    ck(runtime::queued_event_count_for_test()==0,"grant event consumed");
    const auto grant_frames=frames(grant_ok.wire);
    ck(init_grant(),"reset grant");
    IO grant_response_fail{};grant_response_fail.quota=grant_frames[0].size()-1;
    protocol::ExchangeState rfs{};auto gfail=tx(grant_response_fail,rfs,GRANT);
    ck(gfail.status!=protocol::ExchangeStatus::ok&&!gfail.response_sent,"grant response failure status");
    ck(progress()==8,"grant response failure must rollback store");
    ck(runtime::queued_event_count_for_test()==0,"grant response failure must rollback event");

    ck(init_grant(),"reset grant B");
    IO grant_event_fail{};grant_event_fail.quota=grant_frames[0].size()+1;
    protocol::ExchangeState efs{};auto gpart=tx(grant_event_fail,efs,GRANT);
    ck(gpart.status!=protocol::ExchangeStatus::ok&&gpart.response_sent,"grant Event failure status");
    ck(progress()==10,"B response commit required on Event failure");
    ck(runtime::queued_event_count_for_test()==1,"completed Grant candidate retained");
    IO retry{};protocol::ExchangeState retry_s{};auto retry_tx=tx(retry,retry_s,QUERY);
    ck(retry_tx.status==protocol::ExchangeStatus::ok,"grant event retry transaction");
    auto retry_f=frames(retry.wire);ck(retry_f.size()==2,"grant retry must be event frame");
    ck(plain(retry_f[1],retry_s.session_key).find("<Achievement")!=std::string::npos,"grant retry completion event");
    ck(progress()==10,"retry must not duplicate grant state");
    ck(runtime::queued_event_count_for_test()==0,"grant retry consume");
    clear();
    std::cout<<"HF10_B_REAL_SOURCE_ASSERTIONS_PASS "<<assertions
             <<" native_game=NOT_RUN app_ACK=UNPROVEN two_frames=PASS grant_response_commit=PASS event_retry=PASS\n";
}
}
int main(){try{run();return 0;}catch(const std::exception& e){std::cerr<<"HF10_B_SOURCE_FAIL "<<e.what()<<"\n";return 1;}}
