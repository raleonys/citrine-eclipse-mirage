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
    std::filesystem::path dir=std::filesystem::temp_directory_path()/("rf11_control_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));LocalService service{dir/"preloader_l.dll"};asio::io_context io;tcp::socket client{io};std::string received;
    Server(){if(!std::filesystem::create_directory(dir))throw std::runtime_error("fixture");}
    bool start(const char* configuration){if(configuration){std::ofstream f(dir/"surge.cfg",std::ios::binary);f<<configuration;f.close();if(!f)return false;}if(!service.start())return false;client.connect({asio::ip::address_v4::loopback(),service.snapshot().port});client.non_blocking(true);return true;}
    template<class P>bool until(P predicate){const auto end=std::chrono::steady_clock::now()+3s;while(!predicate() && std::chrono::steady_clock::now()<end){service.poll_for(1ms);char b[4096];asio::error_code e;const auto n=client.read_some(asio::buffer(b),e);received.append(b,n);if(e && e!=asio::error::would_block && e!=asio::error::try_again && e!=asio::error::eof && e!=asio::error::connection_reset)throw asio::system_error(e);}return predicate();}
    ~Server(){for(unsigned i=0;i<3000 && !service.stop_for(1ms);++i){}std::error_code e;std::filesystem::remove_all(dir,e);}
};
std::string request(const char* id,const std::string& command){return std::string("<LSX><Request id='")+id+"'>"+command+"</Request></LSX>";}
std::string attr(const tinyxml2::XMLElement* e,const char* key){const auto* v=e->Attribute(key);return v?v:"!MISSING";}
int main(){
    struct Case{const char* name;const char* code;const char* description;};const Case cases[]={
        {"GoOnline","0",""},{"EnableVoip","-1593835518","The function you are calling is not implemented."},{"SetDownloaderUtilization","0",""},{"SetPresenceVisibility","0",""},{"SetPresence","0",""},{"StartDownload","0","Requested a download of a DLC offer."},{"RestartGame","1","Origin is waiting for the game to shutdown."}};
    for(const char* config:{static_cast<const char*>(nullptr),R"("Config2" {"Emulator" {"Online" "false"}})",R"("Config2" {"Emulator" {"Online" "true"}})"}){
        Server s;CHECK(s.start(config));const bool online=config && std::string(config).find("true")!=std::string::npos;
        auto data=request("skip","<ShowIGO/>")+request("chunk","<QueryChunkStatus/>")+request("before","<GetInternetConnectedState/>");
        for(const auto& c:cases)data+=request(c.name,std::string("<")+c.name+" Online='true' Visible='true'><Ignored>not-a-mutation</Ignored></"+c.name+">");
        data+=request("after","<GetInternetConnectedState/>")+request("settings","<GetSettings/>")+request("visibility","<GetPresenceVisibility/>")+request("profile","<GetProfile index='0'/>");
        asio::write(s.client,asio::buffer(data.data(),13));s.service.poll_for(1ms);asio::write(s.client,asio::buffer(data.data()+13,data.size()-13));asio::error_code ec;s.client.shutdown(tcp::socket::shutdown_send,ec);
        CHECK(s.until([&]{const auto p=s.service.snapshot();return p.completed==1 && s.received.size()==p.bytes_written;}));CHECK(s.service.snapshot().frames==14 && s.service.snapshot().responses==12 && s.service.snapshot().malformed==0);
        std::size_t offset=0;
        for(unsigned i=0;i<12;++i){const auto end=s.received.find("</LSX>",offset);CHECK(end!=std::string::npos);tinyxml2::XMLDocument d;CHECK(d.Parse(s.received.data()+offset,end+6-offset)==tinyxml2::XML_SUCCESS);const auto* response=d.RootElement()->FirstChildElement("Response");CHECK(response);const auto* body=response->FirstChildElement();CHECK(body);
            if(i==0 || i==8){CHECK(attr(response,"id")== (i==0?"before":"after") && std::string(body->Name())=="InternetConnectedState" && attr(body,"connected")== (online?"true":"false"));}
            else if(i<8){const auto& c=cases[i-1];CHECK(attr(response,"id")==c.name && std::string(body->Name())=="ErrorSuccess" && attr(body,"Code")==c.code && attr(body,"Description")==c.description);}
            else if(i==9)CHECK(attr(response,"id")=="settings" && std::string(body->Name())=="GetSettingsResponse" && attr(body,"IsManualOffline")== (online?"false":"true"));
            else if(i==10)CHECK(attr(response,"id")=="visibility" && std::string(body->Name())=="GetPresenceVisibilityResponse" && attr(body,"Visible")=="false");
            else CHECK(attr(response,"id")=="profile" && std::string(body->Name())=="GetProfileResponse");
            offset=end+6;
        }
        CHECK(s.received.find_first_not_of(" \r\n\t",offset)==std::string::npos);CHECK(s.until([&]{return s.service.stop_for(1ms);}));
    }
    for(const auto& c:cases){Server s;CHECK(s.start(nullptr));const auto bad=std::string("<LSX><Request><")+c.name+"/></Request></LSX>";asio::write(s.client,asio::buffer(bad));CHECK(s.until([&]{return s.service.snapshot().completed==1;}));CHECK(s.received.empty() && s.service.snapshot().malformed==1 && s.service.snapshot().responses==0);}
    {Server s;CHECK(s.start(nullptr));std::string id;for(unsigned i=0;i<2400;++i)id+="&x;";const auto bad=request(id.c_str(),"<GoOnline/>");CHECK(bad.size()<8192);asio::write(s.client,asio::buffer(bad));CHECK(s.until([&]{return s.service.snapshot().completed==1;}));CHECK(s.received.empty() && s.service.snapshot().malformed==1 && s.service.snapshot().responses==0);}
    std::cout<<"TCP_CONTROL_ACK_FIFO_NO_STATE_TRANSITION_CAPACITY_ATOMIC_PASS\n";
}
