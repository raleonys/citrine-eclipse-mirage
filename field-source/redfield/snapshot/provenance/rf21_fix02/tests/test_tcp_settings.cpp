// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/local_service.hpp"
#include "../third_party/tinyxml2/tinyxml2.h"
#include <asio/ip/tcp.hpp>
#include <asio/write.hpp>
#include <fstream>
#include <iostream>
using namespace redfield;using asio::ip::tcp;using namespace std::chrono_literals;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
struct Server {
    std::filesystem::path dir=std::filesystem::temp_directory_path()/("rf04_settings_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    LocalService service{dir/"preloader_l.dll"};asio::io_context io;tcp::socket client{io};std::string received;
    Server(){if(!std::filesystem::create_directory(dir))throw std::runtime_error("temp dir");}
    bool start(const char* config){if(config){std::ofstream f(dir/"redfield.cfg",std::ios::binary);f<<config;f.close();if(!f)return false;}if(!service.start())return false;client.connect({asio::ip::address_v4::loopback(),service.snapshot().port});client.non_blocking(true);return true;}
    template<class P>bool until(P predicate){const auto end=std::chrono::steady_clock::now()+3s;while(!predicate() && std::chrono::steady_clock::now()<end){service.poll_for(1ms);char b[4096];asio::error_code e;const auto n=client.read_some(asio::buffer(b),e);received.append(b,n);if(e && e!=asio::error::eof && e!=asio::error::would_block && e!=asio::error::try_again)throw asio::system_error(e);}return predicate();}
    ~Server(){for(unsigned i=0;i<3000 && !service.stop_for(1ms);++i){}std::error_code e;std::filesystem::remove_all(dir,e);}
};
std::string request(const char* id,const char* command){return std::string("<LSX><Request id='")+id+"' recipient='SDK'>"+command+"</Request></LSX>";}
int main(){
    const char* configs[]={nullptr,"\"Config2\" {\"Emulator\" {\"Online\" \"1\"}}","\"Config2\" {\"Game\" {\"Language\" \"tr_TR\"} \"Emulator\" {\"Online\" \"true\" \"Environment\" \"stage&dev\"}}"};
    for(unsigned i=0;i<3;++i){Server s;CHECK(s.start(configs[i]));
        const auto input=request("settings","<GetSettings/>")+request("ignored","<GetSetting SettingId='UNKNOWN'/>")+request("language","<GetSetting SettingId='LANG&#85;AGE'/>")+request("offline","<GetSetting SettingId='IS_MANUAL_OFFLINE'/>")+request("connected","<GetInternetConnectedState/>");
        asio::write(s.client,asio::buffer(input.data(),9));s.service.poll_for(1ms);asio::write(s.client,asio::buffer(input.data()+9,input.size()-9));asio::error_code ec;s.client.shutdown(tcp::socket::shutdown_send,ec);
        CHECK(s.until([&]{return s.service.snapshot().completed==1 && s.received.size()==s.service.snapshot().bytes_written;}));
        const auto snapshot=s.service.snapshot();CHECK(snapshot.frames==5 && snapshot.responses==4 && snapshot.bytes_written==s.received.size() && s.received.find('\0')==std::string::npos);
        std::size_t offset=0;const char* ids[]={"settings","language","offline","connected"};
        for(unsigned j=0;j<4;++j){const auto end=s.received.find("</LSX>",offset);CHECK(end!=std::string::npos);tinyxml2::XMLDocument doc;CHECK(doc.Parse(s.received.data()+offset,end+6-offset)==tinyxml2::XML_SUCCESS);const auto* r=doc.RootElement()->FirstChildElement("Response");CHECK(r && std::string_view(r->Attribute("id"))==ids[j] && std::string_view(r->Attribute("sender"))=="SDK");const auto* b=r->FirstChildElement();CHECK(b);
            if(j==0)CHECK(std::string_view(b->Attribute("IsManualOffline"))==(i==2?"false":"true") && std::string_view(b->Attribute("Language"))==(i==2?"tr_TR":"en_US") && std::string_view(b->Attribute("Environment"))==(i==2?"stage&dev":"production"));
            if(j==1)CHECK(std::string_view(b->Attribute("Setting"))==(i==2?"tr_TR":"en_US"));
            if(j==2)CHECK(std::string_view(b->Attribute("Setting"))==(i==2?"false":"true"));
            if(j==3)CHECK(std::string_view(b->Attribute("connected"))==(i==2?"true":"false"));
            offset=end+6;
        }
        CHECK(s.until([&]{return s.service.stop_for(1ms);}));CHECK(s.service.quiescent());
    }
    std::cout<<"TCP_SETTINGS_SNAPSHOT_OFFLINE_ONLINE_PASS\n";
}
