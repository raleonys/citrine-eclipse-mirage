#include "preloader_l/protocol/socket_exchange.hpp"
#include <cstring>
namespace {
struct F { const char* chunks[3]{}; size_t sizes[3]{}; unsigned call=0; char out[2048]{}; size_t o=0; };
preloader_l::net::TransportStatus recv(void*c,uint8_t*b,size_t cap,size_t*n) noexcept {
 auto*f=(F*)c; if(f->call>=3||!f->chunks[f->call]){*n=0;return preloader_l::net::TransportStatus::closed;}
 size_t len=f->sizes[f->call]; const char*src=f->chunks[f->call++]; if(len>cap)len=cap; std::memcpy(b,src,len);*n=len;return preloader_l::net::TransportStatus::ok;
}
preloader_l::net::TransportStatus send(void*c,const uint8_t*b,size_t n,size_t*s) noexcept {auto*f=(F*)c;if(f->o+n>sizeof(f->out))return preloader_l::net::TransportStatus::io_error;std::memcpy(f->out+f->o,b,n);f->o+=n;*s=n;return preloader_l::net::TransportStatus::ok;}
void close(void*) noexcept{}
}
int main(){
 const char a[]="<LSX><Request id=\"x\"><ChallengeResponse ";
 const char b[]="response=\"x\" key=\"a\" version=\"3\"/></Request></LSX>";
 F f{{a,b,nullptr},{sizeof(a)-1,sizeof(b)-1,0},0,{0},0};
 preloader_l::net::TransportEndpoint ep{&f,recv,send,close}; preloader_l::protocol::ExchangeState st{}; preloader_l::protocol::reset_exchange(&st); size_t sent=0;
 // HF18/F1485-001: the fragmented handshake is reconstructed. The one-shot
 // helper then reaches peer close while waiting for the encrypted request.
 const auto status=preloader_l::protocol::serve_one_protocol_exchange(ep,&st,&sent);
 if(status!=preloader_l::protocol::ExchangeStatus::peer_closed || !st.established) return 1;
 if(!std::strstr(f.out,"<Challenge key=\"")) return 2;
 if(!std::strstr(f.out,"ChallengeAccepted")) return 3;
 return 0;
}
