// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/content_response.hpp"
#include "preloader_l/config.hpp"
#include "preloader_l/auth_code_session.hpp"
#include "../third_party/tinyxml2/tinyxml2.h"
#include <array>
#include <atomic>
#include <iostream>
#include <thread>
using namespace preloader_l;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
ResponseResult render(const ConfigSnapshot& config,std::string& wire,const std::string& command="<QueryContent/>"){
    xml::Request request;std::string source="<LSX><Request id='C&amp;lt;' recipient='Store&amp;X'>"+command+"</Request></LSX>";
    if(xml::parse(source,request)!=xml::Result::ok)return ResponseResult::rejected;
    source.assign(source.size(),'!');std::array<char,16384> out{};std::size_t n=99;const auto result=dispatch_response_with_config(request,config,out.data(),out.size(),&n);wire.assign(out.data(),n);return result;
}
std::string empty_wire(){return " <LSX>\n  <Response id=\"C&amp;lt;\" sender=\"Store&amp;X\">\n    <QueryContentResponse/>\n  </Response>\n </LSX>\n";}
int main(){
    ConfigStore store;std::string wire;CHECK(render(*store.snapshot(),wire)==ResponseResult::response && wire==empty_wire());
    CHECK(store.replace(R"("Config2" {"Emulator" {"Online" "false"} "Content" {"z&" {"Name" "A&amp;B" "Version" "v<1" "State" "CUSTOM"} "d" {"Other" "ignored"}}})")==ConfigResult::published);const auto held=store.snapshot();
    const std::string expected=" <LSX>\n  <Response id=\"C&amp;lt;\" sender=\"Store&amp;X\">\n    <QueryContentResponse>\n      <Game progressValue=\"0\" contentID=\"z&amp;\" availableVersion=\"v&lt;1\" displayName=\"A&amp;amp;B\" state=\"CUSTOM\" installedVersion=\"v&lt;1\"/>\n      <Game progressValue=\"0\" contentID=\"d\" availableVersion=\"0\" displayName=\"GAME\" state=\"INSTALLED\" installedVersion=\"0\"/>\n    </QueryContentResponse>\n  </Response>\n </LSX>\n";
    CHECK(render(*held,wire)==ResponseResult::response && wire==expected);CHECK(render(*held,wire,"<QueryContent UserId='invalid' Filter='nothing'><Other/></QueryContent>")==ResponseResult::response && wire==expected);
    xml::Request r;CHECK(xml::parse("<LSX><Request id='C&amp;lt;' recipient='Store&amp;X'><QueryContent/></Request></LSX>",r)==xml::Result::ok);std::array<char,16384> out;out.fill('#');std::size_t n=99;
    CHECK(dispatch_response(r,out.data(),out.size(),&n)==ResponseResult::no_response && n==0);CHECK(dispatch_content_response(r,*held,out.data(),out.size(),&n)==ResponseResult::response && std::string(out.data(),n)==expected);const auto need=n;
    out.fill('#');CHECK(dispatch_content_response(r,*held,out.data(),8,&n)==ResponseResult::rejected && n==0 && out[0]=='#');CHECK(dispatch_content_response(r,*held,out.data(),need,&n)==ResponseResult::rejected && n==0 && out[0]=='#');CHECK(dispatch_content_response(r,*held,out.data(),need+1,&n)==ResponseResult::response && n==need && out[n]=='\0');
    CHECK(dispatch_content_response(r,*held,nullptr,out.size(),&n)==ResponseResult::rejected && n==0);CHECK(dispatch_content_response(r,*held,out.data(),0,&n)==ResponseResult::rejected && n==0);CHECK(dispatch_content_response(r,*held,out.data(),out.size(),nullptr)==ResponseResult::rejected);
    const auto id=r.id;r.id={};out.fill('#');CHECK(dispatch_content_response(r,*held,out.data(),out.size(),&n)==ResponseResult::rejected && n==0 && out[0]=='#');r.id=id;
    // A late invalid field is validated during the first pass; no output prefix.
    for(unsigned field=0;field<4;++field){ConfigSnapshot invalid=*held;auto& bad=invalid.content.back();std::string* values[]={&bad.content_id,&bad.display_name,&bad.version,&bad.state};*values[field]=std::string(1,'\1');out.fill('#');CHECK(dispatch_content_response(r,invalid,out.data(),out.size(),&n)==ResponseResult::rejected && n==0 && out[0]=='#');}
    ConfigSnapshot invalid=*held;invalid.content.back().display_name=std::string("bad\0tail",8);out.fill('#');CHECK(dispatch_content_response(r,invalid,out.data(),out.size(),&n)==ResponseResult::rejected && n==0 && out[0]=='#');
    CHECK(store.replace(R"("Config2" {})")==ConfigResult::published);CHECK(render(*store.snapshot(),wire)==ResponseResult::response && wire==empty_wire());CHECK(render(*held,wire)==ResponseResult::response && wire==expected);
    AuthCodeSession auth;CHECK(dispatch_response_with_session(r,*held,auth,out.data(),out.size(),&n)==ResponseResult::response && std::string(out.data(),n)==expected && !auth.ready());
    r.command={"querycontent",12};CHECK(dispatch_content_response(r,*held,out.data(),out.size(),&n)==ResponseResult::no_response && n==0);
    // No arbitrary 512-record limit: validate a >16KiB response in a larger unit buffer.
    ConfigSnapshot many;for(unsigned i=0;i<600;++i)many.content.push_back({std::to_string(i),"GAME","0","INSTALLED"});CHECK(xml::parse("<LSX><Request id='large'><QueryContent/></Request></LSX>",r)==xml::Result::ok);std::string large(200000,'#');CHECK(dispatch_content_response(r,many,large.data(),large.size(),&n)==ResponseResult::response && n>16384);tinyxml2::XMLDocument document;CHECK(document.Parse(large.data(),n)==tinyxml2::XML_SUCCESS);unsigned count=0;for(auto* e=document.RootElement()->FirstChildElement()->FirstChildElement()->FirstChildElement();e;e=e->NextSiblingElement())++count;CHECK(count==600);out.fill('#');CHECK(dispatch_content_response(r,many,out.data(),out.size(),&n)==ResponseResult::rejected && n==0 && out[0]=='#');
    const char* yes=R"("Config2" {"Emulator" {"Online" "true"} "Content" {"A" {"Name" "alpha" "Version" "1"}}})";const char* no=R"("Config2" {"Emulator" {"Online" "false"} "Content" {"B" {"Name" "beta" "Version" "2"}}})";CHECK(store.replace(yes)==ConfigResult::published);std::atomic<bool> bad=false;std::thread writer([&]{for(unsigned i=0;i<200;++i)if(store.replace(i%2?yes:no)!=ConfigResult::published)bad=true;});
    for(unsigned i=0;i<200;++i){const auto s=store.snapshot();std::string w;if(render(*s,w)!=ResponseResult::response || w.find(s->online?"contentID=\"A\"":"contentID=\"B\"")==std::string::npos || w.find(s->online?"displayName=\"alpha\"":"displayName=\"beta\"")==std::string::npos)bad=true;}
    writer.join();CHECK(!bad);
    std::cout<<"QUERY_CONTENT_TYPED_SNAPSHOT_WIRE_MEASURE_RENDER_CAPACITY_CONCURRENCY_PASS\n";
}
