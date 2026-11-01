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
    std::filesystem::path dir=std::filesystem::temp_directory_path()/("rf12_permission_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));LocalService service{dir/"preloader_l.dll"};asio::io_context io;tcp::socket client{io};std::string received;
    Server(){if(!std::filesystem::create_directory(dir))throw std::runtime_error("fixture");}
    bool start(const char* configuration){if(configuration){std::ofstream f(dir/"preloader_l.cfg",std::ios::binary);f<<configuration;f.close();if(!f)return false;}if(!service.start())return false;client.connect({asio::ip::address_v4::loopback(),service.snapshot().port});client.non_blocking(true);return true;}
    template<class P>bool until(P predicate){const auto end=std::chrono::steady_clock::now()+3s;while(!predicate() && std::chrono::steady_clock::now()<end){service.poll_for(1ms);char b[4096];asio::error_code e;const auto n=client.read_some(asio::buffer(b),e);received.append(b,n);if(e && e!=asio::error::would_block && e!=asio::error::try_again && e!=asio::error::eof && e!=asio::error::connection_reset)throw asio::system_error(e);}return predicate();}
    ~Server(){for(unsigned i=0;i<3000 && !service.stop_for(1ms);++i){}std::error_code e;std::filesystem::remove_all(dir,e);}
};
std::string request(const char* id,const std::string& command){return std::string("<LSX><Request id='")+id+"'>"+command+"</Request></LSX>";}
std::string attr(const tinyxml2::XMLElement* e,const char* key){const auto* v=e->Attribute(key);return v?v:"!MISSING";}
int main(){
    struct Case{const char* config;bool online;};const Case cases[]={
        {nullptr,false},{R"("Config2" {})",false},{R"("Config2" {"Emulator" {"Online" "1"}})",false},{R"("Config2" {"Emulator" {"Online" "false"}})",false},{R"("Config2" {"Emulator" {"Online" "true"}})",true},{R"("Config2" {"Emulator" {"Online" "true" "Online" "0"}})",true}};
    for(const auto& c:cases){
        Server s;CHECK(s.start(c.config));const auto data=request("unbound","<GetAuthCode/>")+request("chunk","<QueryChunkStatus/>")+request("before","<CheckPermission/>")+request("go","<GoOnline/>")+request("after","<CheckPermission/>")+request("connected","<GetInternetConnectedState/>")+request("settings","<GetSettings/>")+request("P&amp;lt;","<CheckPermission PermissionId='unknown'>ignored<Other/></CheckPermission>")+request("profile","<GetProfile index='0'/>");
        asio::write(s.client,asio::buffer(data.data(),13));s.service.poll_for(1ms);asio::write(s.client,asio::buffer(data.data()+13,data.size()-13));asio::error_code ec;s.client.shutdown(tcp::socket::shutdown_send,ec);
        CHECK(s.until([&]{const auto p=s.service.snapshot();return p.completed==1 && s.received.size()==p.bytes_written;}));CHECK(s.service.snapshot().frames==9 && s.service.snapshot().responses==7 && s.service.snapshot().malformed==0);
        std::size_t offset=0;
        for(unsigned i=0;i<7;++i){const auto end=s.received.find("</LSX>",offset);CHECK(end!=std::string::npos);tinyxml2::XMLDocument d;CHECK(d.Parse(s.received.data()+offset,end+6-offset)==tinyxml2::XML_SUCCESS);const auto* response=d.RootElement()->FirstChildElement("Response");CHECK(response);const auto* body=response->FirstChildElement();CHECK(body);
            if(i==0 || i==2 || i==5){CHECK(attr(response,"id")== (i==0?"before":i==2?"after":"P&lt;"));if(c.online)CHECK(std::string(body->Name())=="CheckPermissionResponse" && attr(body,"Access")=="GRANTED");else CHECK(std::string(body->Name())=="ErrorSuccess" && attr(body,"Description")=="" && attr(body,"Code")=="-1576468080");}
            else if(i==1)CHECK(attr(response,"id")=="go" && std::string(body->Name())=="ErrorSuccess" && attr(body,"Code")=="0");
            else if(i==3)CHECK(attr(response,"id")=="connected" && std::string(body->Name())=="InternetConnectedState" && attr(body,"connected")== (c.online?"true":"false"));
            else if(i==4)CHECK(attr(response,"id")=="settings" && std::string(body->Name())=="GetSettingsResponse" && attr(body,"IsManualOffline")== (c.online?"false":"true"));
            else CHECK(attr(response,"id")=="profile" && std::string(body->Name())=="GetProfileResponse");
            offset=end+6;
        }
        CHECK(s.received.find_first_not_of(" \r\n\t",offset)==std::string::npos);CHECK(s.until([&]{return s.service.stop_for(1ms);}));
    }
    for(const char* config:{R"("Config2" {"Emulator" {"Online" "false"}})",R"("Config2" {"Emulator" {"Online" "true"}})"}){
        {Server s;CHECK(s.start(config));const std::string bad="<LSX><Request><CheckPermission/></Request></LSX>";asio::write(s.client,asio::buffer(bad));CHECK(s.until([&]{return s.service.snapshot().completed==1;}));CHECK(s.received.empty() && s.service.snapshot().malformed==1 && s.service.snapshot().responses==0);}
        {Server s;CHECK(s.start(config));std::string id;for(unsigned i=0;i<2400;++i)id+="&x;";const auto bad=request(id.c_str(),"<CheckPermission/>");CHECK(bad.size()<8192);asio::write(s.client,asio::buffer(bad));CHECK(s.until([&]{return s.service.snapshot().completed==1;}));CHECK(s.received.empty() && s.service.snapshot().malformed==1 && s.service.snapshot().responses==0);}
    }
    std::cout<<"TCP_PERMISSION_ONLINE_OFFLINE_NUMERIC_GOONLINE_FIFO_CAPACITY_PASS\n";
}
