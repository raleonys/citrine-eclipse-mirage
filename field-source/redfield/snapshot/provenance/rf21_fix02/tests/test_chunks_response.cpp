// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/chunks_response.hpp"
#include "redfield/config.hpp"
#include "preloader_l/handlers/integration_routes.hpp"
#include "../third_party/tinyxml2/tinyxml2.h"
#include <array>
#include <iostream>
#include <vector>
using namespace redfield;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
std::string envelope(const std::string& rows){return " <LSX>\n  <Response id=\"C&amp;lt;\" sender=\"SDK&amp;X\">\n    <AreChunksInstalledResponse ItemId=\"\" Installed=\"true\">"+rows+"</AreChunksInstalledResponse>\n  </Response>\n </LSX>\n";}
ResponseResult render(const std::string& command,std::string& wire,std::size_t capacity=16384,const ConfigSnapshot* config=nullptr){
    xml::Request request;auto source="<LSX><Request id='C&amp;lt;' recipient='SDK&amp;X'>"+command+"</Request></LSX>";
    if(xml::parse(source,request)!=xml::Result::ok){wire.clear();return ResponseResult::rejected;}
    source.assign(source.size(),'!');std::vector<char> out(capacity,'#');std::size_t n=99;
    const auto result=config?dispatch_response_with_config(request,*config,out.data(),out.size(),&n):dispatch_response(request,out.data(),out.size(),&n);
    wire.assign(out.data(),n);return result;
}
std::size_t rows(const std::string& wire){tinyxml2::XMLDocument d;if(d.Parse(wire.data(),wire.size())!=tinyxml2::XML_SUCCESS)return static_cast<std::size_t>(-1);const auto* r=d.RootElement()->FirstChildElement("Response");if(!r || !r->Attribute("id","C&lt;") || !r->Attribute("sender","SDK&X"))return static_cast<std::size_t>(-1);const auto* body=r->FirstChildElement("AreChunksInstalledResponse");if(!body || !body->Attribute("Installed","true") || !body->Attribute("ItemId",""))return static_cast<std::size_t>(-1);std::size_t n=0;for(const auto* c=body->FirstChildElement();c;c=c->NextSiblingElement()){if(std::string(c->Name())!="ChunkIds" || !c->GetText() || std::string(c->GetText())!="1")return static_cast<std::size_t>(-1);++n;}return n;}
int main(){
    std::string wire;CHECK(render("<AreChunksInstalled/>",wire)==ResponseResult::response && wire==envelope(""));
    const std::string command="<AreChunksInstalled ItemId='ignored' ChunkIds='999'><!--ok--><Other><ChunkIds>bad</ChunkIds></Other><ChunkIds>+0007</ChunkIds><ChunkIds>-0</ChunkIds><ChunkIds>-2147483648</ChunkIds><ChunkIds>2147483647</ChunkIds><ChunkIds>&#43;0009</ChunkIds><ChunkIds><![CDATA[-0008]]></ChunkIds><ChunkIds>+0007</ChunkIds></AreChunksInstalled>";
    const auto expected=envelope("<ChunkIds>7</ChunkIds><ChunkIds>0</ChunkIds><ChunkIds>-2147483648</ChunkIds><ChunkIds>2147483647</ChunkIds><ChunkIds>9</ChunkIds><ChunkIds>-8</ChunkIds><ChunkIds>7</ChunkIds>");
    CHECK(render(command,wire)==ResponseResult::response && wire==expected);
    CHECK(render("<AreChunksInstalled><Unknown/><chunkids>bad</chunkids></AreChunksInstalled>",wire)==ResponseResult::response && wire==envelope(""));
    for(const char* value:{"", "+", "-", "2147483648", "-2147483649", "+2147483648", " 1 ", "1.0", "0x10", "1&bogus;", "<![CDATA[&#49;]]>", "<!--c-->1", "1<tail/>", "<nested>1</nested>", "1<![CDATA[2]]>", "１２"}){
        CHECK(render(std::string("<AreChunksInstalled><ChunkIds>1</ChunkIds><ChunkIds>")+value+"</ChunkIds></AreChunksInstalled>",wire)==ResponseResult::no_response && wire.empty());
    }
    CHECK(render("<AreChunksInstalled><ChunkIds/></AreChunksInstalled>",wire)==ResponseResult::no_response && wire.empty());
    CHECK(render("<AreChunksInstalled><ChunkIds>"+std::string(1000,'9')+"</ChunkIds></AreChunksInstalled>",wire)==ResponseResult::no_response && wire.empty());
    CHECK(render("<AreChunksInstalled>bad<ChunkIds>1</ChunkIds></AreChunksInstalled>",wire)==ResponseResult::rejected && wire.empty());
    ConfigStore offline,online;CHECK(online.replace(R"("Config2" {"Emulator" {"Online" "true"}})")==ConfigResult::published);const auto before=online.snapshot();
    CHECK(render(command,wire,16384,offline.snapshot().get())==ResponseResult::response && wire==expected);CHECK(render(command,wire,16384,before.get())==ResponseResult::response && wire==expected && before==online.snapshot() && before->online);
    xml::Request request;CHECK(xml::parse("<LSX><Request id='x'><AreChunksInstalled><ChunkIds>1</ChunkIds></AreChunksInstalled></Request></LSX>",request)==xml::Result::ok);
    std::array<char,4096> out;out.fill('#');std::size_t n=99;CHECK(dispatch_response(request,out.data(),8,&n)==ResponseResult::rejected && n==0 && out[0]=='#');
    CHECK(dispatch_response(request,out.data(),out.size(),&n)==ResponseResult::response);const auto needed=n;out.fill('#');CHECK(dispatch_response(request,out.data(),needed,&n)==ResponseResult::rejected && n==0 && out[0]=='#');CHECK(dispatch_response(request,out.data(),needed+1,&n)==ResponseResult::response && n==needed && out[n]=='\0');
    CHECK(dispatch_response(request,nullptr,out.size(),&n)==ResponseResult::rejected && n==0);CHECK(dispatch_response(request,out.data(),out.size(),nullptr)==ResponseResult::rejected);
    request.id={};out.fill('#');CHECK(dispatch_response(request,out.data(),out.size(),&n)==ResponseResult::rejected && n==0 && out[0]=='#');
    CHECK(xml::parse("<LSX><Request id='x'><AreChunksInstalled><ChunkIds>1</ChunkIds><ChunkIds>bad</ChunkIds></AreChunksInstalled></Request></LSX>",request)==xml::Result::ok);out.fill('#');CHECK(dispatch_response(request,out.data(),out.size(),&n)==ResponseResult::no_response && n==0 && out[0]=='#');
    std::size_t measured=99;CHECK(preloader_l::handlers::handle_are_chunks_installed(request,nullptr,0,&measured)==preloader_l::handlers::HandlerStatus::unsupported && measured==0);
    std::string many="<AreChunksInstalled>";for(unsigned i=0;i<800;++i)many+="<ChunkIds>1</ChunkIds>";many+="</AreChunksInstalled>";
    CHECK(render(many,wire,40000)==ResponseResult::response && rows(wire)==800);CHECK(render(many,wire)==ResponseResult::rejected && wire.empty());
    std::cout<<"CHUNKS_INT32_CANONICALIZATION_DOM_TWO_PASS_CAPACITY_PASS\n";
}
