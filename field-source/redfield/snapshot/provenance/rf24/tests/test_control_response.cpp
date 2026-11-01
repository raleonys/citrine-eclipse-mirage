// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/config.hpp"
#include "redfield/response_dispatch.hpp"
#include "redfield/handlers/integration_routes.hpp"
#include "../third_party/tinyxml2/tinyxml2.h"
#include <array>
#include <iostream>
#include <vector>
using namespace redfield;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
using Handler=redfield::handlers::HandlerStatus(*)(const xml::Request&,char*,std::size_t,std::size_t*) noexcept;
struct Case{const char* name;const char* code;const char* description;Handler handler;};
const Case cases[]={
    {"GoOnline","0","",redfield::handlers::handle_go_online},
    {"EnableVoip","-1593835518","The function you are calling is not implemented.",redfield::handlers::handle_enable_voip},
    {"SetDownloaderUtilization","0","",redfield::handlers::handle_set_downloader_utilization},
    {"SetPresenceVisibility","0","",redfield::handlers::handle_set_presence_visibility},
    {"SetPresence","0","",redfield::handlers::handle_set_presence},
    {"StartDownload","0","Requested a download of a DLC offer.",redfield::handlers::handle_start_download},
    {"RestartGame","1","Origin is waiting for the game to shutdown.",redfield::handlers::handle_restart_game}
};
ResponseResult render(const std::string& command,std::string& wire,const ConfigSnapshot* config=nullptr){
    xml::Request r;auto source="<LSX><Request id='A&amp;lt;' recipient='SDK&amp;X'>"+command+"</Request></LSX>";
    if(xml::parse(source,r)!=xml::Result::ok){wire.clear();return ResponseResult::rejected;}
    source.assign(source.size(),'!');std::array<char,16384> out{};std::size_t n=99;
    const auto result=config?dispatch_response_with_config(r,*config,out.data(),out.size(),&n):dispatch_response(r,out.data(),out.size(),&n);wire.assign(out.data(),n);return result;
}
int main(){
    ConfigStore initial,offline,online;CHECK(offline.replace(R"("Config2" {"Emulator" {"Online" "false"} "User" {"Username" "unchanged"}})")==ConfigResult::published);CHECK(online.replace(R"("Config2" {"Emulator" {"Online" "true"} "User" {"Username" "unchanged"}})")==ConfigResult::published);
    const auto initial_snapshot=initial.snapshot(),off=offline.snapshot(),on=online.snapshot();std::string wire;
    for(const auto* config:{static_cast<const ConfigSnapshot*>(nullptr),initial_snapshot.get(),off.get(),on.get()}){
        std::vector<std::string> before;
        if(config)for(const char* read:{"<GetInternetConnectedState/>","<GetSettings/>","<GetPresenceVisibility/>","<GetPresence/>","<GetProfile index='0'/>"}){CHECK(render(read,wire,config)==ResponseResult::response);before.push_back(wire);}
        for(const auto& c:cases){
            const auto expected=std::string(" <LSX>\n  <Response id=\"A&amp;lt;\" sender=\"SDK&amp;X\">\n    <ErrorSuccess Description=\"")+c.description+"\" Code=\""+c.code+"\"/>\n  </Response>\n </LSX>\n";
            for(const auto& command:{std::string("<")+c.name+"/>",std::string("<")+c.name+" Value='not-validated' Online='true' Visible='true'>ignored<Unknown><![CDATA[&lt;]]></Unknown></"+c.name+">"}){
                CHECK(render(command,wire,config)==ResponseResult::response && wire==expected);
                tinyxml2::XMLDocument d;CHECK(d.Parse(wire.data(),wire.size())==tinyxml2::XML_SUCCESS);const auto* r=d.RootElement()->FirstChildElement("Response");CHECK(r && r->Attribute("id","A&lt;") && r->Attribute("sender","SDK&X"));const auto* e=r->FirstChildElement("ErrorSuccess");CHECK(e && e->Attribute("Code",c.code) && e->Attribute("Description",c.description));
            }
        }
        if(config){std::size_t i=0;for(const char* read:{"<GetInternetConnectedState/>","<GetSettings/>","<GetPresenceVisibility/>","<GetPresence/>","<GetProfile index='0'/>"}){CHECK(render(read,wire,config)==ResponseResult::response && wire==before[i++]);}}
    }
    CHECK(initial.snapshot()==initial_snapshot && offline.snapshot()==off && online.snapshot()==on && !off->online && on->online);
    std::array<char,4096> out;
    for(const auto& c:cases){
        xml::Request r;const auto source=std::string("<LSX><Request id='id'><")+c.name+"/></Request></LSX>";CHECK(xml::parse(source,r)==xml::Result::ok);std::size_t n=99;
        out.fill('#');CHECK(dispatch_response(r,out.data(),8,&n)==ResponseResult::rejected && n==0 && out[0]=='#');CHECK(dispatch_response(r,out.data(),out.size(),&n)==ResponseResult::response);const auto need=n;out.fill('#');CHECK(dispatch_response(r,out.data(),need,&n)==ResponseResult::rejected && n==0 && out[0]=='#');CHECK(dispatch_response(r,out.data(),need+1,&n)==ResponseResult::response && n==need && out[n]=='\0');
        CHECK(dispatch_response(r,nullptr,out.size(),&n)==ResponseResult::rejected && n==0);CHECK(dispatch_response(r,out.data(),out.size(),nullptr)==ResponseResult::rejected);r.id={};out.fill('#');CHECK(dispatch_response(r,out.data(),out.size(),&n)==ResponseResult::rejected && n==0 && out[0]=='#');
        CHECK(xml::parse("<LSX><Request id='x'><Unbound/></Request></LSX>",r)==xml::Result::ok);CHECK(c.handler(r,out.data(),out.size(),&n)==redfield::handlers::HandlerStatus::wrong_command && n==0);
    }
    for(const char* command:{"<ShowIGO/>","<ShowIGOWindow/>","<GoOffline/>","<BlockUser/>","<goonline/>"})CHECK(render(command,wire)==ResponseResult::no_response && wire.empty());
    std::cout<<"CONTROL_LITERAL_CODES_DESCRIPTIONS_PAYLOAD_IGNORED_STATE_UNCHANGED_PASS\n";
}
