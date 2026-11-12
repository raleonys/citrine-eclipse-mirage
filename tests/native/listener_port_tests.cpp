// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/net/asio_service.hpp"
#include "citrine_eclipse_mirage/net/listener_policy.hpp"
#include "citrine_eclipse_mirage/platform/execution_windows.hpp"
#include <asio.hpp>
#include <array>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace citrine_eclipse_mirage;
namespace {
using Tcp=asio::ip::tcp;
int checks=0;
void require(bool value,const char* label){++checks;if(!value)throw std::runtime_error(label);}
std::string published(){char bytes[64]{};const auto size=GetEnvironmentVariableA("EALsxPort",bytes,sizeof bytes);return size?std::string(bytes,size):std::string{};}
struct Environment {
    std::string previous=published();
    Environment(){require(SetEnvironmentVariableA("EALsxPort","listener-test-sentinel")!=FALSE,"set publication sentinel");}
    ~Environment(){SetEnvironmentVariableA("EALsxPort",previous.empty()?nullptr:previous.c_str());}
};
struct Factory final:net::HandlerFactory {
    int failures=0;
    struct Handler final:server::Handler {
        void reset_02bc70()override{}
        std::string initial_message_02d540()override{return "hello";}
        std::string process_message_033d70(std::string_view)override{return "reply";}
    };
    std::unique_ptr<server::Handler> create()override{return std::make_unique<Handler>();}
    void error(std::string_view,std::exception_ptr)noexcept override{++failures;}
};
struct Running {
    Factory factory;
    execution::MutexState mutex{};
    std::uint8_t ready=0,second=0,third=0;
    std::uint16_t port;
    execution::WindowsPlatform platform{+[](int){throw std::runtime_error("test mutex error");}};
    std::int64_t frequency=platform.query_performance_frequency();
    execution::Execution execution{mutex,{ready,second,third},frequency,platform};
    net::AsioService service{factory,execution,port,ready};
    explicit Running(std::uint16_t stale_port):port(stale_port){execution::initialize_mutex_04a698(mutex,2);}
    void start(){service.start();}
    void stop(){service.stop();}
};
bool ipv6_available(){
    asio::io_context io;Tcp::acceptor probe(io);asio::error_code error;
    probe.open(Tcp::v6(),error);
    if(!error)probe.bind({asio::ip::address_v6::loopback(),0},error);
    if(error==asio::error::address_family_not_supported||
       (error.category()==asio::error::get_system_category()&&error.value()==WSAEADDRNOTAVAIL))return false;
    if(error)throw asio::system_error(error,"IPv6 test capability probe");
    return true;
}
void greeting(std::uint16_t port,bool ipv6=false){
    asio::io_context io;Tcp::socket peer(io);
    const asio::ip::address address=ipv6?asio::ip::address(asio::ip::address_v6::loopback()):asio::ip::address(asio::ip::address_v4::loopback());
    peer.connect({address,port});
    std::array<char,5> bytes{};asio::read(peer,asio::buffer(bytes));
    require(std::string(bytes.data(),bytes.size())=="hello","existing listener survives other candidates' failed binds/listens");
}
}
int main()try{
    Environment environment;
    const bool ipv6=ipv6_available();
    asio::io_context io;
    std::array<std::unique_ptr<Tcp::acceptor>,net::listener_policy::last_port-net::listener_policy::first_port+1> occupied;
    for(std::size_t i=0;i<occupied.size();++i){
        auto socket=std::make_unique<Tcp::acceptor>(io);socket->open(Tcp::v4());
        const BOOL exclusive=TRUE;
        require(::setsockopt(socket->native_handle(),SOL_SOCKET,SO_EXCLUSIVEADDRUSE,
            reinterpret_cast<const char*>(&exclusive),sizeof exclusive)==0,"reserve exclusive test port");
        socket->bind({asio::ip::address_v4::loopback(),static_cast<std::uint16_t>(net::listener_policy::first_port+i)});
        socket->listen();occupied[i]=std::move(socket);
    }
    {Running full(45000);bool failed=false;
     try{full.start();}catch(const asio::system_error& e){failed=std::string(e.what()).find("3226-3236")!=std::string::npos;}
     require(failed,"all eleven occupied ports fail explicitly without arbitrary-port fallback");
     require(full.ready==0&&full.port==45000,"failed bind does not publish readiness or a fabricated live port");
     require(published()=="listener-test-sentinel","failed bind preserves environment publication");
     require(full.factory.failures==1,"startup failure reaches error boundary");full.stop();full.stop();}
    occupied.front().reset();
    Running first(65535);first.start();
    require(first.port==3226&&first.ready==1,"lowest candidate selected even with stale arbitrary input");
    require(published()=="3226","actual first bound port published");
    {
        Tcp::acceptor rival(io);rival.open(Tcp::v4());
        rival.set_option(asio::socket_base::reuse_address(true));
        asio::error_code error;
        rival.bind({asio::ip::address_v4::loopback(),first.port},error);
        if(!error)rival.listen(asio::socket_base::max_listen_connections,error);
        require(error==asio::error::address_in_use||error==asio::error::access_denied,
            "reuse-enabled rival cannot share a live service endpoint");
    }
    occupied.back().reset();
    Running last(0);last.start();
    require(last.port==3236&&last.ready==1,"busy candidates skipped through inclusive last port");
    require(published()=="3236","actual last bound port published");
    greeting(first.port);greeting(last.port);
    if(ipv6){greeting(first.port,true);greeting(last.port,true);}
    else std::cout<<"SKIP: IPv6-only assertions; host loopback family unavailable\n";
    first.stop();first.stop();
    Running system_services(80);system_services.start();
    require(system_services.port==3226,"released lowest port reusable after failed candidates and complete shutdown");
    system_services.stop();last.stop();
    if(ipv6){
        Tcp::acceptor blocker(io,Tcp::v6());blocker.set_option(asio::ip::v6_only(true));
        const BOOL exclusive=TRUE;
        require(::setsockopt(blocker.native_handle(),SOL_SOCKET,SO_EXCLUSIVEADDRUSE,
            reinterpret_cast<const char*>(&exclusive),sizeof exclusive)==0,"exclusive IPv6 candidate blocker");
        blocker.bind({asio::ip::address_v6::loopback(),3226});blocker.listen();
        Running next(0);next.start();
        require(next.port==3236,"IPv6 conflict skips candidate although its IPv4 endpoint is free");
        Tcp::acceptor released(io,{asio::ip::address_v4::loopback(),3226});
        require(released.is_open(),"failed companion bind releases the candidate IPv4 endpoint");
        greeting(next.port,true);next.stop();
    }
    std::cout<<"PASS: "<<checks<<" bounded listener checks; first/last, occupied range, no fallback, publication and recovery\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
