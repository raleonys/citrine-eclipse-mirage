// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/local_service.hpp"
#include "utc_test_support.hpp"
#include "../third_party/tinyxml2/tinyxml2.h"
#include <asio/ip/tcp.hpp>
#include <asio/write.hpp>
#include <fstream>
#include <iostream>
using namespace redfield;using asio::ip::tcp;using namespace std::chrono_literals;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
struct Server {
    std::filesystem::path dir=std::filesystem::temp_directory_path()/("rf07_game_info_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    LocalService service{dir/"preloader_l.dll"};asio::io_context io;tcp::socket client{io};std::string received;
    Server(){if(!std::filesystem::create_directory(dir))throw std::runtime_error("fixture");}
    bool start(const char* configuration){if(configuration){std::ofstream f(dir/"redfield.cfg",std::ios::binary);f<<configuration;f.close();if(!f)return false;}if(!service.start())return false;client.connect({asio::ip::address_v4::loopback(),service.snapshot().port});client.non_blocking(true);return true;}
    template<class P>bool until(P predicate){const auto end=std::chrono::steady_clock::now()+3s;while(!predicate() && std::chrono::steady_clock::now()<end){service.poll_for(1ms);char b[4096];asio::error_code e;const auto n=client.read_some(asio::buffer(b),e);received.append(b,n);if(e && e!=asio::error::would_block && e!=asio::error::try_again && e!=asio::error::eof && e!=asio::error::connection_reset)throw asio::system_error(e);}return predicate();}
    ~Server(){for(unsigned i=0;i<3000 && !service.stop_for(1ms);++i){}std::error_code e;std::filesystem::remove_all(dir,e);}
};
std::string request(const char* id,const char* command){return std::string("<LSX><Request id='")+id+"'>"+command+"</Request></LSX>";}
std::string attr(const tinyxml2::XMLElement* e,const char* key){const auto* p=e->Attribute(key);return p?p:"!MISSING";}
int main(){
    const char* custom=R"("Config2" {"Game" {"Name" "Old" "GameName" "Neon & <Field>" "Version" "3" "GameVersion" "7" "Language" "tr_TR" "Languages" "tr_TR,en_US"} "Emulator" {"Online" "true"}})";
    for(const char* config:{static_cast<const char*>(nullptr),R"("Config2" {})",custom}){
        Server s;CHECK(s.start(config));const bool configured=config==custom;const std::string name=configured?"Neon & <Field>":"Game",version=configured?"7":"1.0.0.0",language=configured?"tr_TR":"en_US";const auto start=utc_wall_seconds();
        const auto data=request("unknown","<GetGameInfo GameInfoId='UNKNOWN'/>")+request("name&amp;lt;","<GetGameInfo GameInfoId='DISPLAY_&#78;AME'/>")+request("all","<GetAllGameInfo/>")+request("language","<GetSetting SettingId='LANGUAGE'/>")+request("available","<GetGameInfo GameInfoId='AVAILABLE_VERSION'/>")+request("presence","<GetPresence/>")+request("child","<GetGameInfo><GameInfoId>DISPLAY_NAME</GameInfoId></GetGameInfo>")+request("chunk","<QueryChunkStatus/>")+request("installed","<GetGameInfo GameInfoId='INSTALLED_VERSION'/>");
        asio::write(s.client,asio::buffer(data.data(),11));s.service.poll_for(1ms);asio::write(s.client,asio::buffer(data.data()+11,data.size()-11));asio::error_code ec;s.client.shutdown(tcp::socket::shutdown_send,ec);
        CHECK(s.until([&]{const auto p=s.service.snapshot();return p.completed==1 && s.received.size()==p.bytes_written;}));CHECK(s.service.snapshot().frames==9 && s.service.snapshot().responses==6 && s.service.snapshot().malformed==0);CHECK(s.received.find('\0')==std::string::npos);
        const char* ids[]={"name&lt;","all","language","available","presence","installed"};std::size_t offset=0;
        for(unsigned i=0;i<6;++i){const auto end=s.received.find("</LSX>",offset);CHECK(end!=std::string::npos);tinyxml2::XMLDocument doc;CHECK(doc.Parse(s.received.data()+offset,end+6-offset)==tinyxml2::XML_SUCCESS);const auto* response=doc.RootElement()->FirstChildElement("Response");CHECK(response && attr(response,"id")==ids[i] && attr(response,"sender")=="EbisuSDK");const auto* body=response->FirstChildElement();CHECK(body);
            if(i==0)CHECK(std::string(body->Name())=="GetGameInfoResponse" && attr(body,"GameInfo")==name);
            else if(i==1){CHECK(std::string(body->Name())=="GetAllGameInfoResponse" && attr(body,"DisplayName")==name && attr(body,"InstalledVersion")==version && attr(body,"AvailableVersion")==version && attr(body,"InstalledLanguage")==language && attr(body,"HasExpiration")=="false");const auto time=attr(body,"SystemTime");CHECK(time>=utc_text(start) && time<=utc_text(utc_wall_seconds()));if(configured)CHECK(attr(body,"Languages")=="tr_TR,en_US");}
            else if(i==2)CHECK(std::string(body->Name())=="GetSettingResponse" && attr(body,"Setting")==language);
            else if(i==4)CHECK(std::string(body->Name())=="GetPresenceResponse" && attr(body,"RichPresence")==name);
            else CHECK(std::string(body->Name())=="GetGameInfoResponse" && attr(body,"GameInfo")==version);
            offset=end+6;
        }
        CHECK(s.received.find_first_not_of(" \r\n\t",offset)==std::string::npos);CHECK(s.until([&]{return s.service.stop_for(1ms);}));
    }
    {Server s;CHECK(s.start(nullptr));const std::string bad="<LSX><Request><GetGameInfo GameInfoId='DISPLAY_NAME'/></Request></LSX>";asio::write(s.client,asio::buffer(bad));CHECK(s.until([&]{return s.service.snapshot().completed==1;}));CHECK(s.received.empty() && s.service.snapshot().malformed==1);}
    {Server s;const auto config=std::string("\"Config2\" {\"Game\" {\"Name\" \"")+std::string(20000,'X')+"\"}}";CHECK(s.start(config.c_str()));const auto frame=request("bounded","<GetAllGameInfo/>");asio::write(s.client,asio::buffer(frame));CHECK(s.until([&]{return s.service.snapshot().completed==1;}));CHECK(s.received.empty() && s.service.snapshot().responses==0 && s.service.snapshot().malformed==1);}
    std::cout<<"TCP_GAME_INFO_DEFAULT_CONFIG_FIFO_NO_RESPONSE_BOUNDED_PASS\n";
}
