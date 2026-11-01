// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/config.hpp"
#include "redfield/response_dispatch.hpp"
#include "../third_party/tinyxml2/tinyxml2.h"
#include <array>
#include <atomic>
#include <iostream>
#include <thread>
using namespace redfield;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
ResponseResult render(const ConfigSnapshot& snapshot,const std::string& command,std::string& wire){
    xml::Request r;auto source="<LSX><Request id='P&amp;lt;' recipient='SDK&amp;X'>"+command+"</Request></LSX>";
    if(xml::parse(source,r)!=xml::Result::ok){wire.clear();return ResponseResult::rejected;}
    source.assign(source.size(),'!');std::array<char,16384> out{};std::size_t n=99;const auto result=dispatch_response_with_config(r,snapshot,out.data(),out.size(),&n);wire.assign(out.data(),n);return result;
}
std::string envelope(bool online){return std::string(" <LSX>\n  <Response id=\"P&amp;lt;\" sender=\"SDK&amp;X\">\n    ")+(online?"<CheckPermissionResponse Access=\"GRANTED\"/>":"<ErrorSuccess Description=\"\" Code=\"-1576468080\"/>")+"\n  </Response>\n </LSX>\n";}
bool policy(const ConfigSnapshot& snapshot,bool online){
    std::string wire;if(render(snapshot,"<CheckPermission/>",wire)!=ResponseResult::response || wire!=envelope(online))return false;
    tinyxml2::XMLDocument d;if(render(snapshot,"<GetInternetConnectedState/>",wire)!=ResponseResult::response || d.Parse(wire.data(),wire.size())!=tinyxml2::XML_SUCCESS)return false;
    const auto* body=d.RootElement()->FirstChildElement()->FirstChildElement();if(!body || std::string(body->Name())!="InternetConnectedState" || !body->Attribute("connected",online?"true":"false"))return false;
    if(render(snapshot,"<GetSettings/>",wire)!=ResponseResult::response || d.Parse(wire.data(),wire.size())!=tinyxml2::XML_SUCCESS)return false;
    body=d.RootElement()->FirstChildElement()->FirstChildElement();return body && body->Attribute("IsManualOffline",online?"false":"true");
}
int main(){
    ConfigStore initial;CHECK(policy(*initial.snapshot(),false));
    struct Case{const char* text;bool online;};const Case cases[]={
        {R"("Config2" {})",false},
        {R"("Config2" {"Emulator" {"Online" "true"}})",true},
        {R"("Config2" {"Emulator" {"Online" "false"}})",false},
        {R"("Config2" {"Emulator" {"Online" "1"}})",false},
        {R"("Config2" {"Emulator" {"Online" "0"}})",false},
        {R"("Config2" {"Emulator" {"Online" "TRUE"}})",false},
        {R"("Config2" {"Emulator" {"Online" "true" "Online" "0"}})",true},
        {R"("Config2" {"Emulator" {"Online" "true" "Online" "false" "Online" "1"}})",false},
        {R"("Config2" {"Emulator" {"Online" "false" "Online" "true"}})",true},
        {R"("Config2" {"Game" {"Online" "true"} "User" {"Online" "true"} "Online" "true"})",false},
        {R"("Config2" {"Emulator" {"Nested" {"Online" "true"}}})",false}
    };
    ConfigStore store;std::string wire;
    for(const auto& c:cases){CHECK(store.replace(c.text)==ConfigResult::published);const auto snapshot=store.snapshot();CHECK(snapshot->online==c.online && policy(*snapshot,c.online));
        CHECK(render(*snapshot,"<CheckPermission PermissionId='unknown' UserId='0'>ignored<Unknown/></CheckPermission>",wire)==ResponseResult::response && wire==envelope(c.online));CHECK(store.snapshot()==snapshot);
        xml::Request r;CHECK(xml::parse("<LSX><Request id='id'><CheckPermission/></Request></LSX>",r)==xml::Result::ok);std::array<char,4096> out;out.fill('#');std::size_t n=99;
        CHECK(dispatch_response_with_config(r,*snapshot,out.data(),8,&n)==ResponseResult::rejected && n==0 && out[0]=='#');CHECK(dispatch_response_with_config(r,*snapshot,out.data(),out.size(),&n)==ResponseResult::response);const auto need=n;out.fill('#');CHECK(dispatch_response_with_config(r,*snapshot,out.data(),need,&n)==ResponseResult::rejected && n==0 && out[0]=='#');CHECK(dispatch_response_with_config(r,*snapshot,out.data(),need+1,&n)==ResponseResult::response && n==need && out[n]=='\0');
        CHECK(dispatch_response(r,out.data(),out.size(),&n)==ResponseResult::no_response && n==0);CHECK(dispatch_response_with_config(r,*snapshot,nullptr,out.size(),&n)==ResponseResult::rejected && n==0);CHECK(dispatch_response_with_config(r,*snapshot,out.data(),out.size(),nullptr)==ResponseResult::rejected);r.id={};out.fill('#');CHECK(dispatch_response_with_config(r,*snapshot,out.data(),out.size(),&n)==ResponseResult::rejected && n==0 && out[0]=='#');
    }
    const char* yes=R"("Config2" {"Emulator" {"Online" "true"}})";const char* no=R"("Config2" {"Emulator" {"Online" "false"}})";
    CHECK(store.replace(yes)==ConfigResult::published);const auto held=store.snapshot();CHECK(store.replace(R"("Config2" {)")==ConfigResult::invalid && store.snapshot()==held);CHECK(store.replace(no)==ConfigResult::published);CHECK(policy(*held,true) && policy(*store.snapshot(),false));
    CHECK(render(*store.snapshot(),"<GoOnline/>",wire)==ResponseResult::response && policy(*store.snapshot(),false));CHECK(render(*held,"<checkpermission/>",wire)==ResponseResult::no_response && wire.empty());CHECK(render(*held,"<GetAuthCode ClientId=''/>",wire)==ResponseResult::no_response && wire.empty());
    std::atomic<bool> bad=false;std::thread writer([&]{for(int i=0;i<300;++i)if(store.replace(i%2?yes:no)!=ConfigResult::published)bad=true;});for(int i=0;i<300;++i){const auto s=store.snapshot();if(!policy(*s,s->online) || !policy(*held,true) || !policy(*initial.snapshot(),false))bad=true;}writer.join();CHECK(!bad);
    std::cout<<"PERMISSION_STRICT_ONLINE_MATRIX_SHARED_SNAPSHOT_CAPACITY_PASS\n";
}
