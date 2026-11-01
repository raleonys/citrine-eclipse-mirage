// HF12 Stage02A: native Windows Asio IOCP-owned challenge/response and
// encrypted LSX ingress. Actual shipping handshake, TinyXML2, wire scanner,
// AES/hex, all linked directly. The Config2 SDK and local social host binding
// are deliberately isolated test stubs, NOT shipping host/dispatcher acceptance.
#include <asio.hpp>
#if !defined(_WIN32) || !defined(ASIO_HAS_IOCP) || defined(ASIO_DISABLE_IOCP)
#error HF12 Stage02 native requires real Windows IOCP, never reactor fallback.
#endif
#if !defined(_WIN64)
#error HF12 native acceptance requires MSVC x64 (the shipping target); x86 results are not authoritative.
#endif
#include "socket_exchange_internal.hpp"
#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/codec/message_codec.hpp"
#include <array>
#include <chrono>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

namespace p = preloader_l::protocol;
namespace w = preloader_l::protocol::detail;
using asio::ip::tcp;
using namespace std::chrono_literals;

// Test-only platform profile binding: no mock of handshake/XML/codec/protocol.
namespace preloader_l { namespace config {
ValueView runtime_configuration_setting(const char*) noexcept {
    return ValueView{nullptr,0u,false};
}
} namespace runtime {
bool bind_host_social_profile(LocalSocialSession* out) noexcept { return out!=nullptr; }
} }

static unsigned checks=0u;
static void check(bool b,const char* message) {
    ++checks;
    if(!b) throw std::runtime_error(message);
}

struct Session final : std::enable_shared_from_this<Session> {
    tcp::socket socket;
    p::ExchangeState state{};
    std::array<char,256> greeting{};
    size_t greeting_n=0;
    uint32_t malformed=0u;
    bool ready=false;
    bool saw_xml=false;
    bool failure=false;
    bool pending_handshake=false;
    w::WireFrameView pending_frame{};
    p::HandshakeResult pending{};
    explicit Session(tcp::socket accepted):socket(std::move(accepted)){}
    void start() {
        if(p::build_challenge_greeting(1700000000u,greeting.data(),greeting.size(),&greeting_n)
            !=p::GreetingStatus::ok) {failure=true;return;}
        auto self=shared_from_this();
        asio::async_write(socket,asio::buffer(greeting.data(),greeting_n+1u),
            [self](const asio::error_code& ec,size_t written){
                if(ec || written!=self->greeting_n+1u){self->failure=true;return;}
                self->process_or_read();
            });
    }
    void process_or_read() {
        w::WireFrameView frame{};
        while(w::scan_available_wire_frame(&state,&frame)) {
            auto step=w::classify_received_wire_frame(&state,frame,&malformed);
            if(step.kind==w::WireStepKind::discarded)continue;
            if(step.kind==w::WireStepKind::terminal){failure=true;return;}
            if(step.kind==w::WireStepKind::handshake_reply){
                if(pending_handshake){failure=true;return;}
                pending_handshake=true;
                pending_frame=frame;
                pending=step.handshake;
                // Required failure-atomic invariant: no key before full reply send.
                if(state.established){failure=true;return;}
                const size_t size=pending.response_length+1u;
                auto self=shared_from_this();
                asio::async_write(socket,asio::buffer(state.plain_response,size),
                    [self,size](const asio::error_code& ec,size_t written){
                        if(ec || written!=size){self->failure=true;return;}
                        if(!w::commit_sent_handshake(&self->state,self->pending_frame,
                                                     self->pending,&self->malformed)){
                            self->failure=true;return;
                        }
                        self->pending_handshake=false;
                        self->ready=true;
                        self->process_or_read();
                    });
                return; // Never start a concurrent read/write against pending state.
            }
            if(step.kind!=w::WireStepKind::xml_request){failure=true;return;}
            const std::string expected="<LSX><Request id=\"8\"><GetSetting/></Request></LSX>";
            if(step.xml_length!=expected.size() ||
               std::memcmp(state.decoded,expected.data(),expected.size())!=0){failure=true;return;}
            w::consume_wire_frame(&state,frame.consume_length);
            saw_xml=true;
            return;
        }
        if(state.wire_size>=sizeof(state.wire_response)){failure=true;return;}
        auto self=shared_from_this();
        socket.async_read_some(asio::buffer(state.wire_response+state.wire_size,
                                            sizeof(state.wire_response)-state.wire_size),
            [self](const asio::error_code& ec,size_t n){
                if(ec || !n || n>sizeof(self->state.wire_response)-self->state.wire_size){
                    self->failure=true;return;
                }
                self->state.wire_size+=n;
                self->process_or_read();
            });
    }
};

