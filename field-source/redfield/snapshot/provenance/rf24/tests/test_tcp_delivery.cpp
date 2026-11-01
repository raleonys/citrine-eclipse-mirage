// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/tcp_ingress.hpp"
#include <asio/ip/tcp.hpp>
#include <asio/write.hpp>
#include <cstring>
#include <iostream>
#include <string>
using namespace redfield;using asio::ip::tcp;using namespace std::chrono_literals;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
constexpr auto frame="<LSX><Request id='x'><Ping/></Request></LSX>";
struct State {unsigned commits=0,drops=0,calls=0;bool allow=true;std::string response=std::string(9000,'R'),event="EVENT";};
struct Delivery final:FrameDelivery {
    State& state;std::string reply,event;
    explicit Delivery(State& s):state(s),reply(s.response),event(s.event){}
    ~Delivery(){++state.drops;}
    std::string_view response() const noexcept override{return reply;}
    std::string_view completion_event() const noexcept override{return event;}
    bool commit() noexcept override{++state.commits;return state.allow;}
};
bool prepare(void* p,std::string_view,char* out,std::size_t,std::size_t* n,std::unique_ptr<FrameDelivery>& delivery) noexcept {
    try{auto& s=*static_cast<State*>(p);if(s.calls++==0)delivery=std::make_unique<Delivery>(s);else{std::memcpy(out,"NEXT",4);*n=4;}return true;}catch(...){return false;}
}
struct Fixture {
    State state;TcpIngress server;asio::io_context io;tcp::socket client{io};std::string received;
    bool start(){if(server.start({&state,nullptr,nullptr,prepare})!=TcpStartResult::started)return false;client.connect({asio::ip::address_v4::loopback(),server.snapshot().port});client.non_blocking(true);return true;}
    void collect(){char b[4096];asio::error_code e;const auto n=client.read_some(asio::buffer(b),e);received.append(b,n);}
    template<class P>bool until(P p){const auto end=std::chrono::steady_clock::now()+3s;while(!p() && std::chrono::steady_clock::now()<end){server.poll_for(0ms);collect();}return p();}
    bool stop(){for(unsigned i=0;i<3000;++i)if(server.stop_for(1ms))return true;return false;}
    ~Fixture(){stop();}
};
int main(){
    {Fixture f;CHECK(f.start());const std::string two=std::string(frame)+frame;asio::write(f.client,asio::buffer(two));asio::error_code e;f.client.shutdown(tcp::socket::shutdown_send,e);
     CHECK(f.until([&]{return f.server.snapshot().bytes_written==4096;}));CHECK(f.state.commits==0 && f.state.drops==0 && f.server.snapshot().responses==0);
     CHECK(f.until([&]{return f.server.snapshot().completed==1 && f.received.size()==9009;}));CHECK(f.received==f.state.response+f.state.event+"NEXT");
     CHECK(f.state.commits==1 && f.state.drops==1 && f.state.calls==2 && f.server.snapshot().events==1 && f.server.snapshot().responses==2 && f.server.snapshot().committed_deliveries==1);}
    {Fixture f;CHECK(f.start());asio::write(f.client,asio::buffer(frame,std::strlen(frame)));CHECK(f.until([&]{return f.server.snapshot().bytes_written==4096;}));CHECK(f.state.drops==0 && f.state.commits==0);CHECK(f.stop());CHECK(f.state.commits==0 && f.state.drops==1 && f.server.snapshot().responses==0 && f.server.snapshot().events==0);}
    {Fixture f;f.state.response="RESPONSE";f.state.allow=false;CHECK(f.start());asio::write(f.client,asio::buffer(frame,std::strlen(frame)));CHECK(f.until([&]{return f.server.snapshot().completed==1;}));CHECK(f.state.commits==1 && f.state.drops==1 && f.received=="RESPONSE" && f.server.snapshot().events==0 && f.server.snapshot().failures==1 && f.server.snapshot().committed_deliveries==0);}
    {Fixture f;f.state.response="RESPONSE";CHECK(f.start());asio::write(f.client,asio::buffer(frame,std::strlen(frame)));CHECK(f.until([&]{return f.state.commits==1;}));CHECK(f.state.drops==0 && f.server.snapshot().events==0);CHECK(f.stop());CHECK(f.state.commits==1 && f.state.drops==1 && f.server.snapshot().committed_deliveries==1 && f.server.snapshot().events==0);}
    for(unsigned mode=0;mode<3;++mode){Fixture f;if(mode==0)f.state.response.clear();if(mode==1)f.state.event.assign(TcpIngress::response_capacity,'E');if(mode==2)f.state.response.assign(TcpIngress::response_capacity,'R');CHECK(f.start());asio::write(f.client,asio::buffer(frame,std::strlen(frame)));CHECK(f.until([&]{return f.server.snapshot().completed==1;}));CHECK(f.received.empty() && f.state.commits==0 && f.state.drops==1 && f.server.snapshot().malformed==1);}
    std::cout<<"TCP_OWNED_DELIVERY_FULL_WRITE_COMMIT_EVENT_FIFO_CANCEL_ADMISSION_PASS\n";
}
