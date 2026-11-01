#include "preloader_l/protocol/socket_exchange.hpp"
#include "preloader_l/codec/session_crypto.hpp"
#include "preloader_l/codec/message_codec.hpp"
#include <cstring>
namespace {
struct F { char in[3][4096]{}; size_t n[3]{}; unsigned call=0; char out[20000]{}; size_t o=0; };
preloader_l::net::TransportStatus rx(void*c,uint8_t*b,size_t cap,size_t*n) noexcept {
 auto*f=(F*)c; if(f->call>=3){*n=0;return preloader_l::net::TransportStatus::closed;}
 size_t len=f->n[f->call]; if(len>cap)len=cap; std::memcpy(b,f->in[f->call],len); ++f->call; *n=len; return preloader_l::net::TransportStatus::ok;
}
preloader_l::net::TransportStatus tx(void*c,const uint8_t*b,size_t n,size_t*s) noexcept {
 auto*f=(F*)c; if(f->o+n>sizeof(f->out))return preloader_l::net::TransportStatus::io_error; std::memcpy(f->out+f->o,b,n); f->o+=n; *s=n; return preloader_l::net::TransportStatus::ok;
}
void cls(void*) noexcept {}
}
int main(){
 F f{};
 const char hs[]="<LSX><Request id=\"173\"><ChallengeResponse response=\"x\" key=\"a\" version=\"3\"/></Request></LSX>";
 std::memcpy(f.in[0],hs,sizeof(hs)-1); f.n[0]=sizeof(hs)-1;
 uint8_t key[16]{}; char reply[128]{}; size_t rn=0;
 if(preloader_l::codec::make_challenge_reply((const uint8_t*)"a",1,"3",1,key,reply,sizeof(reply),&rn)!=preloader_l::codec::SessionStatus::ok)return 1;
 const char q1[]="<LSX><Request id=\"a\"><GetConfig/></Request></LSX>";
 const char q2[]="<LSX><Request id=\"b\"><GetConfig/></Request></LSX>";
 size_t w1=0,w2=0;
 if(preloader_l::codec::encode_response(key,16,(const uint8_t*)q1,sizeof(q1)-1,f.in[1],sizeof(f.in[1]),&w1)!=preloader_l::codec::MessageCodecStatus::ok)return 2;
 if(preloader_l::codec::encode_response(key,16,(const uint8_t*)q2,sizeof(q2)-1,f.in[2],sizeof(f.in[2]),&w2)!=preloader_l::codec::MessageCodecStatus::ok)return 3;
 f.n[1]=w1; f.n[2]=w2;
 preloader_l::net::TransportEndpoint ep{&f,rx,tx,cls}; preloader_l::protocol::ExchangeState st{}; size_t sent=0; uint32_t frames=0;
 auto r=preloader_l::protocol::serve_protocol_session(ep,&st,&sent,&frames);
 if(r!=preloader_l::protocol::ExchangeStatus::peer_closed)return 4;
 if(frames!=2 || sent==0 || f.call!=3)return 5;
 return 0;
}
