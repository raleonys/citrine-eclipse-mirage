// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/config.hpp"
#include "preloader_l/response_dispatch.hpp"
#include "../third_party/tinyxml2/tinyxml2.h"
#include <array>
#include <cstring>
#include <iostream>
using namespace preloader_l;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
struct Reply {
    std::array<char,16384> bytes{};std::size_t written=0;tinyxml2::XMLDocument dom;
    ResponseResult run(const ConfigSnapshot& snapshot,const std::string& command){xml::Request request;const auto source="<LSX><Request id='A&amp;lt;' recipient='E&amp;ALS'>"+command+"</Request></LSX>";if(xml::parse(source,request)!=xml::Result::ok)throw std::runtime_error("request");return dispatch_response_with_config(request,snapshot,bytes.data(),bytes.size(),&written);}
    const tinyxml2::XMLElement* body(){if(dom.Parse(bytes.data(),written)!=tinyxml2::XML_SUCCESS)return nullptr;const auto* r=dom.RootElement()->FirstChildElement("Response");if(!r || std::strcmp(r->Attribute("id"),"A&lt;") || std::strcmp(r->Attribute("sender"),"E&ALS"))return nullptr;return r->FirstChildElement();}
};
int main(){
    ConfigStore store;Reply r;
    for(const auto& text:{std::string("\"Config2\" {}"),std::string("\"Config2\" {\"Emulator\" {\"Online\" \"1\"}}"),std::string("\"Config2\" {\"Emulator\" {\"Online\" \"false\"}}")}){
        CHECK(store.replace(text)==ConfigResult::published);auto s=store.snapshot();CHECK(!s->online);
        CHECK(r.run(*s,"<GetInternetConnectedState/>")==ResponseResult::response);auto* b=r.body();CHECK(b && std::strcmp(b->Attribute("connected"),"false")==0);
        CHECK(r.run(*s,"<GetSettings/>")==ResponseResult::response);b=r.body();CHECK(b && std::strcmp(b->Attribute("IsManualOffline"),"true")==0 && std::strcmp(b->Attribute("IsIGOEnabled"),"false")==0);
    }
    CHECK(store.replace("\"Config2\" {\"Game\" {\"Language\" \"tr_TR\"} \"Emulator\" {\"Online\" \"true\" \"Online\" \"0\" \"Language\" \"wrong\" \"Environment\" \"old\" \"Environment\" \"stage&dev\" \"IGOEnabled\" \"0\" \"IGOAvailable\" \"1\" \"IsTelemetryEnabled\" \"A&B\" \"EALaunchOfflineMode\" \"true\"} \"Unknown\" {\"Environment\" \"wrong\"}}") ==ConfigResult::published);
    const auto pinned=store.snapshot();CHECK(pinned->online);
    CHECK(r.run(*pinned,"<GetSettings/>")==ResponseResult::response);auto* b=r.body();CHECK(b && std::strcmp(b->Attribute("IsManualOffline"),"false")==0 && std::strcmp(b->Attribute("Environment"),"stage&dev")==0 && std::strcmp(b->Attribute("Language"),"tr_TR")==0 && std::strcmp(b->Attribute("IsIGOAvailable"),"true")==0 && std::strcmp(b->Attribute("IsIGOEnabled"),"false")==0 && std::strcmp(b->Attribute("IsTelemetryEnabled"),"A&B")==0);
    struct Selector{const char* id;const char* value;};
    for(const auto c:{Selector{"LANG&#85;AGE","tr_TR"},Selector{"ENVIRONMENT","stage&dev"},Selector{"IS_MANUAL_OFFLINE","false"},Selector{"IS_IGO_ENABLED","DOWN"},Selector{"IS_IGO_AVAILABLE","true"},Selector{"IS_TELEMETRY_ENABLED","A&B"}}){
        CHECK(r.run(*pinned,std::string("<GetSetting SettingId='")+c.id+"'/>")==ResponseResult::response);b=r.body();CHECK(b && std::strcmp(b->Attribute("Setting"),c.value)==0);
    }
    CHECK(store.replace("\"Config2\" {\"Emulator\" {\"Online\" \"false\"}}") ==ConfigResult::published);CHECK(r.run(*pinned,"<GetSetting SettingId='IS_MANUAL_OFFLINE'/>")==ResponseResult::response);CHECK(std::strcmp(r.body()->Attribute("Setting"),"false")==0);
    for(const auto command:{"<GetSetting/>","<GetSetting SettingId='UNKNOWN'/>","<GetSetting SettingId='LANG&amp;UAGE'/>"})CHECK(r.run(*pinned,command)==ResponseResult::no_response && r.written==0);
    CHECK(store.replace("\"Config2\" {\"Emulator\" {\"IGOEnabled\" \"maybe\"}}") ==ConfigResult::published);CHECK(r.run(*store.snapshot(),"<GetSettings/>")==ResponseResult::no_response && r.written==0);CHECK(r.run(*store.snapshot(),"<GetSetting SettingId='IS_IGO_ENABLED'/>")==ResponseResult::no_response);
    xml::Request request;CHECK(xml::parse("<LSX><Request id='1'><GetSettings/></Request></LSX>",request)==xml::Result::ok);std::size_t n=99;CHECK(dispatch_response(request,r.bytes.data(),r.bytes.size(),&n)==ResponseResult::no_response && n==0);
    r.bytes.fill('#');CHECK(dispatch_response_with_config(request,*pinned,r.bytes.data(),8,&n)==ResponseResult::rejected && n==0 && r.bytes[0]=='#');
    CHECK(xml::parse("<LSX><Request><GetInternetConnectedState/></Request></LSX>",request)==xml::Result::ok);CHECK(dispatch_response_with_config(request,*pinned,r.bytes.data(),r.bytes.size(),&n)==ResponseResult::rejected && n==0);
    std::cout<<"SETTINGS_PINNED_POLICY_AND_SELECTOR_PASS\n";
}