static void one_round() {
    asio::io_context io;
    tcp::acceptor listener(io,tcp::endpoint(asio::ip::address_v4::loopback(),0));
    std::shared_ptr<Session> server;
    asio::streambuf client_greeting, client_reply;
    tcp::socket client(io);
    bool client_verified=false;
    bool accept_ok=false;
    bool client_error=false;
    const std::string challenge="<LSX><Request id=\"7\"><ChallengeResponse key=\"a\" version=\"3\"/></Request></LSX>";
    listener.async_accept([&](const asio::error_code& ec,tcp::socket sock) {
        accept_ok=!ec;
        if(ec){client_error=true;return;}
        server=std::make_shared<Session>(std::move(sock));
        server->start();
    });
    asio::error_code ec;
    client.connect(listener.local_endpoint(),ec);
    check(!ec,"native loopback connect");
    asio::async_read_until(client,client_greeting,'\0',
        [&](const asio::error_code& read_ec,size_t){
            if(read_ec){client_error=true;return;}
            std::string challenge_frame=challenge+std::string(1,'\0');
            auto send_buf=std::make_shared<std::string>(std::move(challenge_frame));
            asio::async_write(client,asio::buffer(*send_buf),
                [&,send_buf](const asio::error_code& write_ec,size_t n){
                    if(write_ec || n!=send_buf->size()){client_error=true;return;}
                    asio::async_read_until(client,client_reply,'\0',
                        [&](const asio::error_code& reply_ec,size_t){
                            if(reply_ec){client_error=true;return;}
                            std::string raw(asio::buffers_begin(client_reply.data()),
                                            asio::buffers_end(client_reply.data()));
                            if(raw.find("<ChallengeAccepted response=\"fe123486508b0eb503a8468388ef5812\"/>")
                                ==std::string::npos){client_error=true;return;}
                            client_verified=true;
                            const uint8_t key[16]={0x4e,0x54,0x1f,0x16,0x5f,0x6a,0x47,
                                0xa9,0xc5,0x18,0xd6,0x66,0x8a,0x14,0x45,0x37};
                            const std::string xml="<LSX><Request id=\"8\"><GetSetting/></Request></LSX>";
                            size_t required=0;
                            if(preloader_l::codec::encode_response(key,16,
                                 reinterpret_cast<const uint8_t*>(xml.data()),xml.size(),
                                 nullptr,0,&required)!=preloader_l::codec::MessageCodecStatus::capacity){
                                client_error=true;return;
                            }
                            auto cipher=std::make_shared<std::string>(required,'\0');
                            size_t done=0;
                            if(preloader_l::codec::encode_response(key,16,
                                 reinterpret_cast<const uint8_t*>(xml.data()),xml.size(),
                                 &(*cipher)[0],cipher->size(),&done)!=preloader_l::codec::MessageCodecStatus::ok
                               || done!=required){client_error=true;return;}
                            asio::async_write(client,asio::buffer(*cipher),
                                [&,cipher](const asio::error_code& cipher_ec,size_t n){
                                    if(cipher_ec || n!=cipher->size())client_error=true;
                                });
                        });
                });
        });
    // A fixed native acceptance deadline, never an unbounded blocking wait.
    io.run_for(1000ms);
    check(accept_ok && !client_error,"IOCP accept and client I/O completion");
    check(server!=nullptr && !server->failure,"source-linked handshake server state");
    check(client_verified && server->ready,"actual handshake sent and committed");
    check(server->saw_xml,"real encrypted LSX request decoded by shared step");
    if(server){
        asio::error_code close_ec;
        server->socket.cancel(close_ec);
        check(!close_ec,"Asio-owned cancel");
        server->socket.close(close_ec);
        check(!close_ec,"Asio-owned close without release");
    }
    listener.close();
    client.close();
    io.restart();io.poll();
    check(server.use_count()==1u,"completion owners drained");
}
int main(){
    try {
        for(unsigned round=0u;round<8u;++round)one_round();
        std::cout<<"HF12_STAGE02_REAL_WINDOWS_IOCP_HANDSHAKE PASS rounds=8 checks="<<checks<<"\n";
    } catch(const std::exception& e) {
        std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<"\n";
        return 1;
    }
}
