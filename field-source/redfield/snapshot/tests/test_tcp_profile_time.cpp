// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/local_service.hpp"
#include "utc_test_support.hpp"
#include "../third_party/tinyxml2/tinyxml2.h"
#include <asio/ip/tcp.hpp>
#include <asio/write.hpp>
#include <fstream>
#include <iostream>
using namespace preloader_l;using asio::ip::tcp;using namespace std::chrono_literals;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
struct Server {
    std::filesystem::path dir=std::filesystem::temp_directory_path()/("rf06_profile_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    LocalService service{dir/"preloader_l.dll"};asio::io_context io;tcp::socket client{io};std::string received;
    Server(){if(!std::filesystem::create_directory(dir))throw std::runtime_error("fixture");}
    bool start(const char* configuration){if(configuration){std::ofstream f(dir/"surge.cfg",std::ios::binary);f<<configuration;f.close();if(!f)return false;}if(!service.start())return false;client.connect({asio::ip::address_v4::loopback(),service.snapshot().port});client.non_blocking(true);return true;}
    template<class P>bool until(P predicate){const auto end=std::chrono::steady_clock::now()+3s;while(!predicate() && std::chrono::steady_clock::now()<end){service.poll_for(1ms);char b[4096];asio::error_code e;const auto n=client.read_some(asio::buffer(b),e);received.append(b,n);if(e && e!=asio::error::would_block && e!=asio::error::try_again && e!=asio::error::eof && e!=asio::error::connection_reset)throw asio::system_error(e);}return predicate();}
    ~Server(){for(unsigned i=0;i<3000 && !service.stop_for(1ms);++i){}std::error_code e;std::filesystem::remove_all(dir,e);}
};
std::string request(const char* id,const char* command){return std::string("<LSX><Request id='")+id+"'>"+command+"</Request></LSX>";}
int main(){
    for(const char* config:{static_cast<const char*>(nullptr),R"("Config2" {})",R"("Config2" {"User" {"Username" "Sky & <King>" "Country" "TR"} "Game" {"Name" "Neon"}})"}){
        Server s;CHECK(s.start(config));
        const std::string username=!config?"SURGE":std::string(config).find("Sky")!=std::string::npos?"Sky & <King>":"";
        const std::string title=username=="Sky & <King>"?"Neon":"Game";
        const auto start=utc_wall_seconds();
        // An unsupported selector produces no response and does not consume a
        // previous response buffer. The remaining coalesced requests stay FIFO.
        const auto data=request("skip","<GetProfile><index>0</index></GetProfile>")+request("profile&amp;lt;","<GetProfile index='&#50;'/>")+request("presence","<GetPresence/>")+request("time","<GetUTCTime/>")+request("chunk","<QueryChunkStatus/>")+request("next","<GetPresence/>");
        asio::write(s.client,asio::buffer(data.data(),13));s.service.poll_for(1ms);asio::write(s.client,asio::buffer(data.data()+13,data.size()-13));asio::error_code ec;s.client.shutdown(tcp::socket::shutdown_send,ec);
        CHECK(s.until([&]{const auto p=s.service.snapshot();return p.completed==1 && s.received.size()==p.bytes_written;}));
        CHECK(s.service.snapshot().frames==6 && s.service.snapshot().responses==4 && s.service.snapshot().malformed==0);
        CHECK(s.received.find('\0')==std::string::npos);
        std::size_t offset=0;const char* ids[]={"profile&lt;","presence","time","next"};
        for(unsigned i=0;i<4;++i){const auto end=s.received.find("</LSX>",offset);CHECK(end!=std::string::npos);const auto length=end+6-offset;tinyxml2::XMLDocument doc;CHECK(doc.Parse(s.received.data()+offset,length)==tinyxml2::XML_SUCCESS);const auto* response=doc.RootElement()->FirstChildElement("Response");CHECK(response && std::string(response->Attribute("id"))==ids[i]);const auto* body=response->FirstChildElement();CHECK(body);
            if(i==0){CHECK(std::string(body->Name())=="GetProfileResponse" && std::string(body->Attribute("Persona"))==username && std::string(body->Attribute("UserIndex"))=="2");}
            else if(i==2){CHECK(std::string(body->Name())=="GetUTCTimeResponse");const std::string time=body->Attribute("Time");const auto after=utc_wall_seconds();if(time<utc_text(start) || time>utc_text(after))std::cerr<<"clock bounds: "<<utc_text(start)<<" <= "<<time<<" <= "<<utc_text(after)<<"\n";CHECK(time>=utc_text(start) && time<=utc_text(after));}
            else CHECK(std::string(body->Name())=="GetPresenceResponse" && std::string(body->Attribute("RichPresence"))==title && std::string(body->Attribute("UserId"))=="0");
            offset=end+6;
        }
        CHECK(s.received.find_first_not_of(" \r\n\t",offset)==std::string::npos);CHECK(s.until([&]{return s.service.stop_for(1ms);}));
    }
    {Server s;CHECK(s.start(nullptr));const std::string bad="<LSX><Request><GetUTCTime/></Request></LSX>";asio::write(s.client,asio::buffer(bad));CHECK(s.until([&]{return s.service.snapshot().completed==1;}));CHECK(s.received.empty() && s.service.snapshot().malformed==1);}
    std::cout<<"TCP_PROFILE_TIME_MISSING_EMPTY_OVERLAY_FIFO_PASS\n";
}
