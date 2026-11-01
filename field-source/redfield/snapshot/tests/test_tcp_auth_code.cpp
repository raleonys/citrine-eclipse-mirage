// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/local_service.hpp"
#include "../third_party/tinyxml2/tinyxml2.h"
#include <asio/ip/tcp.hpp>
#include <asio/write.hpp>
#include <fstream>
#include <iostream>
using namespace preloader_l;using asio::ip::tcp;using namespace std::chrono_literals;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
struct Server {
    std::filesystem::path dir=std::filesystem::temp_directory_path()/("rf13_auth_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));LocalService service{dir/"preloader_l.dll"};asio::io_context io;tcp::socket client{io};std::string received;
    Server(){if(!std::filesystem::create_directory(dir))throw std::runtime_error("fixture");}
    bool start(const char* configuration){if(configuration){std::ofstream f(dir/"surge.cfg",std::ios::binary);f<<configuration;f.close();if(!f)return false;}if(!service.start())return false;client.connect({asio::ip::address_v4::loopback(),service.snapshot().port});client.non_blocking(true);return true;}
    template<class P>bool until(P predicate){const auto end=std::chrono::steady_clock::now()+3s;while(!predicate() && std::chrono::steady_clock::now()<end){service.poll_for(1ms);char b[4096];asio::error_code e;const auto n=client.read_some(asio::buffer(b),e);received.append(b,n);if(e && e!=asio::error::would_block && e!=asio::error::try_again && e!=asio::error::eof && e!=asio::error::connection_reset)throw asio::system_error(e);}return predicate();}
    ~Server(){for(unsigned i=0;i<3000 && !service.stop_for(1ms);++i){}std::error_code e;std::filesystem::remove_all(dir,e);}
};
std::string request(const std::string& id,const std::string& command){return "<LSX><Request id='"+id+"'>"+command+"</Request></LSX>";}
std::string attr(const tinyxml2::XMLElement* e,const char* key){const auto* v=e->Attribute(key);return v?v:"!MISSING";}
bool auth8(const std::string& token){return token.size()==8 && token.find_first_not_of("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz")==std::string::npos;}
std::string auth_value(const std::string& wire){tinyxml2::XMLDocument d;if(d.Parse(wire.data(),wire.size())!=tinyxml2::XML_SUCCESS)return {};const auto* r=d.RootElement()->FirstChildElement("Response");const auto* a=r?r->FirstChildElement("AuthCode"):nullptr;return a?attr(a,"value"):"";}
int main(){
    struct Case{const char* config;bool online;};const Case cases[]={
        {nullptr,false},{R"("Config2" {})",false},{R"("Config2" {"Emulator" {"Online" "1"}})",false},{R"("Config2" {"Emulator" {"Online" "false"}})",false},{R"("Config2" {"Emulator" {"Online" "true"}})",true},{R"("Config2" {"Emulator" {"Online" "true" "Online" "0"}})",true}};
    for(const auto& c:cases){
        Server s;CHECK(s.start(c.config));const auto data=request("missing","<GetAuthCode/>")+request("first","<GetAuthCode ClientId=''/>")+request("go","<GoOnline/>")+request("A&amp;lt;","<GetAuthCode ClientId='other'>ignored<Other/></GetAuthCode>")+request("connected","<GetInternetConnectedState/>");
        asio::write(s.client,asio::buffer(data.data(),13));s.service.poll_for(1ms);asio::write(s.client,asio::buffer(data.data()+13,data.size()-13));asio::error_code ec;s.client.shutdown(tcp::socket::shutdown_send,ec);
        CHECK(s.until([&]{const auto p=s.service.snapshot();return p.completed==1 && s.received.size()==p.bytes_written;}));CHECK(s.service.snapshot().frames==5 && s.service.snapshot().responses==4 && s.service.snapshot().malformed==0);
        std::size_t offset=0;std::string first;
        for(unsigned i=0;i<4;++i){const auto end=s.received.find("</LSX>",offset);CHECK(end!=std::string::npos);tinyxml2::XMLDocument d;CHECK(d.Parse(s.received.data()+offset,end+6-offset)==tinyxml2::XML_SUCCESS);const auto* response=d.RootElement()->FirstChildElement("Response");CHECK(response);const auto* body=response->FirstChildElement();CHECK(body);
            if(i==0 || i==2){CHECK(attr(response,"id")== (i==0?"first":"A&lt;"));if(c.online){const auto value=attr(body,"value");CHECK(std::string(body->Name())=="AuthCode" && auth8(value));if(i==0)first=value;else CHECK(value==first);}else CHECK(std::string(body->Name())=="ErrorSuccess" && attr(body,"Description")=="You need to be online to get an AuthCode." && attr(body,"Code")=="-1577058293");}
            else if(i==1)CHECK(attr(response,"id")=="go" && attr(body,"Code")=="0");
            else CHECK(attr(response,"id")=="connected" && attr(body,"connected")== (c.online?"true":"false"));
            offset=end+6;
        }
        CHECK(s.received.find_first_not_of(" \r\n\t",offset)==std::string::npos);
        // The same owner survives a peer half-close; a second peer sees its token.
        const auto previous_bytes=s.service.snapshot().bytes_written;s.client.close();s.received.clear();s.client.connect({asio::ip::address_v4::loopback(),s.service.snapshot().port});s.client.non_blocking(true);const auto second=request("again","<GetAuthCode ClientId='new-peer'/>");asio::write(s.client,asio::buffer(second));s.client.shutdown(tcp::socket::shutdown_send,ec);
        CHECK(s.until([&]{const auto p=s.service.snapshot();return p.completed==2 && s.received.size()==p.bytes_written-previous_bytes;}));
        CHECK(s.until([&]{return s.received.find("</LSX>")!=std::string::npos;}));if(c.online)CHECK(auth_value(s.received)==first);
        const auto held=s.service.configuration();CHECK(held && held->online==c.online);CHECK(s.until([&]{return s.service.stop_for(1ms);}));CHECK(!s.service.start() && held->online==c.online);
    }
    for(const char* config:{R"("Config2" {"Emulator" {"Online" "false"}})",R"("Config2" {"Emulator" {"Online" "true"}})"}){
        {Server s;CHECK(s.start(config));const std::string bad="<LSX><Request><GetAuthCode ClientId=''/></Request></LSX>";asio::write(s.client,asio::buffer(bad));CHECK(s.until([&]{return s.service.snapshot().completed==1;}));CHECK(s.received.empty() && s.service.snapshot().malformed==1 && s.service.snapshot().responses==0);}
        {Server s;CHECK(s.start(config));std::string id;for(unsigned i=0;i<2400;++i)id+="&x;";const auto bad=request(id,"<GetAuthCode ClientId=''/>");CHECK(bad.size()<8192);asio::write(s.client,asio::buffer(bad));CHECK(s.until([&]{return s.service.snapshot().completed==1;}));CHECK(s.received.empty() && s.service.snapshot().malformed==1 && s.service.snapshot().responses==0);}
    }
    std::cout<<"TCP_AUTH8_SESSION_PEERS_FIFO_OFFLINE_REQUIRED_CLIENT_CAPACITY_STOP_PASS\n";
}
