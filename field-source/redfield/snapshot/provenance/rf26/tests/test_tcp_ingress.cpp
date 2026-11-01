// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/local_service.hpp"
#include <asio/ip/tcp.hpp>
#include <asio/write.hpp>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
using namespace preloader_l;using asio::ip::tcp;using namespace std::chrono_literals;
#define CHECK(x) do{if(!(x)){std::cerr<<"line "<<__LINE__<<": " #x "\n";return 1;}}while(false)
struct Probe{
    std::vector<xml::Request> requests;
    static bool frame(void* context,std::string_view source) noexcept{
        auto& p=*static_cast<Probe*>(context);try{
            xml::Request request;if(xml::parse(source,request)!=xml::Result::ok)return false;
            p.requests.push_back(std::move(request));return true;
        }catch(...){return false;}
    }
};
struct Fixture {
    TcpIngress ingress;Probe probe;asio::io_context clients;
    bool start(){return ingress.start({&probe,Probe::frame})==TcpStartResult::started;}
    tcp::socket connect(){tcp::socket s(clients);s.connect({asio::ip::address_v4::loopback(),ingress.snapshot().port});return s;}
    template<class P>bool until(P predicate){const auto end=std::chrono::steady_clock::now()+3s;while(!predicate() && std::chrono::steady_clock::now()<end)ingress.poll_for(1ms);return predicate();}
    ~Fixture(){for(int i=0;i<3000 && !ingress.stop_for(1ms);++i){} }
};
struct TempDirectory {
    std::filesystem::path path=std::filesystem::temp_directory_path()/("preloader_l_rf02_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    TempDirectory(){if(!std::filesystem::create_directory(path))throw std::runtime_error("fixture already exists");}
    ~TempDirectory(){std::error_code error;std::filesystem::remove_all(path,error);}
};
int main(){
    {
        Fixture f;CHECK(f.start());const auto port=f.ingress.snapshot().port;CHECK(port);
        auto socket=f.connect();CHECK(f.until([&]{return f.ingress.snapshot().accepted==1;}));
        const std::string frame="<LSX><Request id='A&amp;lt;'><A><Value><![CDATA[</LSX>]]></Value></A></Request></LSX>";
        for(const char value:frame){asio::write(socket,asio::buffer(&value,1));f.ingress.poll_for(1ms);}
        CHECK(f.until([&]{return f.probe.requests.size()==1;}));
        const auto both=frame+frame;asio::write(socket,asio::buffer(both));
        CHECK(f.until([&]{return f.probe.requests.size()==3;}));
        CHECK(preloader_l::xml::span_equals(f.probe.requests[0].id,"A&lt;"));
        asio::error_code ec;socket.shutdown(tcp::socket::shutdown_send,ec);
        CHECK(f.until([&]{return f.ingress.snapshot().completed==1;}));
        CHECK(f.ingress.snapshot().frames==3 && f.ingress.snapshot().active==0);
        CHECK(f.until([&]{return f.ingress.stop_for(1ms);}));
        CHECK(f.ingress.quiescent() && f.ingress.snapshot().accepted==f.ingress.snapshot().completed);
        CHECK(f.ingress.stop_for(0ms));
        CHECK(preloader_l::xml::span_equals(f.probe.requests[0].id,"A&lt;"));
        tcp::socket refused(f.clients);refused.connect({asio::ip::address_v4::loopback(),port},ec);CHECK(ec);
    }
    {
        Fixture f;CHECK(f.start());auto socket=f.connect();
        const std::string invalid="<LSX><Request><A/><B/></Request></LSX>";asio::write(socket,asio::buffer(invalid));
        CHECK(f.until([&]{return f.ingress.snapshot().completed==1;}));
        CHECK(f.probe.requests.empty() && f.ingress.snapshot().malformed==1);
        auto next=f.connect();const std::string bad="NO_LSX";asio::write(next,asio::buffer(bad));
        CHECK(f.until([&]{return f.ingress.snapshot().completed==2;}));CHECK(f.ingress.snapshot().malformed==2);
    }
    {
        Fixture f;CHECK(f.start());auto socket=f.connect();const std::string partial="<LSX><Request>";
        asio::write(socket,asio::buffer(partial));asio::error_code ec;socket.shutdown(tcp::socket::shutdown_send,ec);
        CHECK(f.until([&]{return f.ingress.snapshot().completed==1;}));CHECK(f.ingress.snapshot().truncated==1 && f.probe.requests.empty());
        auto huge=f.connect();const std::string over="<LSX>"+std::string(8192-5,' ');asio::write(huge,asio::buffer(over));
        CHECK(f.until([&]{return f.ingress.snapshot().completed==2;}));CHECK(f.ingress.snapshot().capacity==1);
    }
    {
        Fixture f;CHECK(f.start());std::vector<tcp::socket> sockets;
        for(unsigned i=0;i<32;++i)sockets.push_back(f.connect());
        CHECK(f.until([&]{return f.ingress.snapshot().accepted==32;}));
        const std::string partial="<LSX>";for(auto& s:sockets)asio::write(s,asio::buffer(partial));
        CHECK(f.until([&]{return f.ingress.stop_for(1ms);}));
        const auto snapshot=f.ingress.snapshot();CHECK(snapshot.active==0 && snapshot.completed==32 && snapshot.frames==0);
    }
    {
        Fixture f;CHECK(f.start());Fixture occupied;
        CHECK(occupied.ingress.start({&occupied.probe,Probe::frame},f.ingress.snapshot().port)==TcpStartResult::failed);
        CHECK(occupied.ingress.stop_for(0ms));
    }
    {
        TempDirectory directory;LocalService service(directory.path/"preloader_l.dll");
        CHECK(service.start());const auto port=service.snapshot().port;CHECK(port && !service.configuration()->online);
        asio::io_context io;tcp::socket socket(io);socket.connect({asio::ip::address_v4::loopback(),port});
        const std::string source="<LSX><Request><GetSetting/></Request></LSX>";asio::write(socket,asio::buffer(source));
        const auto end=std::chrono::steady_clock::now()+3s;
        while(service.snapshot().frames==0 && std::chrono::steady_clock::now()<end)service.poll_for(1ms);
        CHECK(service.snapshot().frames==1);
        for(unsigned i=0;i<3000 && !service.stop_for(1ms);++i){}
        CHECK(service.quiescent() && service.snapshot().active==0 && !service.configuration());
    }
    std::cout<<"TCP_INGRESS_REAL_LOOPBACK_PASS\n";
}
