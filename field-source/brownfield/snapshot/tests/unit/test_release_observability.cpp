#ifndef PRELOADER_L_TRACE_DIAGNOSTICS
#define PRELOADER_L_TRACE_DIAGNOSTICS 1
#endif
#include "preloader_l/runtime/bootstrap_state.hpp"
#include "preloader_l/runtime/bootstrap_runner.hpp"
#include "preloader_l/runtime/release_server.hpp"
#include "preloader_l/runtime/runtime_state.hpp"
#include "preloader_l/runtime/event_trace.hpp"
#include "preloader_l/codec/session_crypto.hpp"
#include "preloader_l/codec/message_codec.hpp"
#include <cstring>
namespace {
struct Fake { unsigned read=0; char hs[512]{}; size_t hs_n=0; char wire[2048]{}; size_t wire_n=0; char sent[8192]{}; size_t sent_n=0; unsigned events=0; };
int startup(void*,uint16_t) noexcept{return 0;} int cleanup(void*) noexcept{return 0;}
preloader_l::net::SocketHandle open(void*) noexcept{return 2;}
int bind(void*,preloader_l::net::SocketHandle,uint16_t p,uint16_t* out) noexcept{*out=p;return 0;}
int listen(void*,preloader_l::net::SocketHandle,int) noexcept{return 0;}
preloader_l::net::SocketHandle accept(void*,preloader_l::net::SocketHandle) noexcept{return 3;}
preloader_l::net::TransportStatus recv(void* c,preloader_l::net::SocketHandle,uint8_t* o,size_t cap,size_t* n) noexcept{
    auto*f=(Fake*)c;*n=0;const char*src=nullptr;size_t len=0;
    if(f->read==0){src=f->hs;len=f->hs_n;} else if(f->read==1){src=f->wire;len=f->wire_n;} else return preloader_l::net::TransportStatus::closed;
    ++f->read;if(cap<len)return preloader_l::net::TransportStatus::io_error;std::memcpy(o,src,len);*n=len;return preloader_l::net::TransportStatus::ok;
}
preloader_l::net::TransportStatus send(void*c,preloader_l::net::SocketHandle,const uint8_t*i,size_t l,size_t*n) noexcept{auto*f=(Fake*)c;if(f->sent_n+l>sizeof(f->sent))return preloader_l::net::TransportStatus::io_error;std::memcpy(f->sent+f->sent_n,i,l);f->sent_n+=l;*n=l;return preloader_l::net::TransportStatus::ok;}
int close(void*, preloader_l::net::SocketHandle) noexcept { return 0;} int last(void*) noexcept{return 0;}
Fake* g=nullptr; void trace(preloader_l::runtime::TraceEvent e,uint32_t) noexcept{ if(g && e!=preloader_l::runtime::TraceEvent::none) ++g->events; }
}
int main(){
    Fake f{};g=&f;
    const char hs[]="<LSX><Request id=\"59\"><ChallengeResponse response=\"x\" key=\"a\" version=\"3\"/></Request></LSX>";std::memcpy(f.hs,hs,sizeof(hs)-1);f.hs_n=sizeof(hs)-1;
    uint8_t key[16]{};char reply[128]{};size_t rn=0;if(preloader_l::codec::make_challenge_reply((const uint8_t*)"a",1,"3",1,key,reply,sizeof(reply),&rn)!=preloader_l::codec::SessionStatus::ok)return 10;
    const char q[]="<LSX><Request id=\"59b\"><GetConfig/></Request></LSX>";size_t wn=0;if(preloader_l::codec::encode_response(key,16,(const uint8_t*)q,sizeof(q)-1,f.wire,sizeof(f.wire),&wn)!=preloader_l::codec::MessageCodecStatus::ok)return 11;f.wire_n=wn;
    preloader_l::net::SocketApi api{&f,startup,cleanup,open,bind,listen,accept,recv,send,close,last};
    preloader_l::runtime::ReleaseServerContext c{}; preloader_l::runtime::reset_release_server(&c,&api); c.trace=&trace;
    preloader_l::runtime::reset_state_for_test(); preloader_l::runtime::on_process_attach((void*)0x180000000ull);
    preloader_l::runtime::reset_bootstrap_for_test(); preloader_l::runtime::reset_trace_for_test(); if(!preloader_l::runtime::mark_bootstrap_scheduled()) return 1;
    if(preloader_l::runtime::run_one_shot_bootstrap(preloader_l::runtime::make_release_server_backend(&c))!=preloader_l::runtime::BootstrapRunResult::ok)return 2;
    if(f.events<3 || f.sent_n==0) return 3;
    preloader_l::runtime::TraceRecord rec[64]{}; size_t n=preloader_l::runtime::snapshot_trace(rec,64); bool listener_seen=false,response_seen=false,session_closed_seen=false;
    for(size_t i=0;i<n;++i){ if(rec[i].event==preloader_l::runtime::TraceEvent::listener_active) listener_seen=true; if(rec[i].event==preloader_l::runtime::TraceEvent::response_sent) response_seen=true; if(rec[i].event==preloader_l::runtime::TraceEvent::persistent_session_closed) session_closed_seen=true; }
    return listener_seen&&response_seen&&session_closed_seen?0:4;
}
