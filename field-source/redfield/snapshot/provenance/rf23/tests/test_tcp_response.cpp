// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/local_service.hpp"
#include "redfield/response_dispatch.hpp"
#include <asio/ip/tcp.hpp>
#include <asio/write.hpp>
#include <array>
#include <iostream>
using namespace redfield;using asio::ip::tcp;using namespace std::chrono_literals;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
struct Fixture {
    std::filesystem::path dir=std::filesystem::temp_directory_path()/("rf03_response_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::optional<std::string> port=std::string("previous");bool fail_restore=false;
    static bool read(void* p,std::optional<std::string>& value) noexcept {try{value=static_cast<Fixture*>(p)->port;return true;}catch(...){return false;}}
    static bool write(void* p,const char* value) noexcept {auto& f=*static_cast<Fixture*>(p);if(f.fail_restore)return false;try{if(value)f.port=value;else f.port.reset();return true;}catch(...){return false;}}
    LocalService service{dir/"preloader_l.dll",{this,read,write}};
    asio::io_context io;tcp::socket client{io};std::string received;
    Fixture(){if(!std::filesystem::create_directory(dir))throw std::runtime_error("temp dir");}
    bool start(){if(!service.start())return false;client.connect({asio::ip::address_v4::loopback(),service.snapshot().port});client.non_blocking(true);return true;}
    void collect(){char bytes[4096];asio::error_code ec;const auto n=client.read_some(asio::buffer(bytes),ec);received.append(bytes,n);if(ec && ec!=asio::error::would_block && ec!=asio::error::try_again && ec!=asio::error::eof && ec!=asio::error::connection_reset)throw asio::system_error(ec);}
    template<class P>bool until(P predicate){const auto end=std::chrono::steady_clock::now()+3s;while(!predicate() && std::chrono::steady_clock::now()<end){service.poll_for(1ms);collect();}return predicate();}
    ~Fixture(){fail_restore=false;for(unsigned i=0;i<3000 && !service.stop_for(1ms);++i){}std::error_code e;std::filesystem::remove_all(dir,e);}
};
std::string expected(const std::string& input){xml::Request request;if(xml::parse(input,request)!=xml::Result::ok)throw std::runtime_error("parse");std::array<char,16384> b;std::size_t n=0;if(dispatch_response(request,b.data(),b.size(),&n)!=ResponseResult::response)throw std::runtime_error("dispatch");return {b.data(),n};}
bool large(void*,std::string_view,char* out,std::size_t cap,std::size_t* n) noexcept {for(std::size_t i=0;i<cap;++i)out[i]='X';*n=cap;return true;}
int main(){
    const std::string config="<LSX><Request id='A&amp;lt;' recipient='SDK'><GetConfig/></Request></LSX>";
    const std::string pi="<LSX><Request id='2'><IsProgressiveInstallationAvailable/></Request></LSX>";
    const std::string unknown="<LSX><Request id='3'><NotImplementedRF03/></Request></LSX>";
    {Fixture f;CHECK(f.start());CHECK(f.port==std::to_string(f.service.snapshot().port));
     asio::write(f.client,asio::buffer(config.data(),7));f.service.poll_for(1ms);
     const auto rest=config.substr(7)+unknown+pi+pi;asio::write(f.client,asio::buffer(rest));asio::error_code ec;f.client.shutdown(tcp::socket::shutdown_send,ec);
     const auto want=expected(config)+expected(pi)+expected(pi);
     CHECK(f.until([&]{return f.service.snapshot().completed==1 && f.received.size()==want.size();}));
     CHECK(f.received==want && f.received.find('\0')==std::string::npos);
     const auto state=f.service.snapshot();CHECK(state.frames==4 && state.responses==3 && state.bytes_written==want.size());
     f.fail_restore=true;CHECK(!f.service.stop_for(0ms) && !f.service.quiescent());f.fail_restore=false;
     CHECK(f.until([&]{return f.service.stop_for(1ms);}));CHECK(f.port=="previous");}
    {Fixture f;f.fail_restore=true;CHECK(!f.service.start() && !f.service.quiescent() && f.port=="previous");f.fail_restore=false;CHECK(f.until([&]{return f.service.stop_for(1ms);}));CHECK(f.service.quiescent());}
    {Fixture f;CHECK(f.start());const auto data=pi+"<LSX><Request><A/><B/></Request></LSX>";asio::write(f.client,asio::buffer(data));
     CHECK(f.until([&]{return f.service.snapshot().completed==1 && f.received.size()==expected(pi).size();}));CHECK(f.received==expected(pi) && f.service.snapshot().malformed==1);}
    {asio::io_context io;TcpIngress ingress;CHECK(ingress.start({nullptr,nullptr,large})==TcpStartResult::started);tcp::socket client(io);client.connect({asio::ip::address_v4::loopback(),ingress.snapshot().port});asio::write(client,asio::buffer(pi));
     const auto end=std::chrono::steady_clock::now()+3s;while(!ingress.snapshot().bytes_written && std::chrono::steady_clock::now()<end)ingress.poll_for(0ms);
     CHECK(ingress.snapshot().bytes_written==4096 && ingress.snapshot().responses==0 && ingress.snapshot().frames==1);
     for(unsigned i=0;i<3000 && !ingress.stop_for(1ms);++i){}CHECK(ingress.quiescent() && ingress.snapshot().completed==1 && ingress.snapshot().responses==0);}
    std::cout<<"TCP_RESPONSE_ORDER_EOF_CANCEL_PASS\n";
}
