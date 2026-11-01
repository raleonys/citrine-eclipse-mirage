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
    std::filesystem::path dir=std::filesystem::temp_directory_path()/("rf17_ach_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));LocalService service{dir/"preloader_l.dll"};asio::io_context io;tcp::socket client{io};std::string received;
    Server(){if(!std::filesystem::create_directory(dir))throw std::runtime_error("fixture");}
    bool start(const std::string* config){if(config){std::ofstream f(dir/"redfield.cfg",std::ios::binary);f<<*config;f.close();if(!f)return false;}if(!service.start())return false;client.connect({asio::ip::address_v4::loopback(),service.snapshot().port});client.non_blocking(true);return true;}
    template<class P>bool until(P predicate){const auto end=std::chrono::steady_clock::now()+3s;while(!predicate() && std::chrono::steady_clock::now()<end){service.poll_for(1ms);char b[4096];asio::error_code e;const auto n=client.read_some(asio::buffer(b),e);received.append(b,n);if(e && e!=asio::error::would_block && e!=asio::error::try_again && e!=asio::error::eof && e!=asio::error::connection_reset)throw asio::system_error(e);}return predicate();}
    ~Server(){for(unsigned i=0;i<3000 && !service.stop_for(1ms);++i){}std::error_code e;std::filesystem::remove_all(dir,e);}
};
std::string request(const std::string& id,const std::string& command){return "<LSX><Request id='"+id+"'>"+command+"</Request></LSX>";}
std::string attr(const tinyxml2::XMLElement* e,const char* key){const auto* v=e->Attribute(key);return v?v:"!MISSING";}
int main(){
    const std::string empty=R"("Config2" {})",data=R"("Config2" {"Game" {"Name" "Configured"} "Content" {"game" {"Name" "configured"}} "Entitlements" {"E" {"Group" "g"}} "Achievements" {"AchievementsSet" "Set" "AchievementNames" {"B" "Beta" "A" "Alpha"} "AchievementTotals" {"A" "5" "A" "7"}}})";
    for(const auto* config:{static_cast<const std::string*>(nullptr),&empty,&data}){Server s;CHECK(s.start(config));const auto held=s.service.configuration();CHECK(held);const auto count=held->achievements.size();
        const auto frames=request("unbound","<GrantAchievement/>")+request("A&amp;lt;","<QueryAchievements><All>true</All></QueryAchievements>")+request("default","<QueryAchievements/>")+request("spaced","<QueryAchievements><All> true </All></QueryAchievements>")+request("content","<QueryContent/>")+request("ent","<QueryEntitlements/>")+request("entity","<QueryAchievements><All>&#116;rue</All></QueryAchievements>");
        asio::write(s.client,asio::buffer(frames.data(),13));s.service.poll_for(1ms);asio::write(s.client,asio::buffer(frames.data()+13,frames.size()-13));asio::error_code ec;s.client.shutdown(tcp::socket::shutdown_send,ec);
        CHECK(s.until([&]{const auto p=s.service.snapshot();return p.completed==1 && s.received.size()==p.bytes_written;}));CHECK(s.service.snapshot().frames==7 && s.service.snapshot().responses==6 && s.service.snapshot().malformed==0 && s.service.configuration()==held);
        std::size_t offset=0;
        for(unsigned i=0;i<6;++i){const auto end=s.received.find("</LSX>",offset);CHECK(end!=std::string::npos);tinyxml2::XMLDocument d;CHECK(d.Parse(s.received.data()+offset,end+6-offset)==tinyxml2::XML_SUCCESS);const auto* response=d.RootElement()->FirstChildElement("Response");CHECK(response);const auto* body=response->FirstChildElement();CHECK(body);
            if(i==3 || i==4){CHECK(std::string(body->Name())==(i==3?"QueryContentResponse":"QueryEntitlementsResponse"));std::size_t n=0;for(auto* e=body->FirstChildElement();e;e=e->NextSiblingElement())++n;CHECK(n==(i==3?held->content.size():held->entitlements.size()));}
            else {CHECK(std::string(body->Name())=="AchievementSets");const auto* set=body->FirstChildElement("AchievementSet");CHECK(set && attr(set,"GameName")== (config==&data?"Configured":"Game") && attr(set,"Name")== (config==&data?"Set":""));std::size_t n=0;for(auto* a=set->FirstChildElement();a;a=a->NextSiblingElement()){CHECK(std::string(a->Name())=="Achievement" && attr(a,"Progress")=="0" && attr(a,"Count")=="0");if(config==&data)CHECK(attr(a,"Id")== (n<2?"A":"B") && attr(a,"Total")== (n==0?"5":n==1?"7":"1"));++n;}CHECK(n==((i==0 || i==5)?count:0u));if(i==0)CHECK(attr(response,"id")=="A&lt;");}
            offset=end+6;
        }
        CHECK(s.received.find_first_not_of(" \r\n\t",offset)==std::string::npos);CHECK(s.until([&]{return s.service.stop_for(1ms);}));CHECK(held->achievements.size()==count);if(count)CHECK(held->achievements[0].name=="Alpha");
    }
    for(const auto* config:{&empty,&data})for(const auto* body:{"<QueryAchievements><All><Nested/></All></QueryAchievements>","<QueryAchievements>bad<All>true</All></QueryAchievements>"}){Server s;CHECK(s.start(config));const auto bad=request("invalid",body);asio::write(s.client,asio::buffer(bad));CHECK(s.until([&]{return s.service.snapshot().completed==1;}));CHECK(s.received.empty() && s.service.snapshot().malformed==1 && s.service.snapshot().responses==0);}
    std::string huge="\"Config2\" {\"Achievements\" {\"AchievementNames\" {";for(unsigned i=0;i<100;++i)huge+='"'+std::to_string(i)+"\" \"name\"";huge+="}}}";
    {Server s;CHECK(s.start(&huge));const auto frame=request("big","<QueryAchievements><All>true</All></QueryAchievements>");asio::write(s.client,asio::buffer(frame));CHECK(s.until([&]{return s.service.snapshot().completed==1;}));CHECK(s.received.empty() && s.service.snapshot().malformed==1 && s.service.snapshot().responses==0 && s.service.configuration()->achievements.size()==100);}
    const std::string unsafe="\"Config2\" {\"Achievements\" {\"AchievementNames\" {\"A\" \"safe\" \"B\" \"bad"+std::string(1,'\1')+"\"}}}";
    for(bool all:{false,true}){Server s;CHECK(s.start(&unsafe));const auto frame=request("unsafe",all?"<QueryAchievements><All>true</All></QueryAchievements>":"<QueryAchievements/>");asio::write(s.client,asio::buffer(frame));asio::error_code ec;s.client.shutdown(tcp::socket::shutdown_send,ec);CHECK(s.until([&]{const auto p=s.service.snapshot();return p.completed==1 && s.received.size()==p.bytes_written;}));CHECK(s.service.snapshot().responses==(all?0u:1u) && s.service.snapshot().malformed==(all?1u:0u));CHECK(all?s.received.empty():s.received.find("<Achievement Id=")==std::string::npos);}
    std::cout<<"TCP_ACHIEVEMENTS_ALL_TOTAL_ORDER_FIFO_HALF_CLOSE_CAP_REJECTION_PASS\n";
}
