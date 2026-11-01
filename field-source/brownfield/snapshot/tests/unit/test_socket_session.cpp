#include "preloader_l/net/socket_session.hpp"
#include <cassert>
#include <cstring>

namespace {
struct FakeEndpoint {
    const char* request;
    size_t request_size;
    bool received;
    char sent[2048];
    size_t sent_size;
    int closes;
};
preloader_l::net::TransportStatus recv_fn(void* c, uint8_t* out, size_t cap, size_t* received) noexcept {
    auto* f=static_cast<FakeEndpoint*>(c); if(received)*received=0;
    if(f->received) return preloader_l::net::TransportStatus::would_block;
    if(cap<f->request_size) return preloader_l::net::TransportStatus::io_error;
    std::memcpy(out,f->request,f->request_size); f->received=true; if(received)*received=f->request_size;
    return preloader_l::net::TransportStatus::ok;
}
preloader_l::net::TransportStatus send_fn(void* c, const uint8_t* in, size_t length, size_t* sent) noexcept {
    auto* f=static_cast<FakeEndpoint*>(c); if(f->sent_size+length>sizeof(f->sent)) return preloader_l::net::TransportStatus::io_error;
    std::memcpy(f->sent+f->sent_size,in,length); f->sent_size+=length; if(sent)*sent=length; return preloader_l::net::TransportStatus::ok;
}
void close_fn(void* c) noexcept { ++static_cast<FakeEndpoint*>(c)->closes; }
}
int main(){ using namespace preloader_l::net;
    static const char request[]="<LSX><Request id=\"49\"><GetConfig/></Request></LSX>";
    FakeEndpoint fake{request,sizeof(request)-1,false,{},0,0};
    TransportEndpoint endpoint{&fake,recv_fn,send_fn,close_fn};
    SocketSessionServer server{}; reset_socket_session_server(&server);
    assert(bind_socket_session_server(&server,49,endpoint)==SessionStatus::ok);
    FakeEndpoint second_fake{request,sizeof(request)-1,false,{},0,0};
    TransportEndpoint second_endpoint{&second_fake,recv_fn,send_fn,close_fn};
    assert(bind_socket_session_server(&server,50,second_endpoint)==SessionStatus::already_connected);
    assert(server.session.session_id==49 && server.endpoint.context==&fake && second_fake.closes==0);
    char response[2048]{}; size_t written=0;
    assert(pump_socket_session_once(&server,response,sizeof(response),&written)==SocketPumpStatus::ok);
    assert(written>0 && fake.sent_size==written && std::memcmp(fake.sent,response,written)==0);
    assert(server.session.receive_events==1 && server.session.send_events==1);
    assert(server.dispatch_count==1 && server.responses_sent==1 && server.ingress.frames_consumed==1);
    assert(close_socket_session_server(&server)==SessionStatus::ok); assert(fake.closes==1);

    // F323-S0470-001: output capacity is recoverable. The complete request
    // remains pending, required-size metadata is not counted as sent bytes,
    // and a larger retry consumes/sends exactly once.
    FakeEndpoint retry_fake{request,sizeof(request)-1,false,{},0,0};
    TransportEndpoint retry_endpoint{&retry_fake,recv_fn,send_fn,close_fn};
    SocketSessionServer retry{}; reset_socket_session_server(&retry);
    assert(bind_socket_session_server(&retry,51,retry_endpoint)==SessionStatus::ok);
    char tiny[8]{}; size_t required=0;
    assert(pump_socket_session_once(&retry,tiny,sizeof(tiny),&required)==SocketPumpStatus::capacity);
    assert(required>=sizeof(tiny));
    assert(retry.ingress.frames_consumed==0 && retry.ingress.receive.size==sizeof(request)-1);
    assert(retry.session.send_events==0 && retry.session.sent_bytes==0);
    assert(retry.dispatch_count==0 && retry.responses_sent==0 && retry_fake.sent_size==0);
    char larger[2048]{}; size_t retried=0;
    assert(pump_socket_session_once(&retry,larger,sizeof(larger),&retried)==SocketPumpStatus::ok);
    assert(retried>0 && retry_fake.sent_size==retried);
    assert(retry.ingress.frames_consumed==1 && retry.ingress.receive.size==0);
    assert(retry.session.send_events==1 && retry.session.sent_bytes==retried);
    assert(retry.dispatch_count==1 && retry.responses_sent==1);
    assert(close_socket_session_server(&retry)==SessionStatus::ok);
}
