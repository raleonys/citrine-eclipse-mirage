#include "preloader_l/protocol/socket_exchange.hpp"
#include "preloader_l/codec/session_crypto.hpp"
#include "preloader_l/codec/message_codec.hpp"
#include <cstring>
namespace {
bool contains_bytes(const char* data,size_t n,const char* needle){size_t m=std::strlen(needle);if(m>n)return false;for(size_t i=0;i+m<=n;++i)if(std::memcmp(data+i,needle,m)==0)return true;return false;}
struct F {
    char first[4096]{}; size_t first_n=0;
    char second[4096]{}; size_t second_n=0;
    unsigned call=0;
    char out[12000]{}; size_t o=0;
};
preloader_l::net::TransportStatus recv(void*c,uint8_t*b,size_t cap,size_t*n) noexcept {
    auto*f=(F*)c; const char*src=nullptr; size_t len=0;
    if(f->call==0){src=f->first;len=f->first_n;} else if(f->call==1){src=f->second;len=f->second_n;} else {*n=0;return preloader_l::net::TransportStatus::closed;}
    ++f->call; if(len>cap)len=cap; std::memcpy(b,src,len); *n=len; return preloader_l::net::TransportStatus::ok;
}
preloader_l::net::TransportStatus send(void*c,const uint8_t*b,size_t n,size_t*s) noexcept {
    auto*f=(F*)c;if(f->o+n>sizeof(f->out))return preloader_l::net::TransportStatus::io_error;
    std::memcpy(f->out+f->o,b,n);f->o+=n;*s=n;return preloader_l::net::TransportStatus::ok;
}
void close(void*) noexcept{}
}
int main(){
 F f{}; const char hs[]="<LSX><Request id=\"61\"><ChallengeResponse response=\"x\" key=\"a\" version=\"3\"/></Request></LSX>";
 std::memcpy(f.first,hs,sizeof(hs)-1);f.first_n=sizeof(hs)-1;
 uint8_t key[16]{};char reply[128]{};size_t rn=0;if(preloader_l::codec::make_challenge_reply((const uint8_t*)"a",1,"3",1,key,reply,sizeof(reply),&rn)!=preloader_l::codec::SessionStatus::ok)return 1;
 const char q[]="<LSX><Request id=\"x\"><GetConfig/></Request></LSX>";char wire[1024]{};size_t wn=0;
 if(preloader_l::codec::encode_response(key,16,(const uint8_t*)q,sizeof(q)-1,wire,sizeof(wire),&wn)!=preloader_l::codec::MessageCodecStatus::ok)return 2;
 std::memcpy(f.second,wire,wn);f.second_n=wn;
 preloader_l::net::TransportEndpoint ep{&f,recv,send,close};preloader_l::protocol::ExchangeState st{};preloader_l::protocol::reset_exchange(&st);size_t sent=0;
 if(preloader_l::protocol::serve_one_protocol_exchange(ep,&st,&sent)!=preloader_l::protocol::ExchangeStatus::ok||!st.established||sent==0)return 3;
 if (!contains_bytes(f.out,f.o,"<Challenge key=\"")) return 4;
 if (!contains_bytes(f.out,f.o,"ChallengeAccepted")) return 5;
 return 0;
}
