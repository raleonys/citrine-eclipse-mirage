#include "preloader_l/protocol/socket_exchange.hpp"
#include "preloader_l/codec/session_crypto.hpp"
#include "preloader_l/codec/message_codec.hpp"
#include <cstring>

namespace {
struct FakeTransport {
    char frames[4][4096]{};
    size_t sizes[4]{};
    unsigned receive_index=0;
    char sent[16384]{};
    size_t sent_size=0;
    unsigned send_calls=0;
};

preloader_l::net::TransportStatus recv(void* c,uint8_t* out,size_t cap,size_t* n) noexcept {
    auto* f=static_cast<FakeTransport*>(c);
    if(n)*n=0;
    if(f->receive_index>=4) return preloader_l::net::TransportStatus::closed;
    const size_t len=f->sizes[f->receive_index];
    if(len>cap) return preloader_l::net::TransportStatus::io_error;
    std::memcpy(out,f->frames[f->receive_index],len);
    ++f->receive_index;
    if(n)*n=len;
    return preloader_l::net::TransportStatus::ok;
}
preloader_l::net::TransportStatus send(void* c,const uint8_t* data,size_t n,size_t* sent) noexcept {
    auto* f=static_cast<FakeTransport*>(c);
    if(f->sent_size+n>sizeof(f->sent)) return preloader_l::net::TransportStatus::io_error;
    std::memcpy(f->sent+f->sent_size,data,n);
    f->sent_size+=n;
    ++f->send_calls;
    *sent=n;
    return preloader_l::net::TransportStatus::ok;
}
void close(void*) noexcept {}
}

int main()
{
    FakeTransport f{};
    const char handshake[]="<LSX><Request id=\"182\"><ChallengeResponse response=\"x\" key=\"a\" version=\"3\"/></Request></LSX>";
    std::memcpy(f.frames[0],handshake,sizeof(handshake)-1);
    f.sizes[0]=sizeof(handshake)-1;

    uint8_t key[16]{}; char challenge[128]{}; size_t challenge_n=0;
    if(preloader_l::codec::make_challenge_reply(reinterpret_cast<const uint8_t*>("a"),1,"3",1,key,challenge,sizeof(challenge),&challenge_n)!=preloader_l::codec::SessionStatus::ok) return 1;

    // F1427-001: a syntactically valid, encrypted true-unknown command receives
    // no normal response and must not terminate the persistent session.
    const char unknown[]="<LSX><Request id=\"182u\"><NoSuchCommand/></Request></LSX>";
    size_t unknown_wire_n=0;
    if(preloader_l::codec::encode_response(key,16,reinterpret_cast<const uint8_t*>(unknown),sizeof(unknown)-1,f.frames[1],sizeof(f.frames[1]),&unknown_wire_n)!=preloader_l::codec::MessageCodecStatus::ok) return 2;
    f.sizes[1]=unknown_wire_n;

    // HOTFIX02: QueryChunkStatus is no longer accidentally silent because it is
    // unknown. It is explicitly recognized as no-response-required, and the
    // same persistent session must continue to the following request.
    const char chunk[]="<LSX><Request id=\"182c\"><QueryChunkStatus ItemId=\"GAME\"/></Request></LSX>";
    size_t chunk_wire_n=0;
    if(preloader_l::codec::encode_response(key,16,reinterpret_cast<const uint8_t*>(chunk),sizeof(chunk)-1,f.frames[2],sizeof(f.frames[2]),&chunk_wire_n)!=preloader_l::codec::MessageCodecStatus::ok) return 3;
    f.sizes[2]=chunk_wire_n;

    const char request[]="<LSX><Request id=\"182b\"><GetConfig/></Request></LSX>";
    size_t wire_n=0;
    if(preloader_l::codec::encode_response(key,16,reinterpret_cast<const uint8_t*>(request),sizeof(request)-1,f.frames[3],sizeof(f.frames[3]),&wire_n)!=preloader_l::codec::MessageCodecStatus::ok) return 4;
    f.sizes[3]=wire_n;

    preloader_l::net::TransportEndpoint ep{&f,recv,send,close};
    preloader_l::protocol::ExchangeState state{};
    size_t sent=0; uint32_t frames=0;
    const auto status=preloader_l::protocol::serve_protocol_session(ep,&state,&sent,&frames);
    if(status!=preloader_l::protocol::ExchangeStatus::peer_closed) return 5;
    if(frames!=1 || sent==0 || f.receive_index!=4) return 6;
    // One send for the initial greeting, one for ChallengeAccepted, and one for
    // the later valid GetConfig. Neither the unknown frame nor the explicitly
    // recognized QueryChunkStatus no-response frame adds a send.
    if(f.send_calls!=3) return 7;
    return 0;
}
