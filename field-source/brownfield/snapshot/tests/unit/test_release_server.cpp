#include "preloader_l/runtime/bootstrap_state.hpp"
#include "preloader_l/runtime/bootstrap_runner.hpp"
#include "preloader_l/runtime/release_server.hpp"
#include "preloader_l/runtime/runtime_state.hpp"
#include "preloader_l/codec/session_crypto.hpp"
#include "preloader_l/codec/message_codec.hpp"
#include <cassert>
#include <cstring>

namespace {
bool contains_bytes(const char* data,size_t n,const char* needle){size_t m=std::strlen(needle);if(m>n)return false;for(size_t i=0;i+m<=n;++i)if(std::memcmp(data+i,needle,m)==0)return true;return false;}
struct FakeSocket {
    int startup_calls;
    int cleanup_calls;
    int bind_calls;
    int close_calls;
    int last_backlog;
    unsigned receive_call;
    char handshake[512]; size_t handshake_n;
    char wire[2048]; size_t wire_n;
    char sent[8192]; size_t sent_size;
};
int startup(void* c,uint16_t) noexcept { ++static_cast<FakeSocket*>(c)->startup_calls; return 0; }
int cleanup(void* c) noexcept { ++static_cast<FakeSocket*>(c)->cleanup_calls; return 0; }
preloader_l::net::SocketHandle open_tcp(void*) noexcept { static uintptr_t next=10; return ++next; }
int bind_loopback(void* c, preloader_l::net::SocketHandle, uint16_t requested, uint16_t* bound) noexcept {
    auto* f=static_cast<FakeSocket*>(c); ++f->bind_calls;
    if(requested==3216 || requested==3226) return -1;
    *bound=requested; return 0;
}
int listen_fn(void* c,preloader_l::net::SocketHandle,int backlog) noexcept { static_cast<FakeSocket*>(c)->last_backlog=backlog; return 0; }
preloader_l::net::SocketHandle accept_fn(void*,preloader_l::net::SocketHandle) noexcept { return 99; }
preloader_l::net::TransportStatus receive_fn(void* c,preloader_l::net::SocketHandle,uint8_t* out,size_t cap,size_t* received) noexcept {
    auto* f=static_cast<FakeSocket*>(c); if(received)*received=0;
    const char* src=nullptr; size_t len=0;
    if(f->receive_call==0){src=f->handshake;len=f->handshake_n;}
    else if(f->receive_call==1){src=f->wire;len=f->wire_n;}
    else return preloader_l::net::TransportStatus::closed;
    ++f->receive_call; if(cap<len)return preloader_l::net::TransportStatus::io_error;
    std::memcpy(out,src,len);*received=len;return preloader_l::net::TransportStatus::ok;
}
preloader_l::net::TransportStatus send_fn(void* c,preloader_l::net::SocketHandle,const uint8_t* in,size_t len,size_t* sent) noexcept {
    auto* f=static_cast<FakeSocket*>(c); if(f->sent_size+len>sizeof(f->sent)) return preloader_l::net::TransportStatus::io_error;
    std::memcpy(f->sent+f->sent_size,in,len); f->sent_size+=len; *sent=len; return preloader_l::net::TransportStatus::ok;
}
int close_fn(void* c, preloader_l::net::SocketHandle) noexcept { ++static_cast<FakeSocket*>(c)->close_calls;  return 0;}
int last_error(void*) noexcept { return 10048; }
}

int main()
{
    using namespace preloader_l;
    FakeSocket fake{};
    const char hs[]="<LSX><Request id=\"53\"><ChallengeResponse response=\"x\" key=\"a\" version=\"3\"/></Request></LSX>";
    std::memcpy(fake.handshake,hs,sizeof(hs)-1);fake.handshake_n=sizeof(hs)-1;
    uint8_t key[16]{};char reply[128]{};size_t reply_n=0;
    assert(codec::make_challenge_reply(reinterpret_cast<const uint8_t*>("a"),1,"3",1,key,reply,sizeof(reply),&reply_n)==codec::SessionStatus::ok);
    const char request[]="<LSX><Request id=\"53b\"><GetConfig/></Request></LSX>";size_t wire_n=0;
    assert(codec::encode_response(key,16,reinterpret_cast<const uint8_t*>(request),sizeof(request)-1,fake.wire,sizeof(fake.wire),&wire_n)==codec::MessageCodecStatus::ok);
    fake.wire_n=wire_n;

    net::SocketApi api{&fake,startup,cleanup,open_tcp,bind_loopback,listen_fn,accept_fn,receive_fn,send_fn,close_fn,last_error};
    runtime::ReleaseServerContext server{};
    runtime::reset_release_server(&server,&api);
    runtime::reset_state_for_test();
    runtime::on_process_attach(reinterpret_cast<void*>(0x180000000ull));
    runtime::reset_bootstrap_for_test();
    assert(runtime::mark_bootstrap_scheduled());
    const auto result=runtime::run_one_shot_bootstrap(runtime::make_release_server_backend(&server));
    assert(result==runtime::BootstrapRunResult::ok);
    const auto snapshot=runtime::snapshot_bootstrap();
    assert(snapshot.phase==runtime::BootstrapPhase::completed && snapshot.bound_port==3227);
    assert(fake.startup_calls==1 && fake.cleanup_calls==1 && fake.bind_calls==2);
    assert(fake.last_backlog==0x7fffffff);
    assert(fake.sent_size>1 && fake.sent[0]==' ' && fake.sent[1]=='<');
    assert(contains_bytes(fake.sent,fake.sent_size,"ChallengeAccepted"));
    assert(server.last_written==fake.sent_size);
    assert(runtime::snapshot_state().phase==runtime::Phase::shutdown);
}
