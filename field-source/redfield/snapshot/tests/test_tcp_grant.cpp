// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/local_service.hpp"
#include "../third_party/tinyxml2/tinyxml2.h"
#include <asio/ip/tcp.hpp>
#include <asio/write.hpp>
#include <fstream>
#include <iostream>
#include <vector>
using namespace preloader_l;using asio::ip::tcp;using namespace std::chrono_literals;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
struct Server {
    std::filesystem::path dir=std::filesystem::temp_directory_path()/("rf20_grant_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));LocalService service{dir/"preloader_l.dll"};asio::io_context io;tcp::socket client{io};std::string received;
    Server(){if(!std::filesystem::create_directory(dir))throw std::runtime_error("fixture");}
    bool start(){std::ofstream f(dir/"surge.cfg");f<<R"("Config2" {"Achievements" {"AchievementsSet" "Set" "AchievementNames" {"A" "Alpha & Beta" "B" "Beta"} "AchievementTotals" {"A" "2" "B" "3"}}})";f.close();if(!f || !service.start())return false;client.connect({asio::ip::address_v4::loopback(),service.snapshot().port});client.non_blocking(true);return true;}
    template<class P>bool until(P p){const auto end=std::chrono::steady_clock::now()+3s;while(!p() && std::chrono::steady_clock::now()<end){service.poll_for(0ms);char b[4096];asio::error_code e;const auto n=client.read_some(asio::buffer(b),e);received.append(b,n);}return p();}
    ~Server(){for(unsigned i=0;i<3000 && !service.stop_for(1ms);++i){}std::error_code e;std::filesystem::remove_all(dir,e);}
};
std::string req(std::string body){return "<LSX><Request id='x' recipient='SDK'>"+body+"</Request></LSX>";}
std::string grant(std::string id,std::string progress){return req("<GrantAchievement><AchievementId>"+id+"</AchievementId><Progress>"+progress+"</Progress></GrantAchievement>");}
std::string attr(const tinyxml2::XMLElement* p,const char* key){const auto* s=p?p->Attribute(key):nullptr;return s?s:"MISSING";}
int main(){
    Server s;CHECK(s.start());const auto initial=s.service.configuration();CHECK(initial==s.service.achievements() && initial->achievements.size()==2);
    const auto wire=grant("A","1")+req("<QueryAchievements/>")+grant("A","1")+req("<QueryAchievements/>")+grant("A","99")+grant("B","0")+req("<QueryContent/>");
    asio::write(s.client,asio::buffer(wire.data(),17));s.service.poll_for(0ms);asio::write(s.client,asio::buffer(wire.data()+17,wire.size()-17));asio::error_code e;s.client.shutdown(tcp::socket::shutdown_send,e);
    CHECK(s.until([&]{const auto p=s.service.snapshot();return p.completed==1 && s.received.size()==p.bytes_written;}));
    const auto p=s.service.snapshot();CHECK(p.frames==7 && p.responses==7 && p.events==1 && p.committed_deliveries==4 && p.malformed==0 && p.failures==0);
    const auto committed=s.service.achievements();CHECK(committed!=initial && committed->achievements[0].progress==2 && committed->achievements[0].count==1 && committed->achievements[1].progress==0 && initial->achievements[0].progress==0 && s.service.configuration()==initial);
    std::vector<std::string> messages;std::size_t at=0;for(;;){const auto end=s.received.find("</LSX>",at);if(end==std::string::npos)break;messages.push_back(s.received.substr(at,end+6-at));at=end+6;}CHECK(messages.size()==8 && s.received.find_first_not_of(" \r\n\t",at)==std::string::npos);
    std::string completed_record;
    for(std::size_t i=0;i<messages.size();++i){tinyxml2::XMLDocument d;CHECK(d.Parse(messages[i].c_str())==tinyxml2::XML_SUCCESS);const auto* root=d.RootElement();CHECK(root);
      if(i==0 || i==2 || i==5 || i==6){const auto* a=root->FirstChildElement("Achievement");CHECK(a && attr(a,"Id")== (i==6?"B":"A") && attr(a,"Progress")== (i==0?"1":i==6?"0":"2") && attr(a,"Count")== (i==0 || i==6?"0":"1"));if(i==2)completed_record=attr(a,"GrantDate");CHECK(i==6 || (attr(a,"GrantDate").size()==19 && attr(a,"GrantDate").substr(0,4)!="0000"));}
      else if(i==3){const auto* event=root->FirstChildElement("Event");CHECK(event && attr(event,"sender")=="EbisuSDK");const auto* a=event->FirstChildElement("Achievement");CHECK(attr(a,"Id")=="A" && attr(a,"Progress")=="2" && attr(a,"Count")=="1" && attr(a,"GrantDate")==completed_record);}
      else if(i==1 || i==4){const auto* response=root->FirstChildElement("Response");CHECK(response);const auto* sets=response->FirstChildElement("AchievementSets");CHECK(sets);const auto* set=sets->FirstChildElement("AchievementSet");CHECK(set);const auto* a=set->FirstChildElement("Achievement");CHECK(attr(a,"Id")=="A" && attr(a,"Progress")== (i==1?"1":"2") && !a->NextSiblingElement() && attr(a,"Name")=="Alpha & Beta");}
      else CHECK(root->FirstChildElement("Response")->FirstChildElement("QueryContentResponse"));
    }
    CHECK(s.until([&]{return s.service.stop_for(1ms);}));CHECK(!s.service.achievements() && committed->achievements[0].count==1 && initial->achievements[0].count==0);
    std::cout<<"LOCAL_GRANT_PARTIAL_COMPLETE_EVENT_QUERY_COMMITTED_FIFO_HALF_CLOSE_PASS\n";
}
