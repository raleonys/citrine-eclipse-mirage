// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/query_presence_response.hpp"
#include "../third_party/tinyxml2/tinyxml2.h"
#include <array>
#include <atomic>
#include <iostream>
#include <thread>
using namespace preloader_l;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
std::string friend_xml(const std::string& id,const std::string& persona){return "<Friend AvatarId=\"\" Title=\"\" GroupId=\"\" MultiplayerId=\"\" TitleId=\"\" Presence=\"UNKNOWN\" PersonaId=\"0\" GamePresence=\"\" Group=\"\" Persona=\""+persona+"\" RichPresence=\"\" UserId=\""+id+"\" State=\"NONE\"/>";}
std::string envelope(const std::string& rows){return " <LSX>\n  <Response id=\"P&amp;lt;\" sender=\"SDK&amp;X\">\n    <QueryPresenceResponse>"+rows+"</QueryPresenceResponse>\n  </Response>\n </LSX>\n";}
ResponseResult render(const ConfigSnapshot& snapshot,const std::string& command,std::string& output,std::size_t capacity=16384){xml::Request request;auto source="<LSX><Request id='P&amp;lt;' recipient='SDK&amp;X'>"+command+"</Request></LSX>";if(xml::parse(source,request)!=xml::Result::ok)return ResponseResult::rejected;source.assign(source.size(),'!');std::vector<char> buffer(capacity,'#');std::size_t n=99;const auto result=dispatch_response_with_config(request,snapshot,buffer.data(),buffer.size(),&n);output.assign(buffer.data(),n);return result;}
std::size_t friends(const std::string& output,const char* persona){tinyxml2::XMLDocument d;if(d.Parse(output.data(),output.size())!=tinyxml2::XML_SUCCESS)return static_cast<std::size_t>(-1);const auto* q=d.RootElement()->FirstChildElement()->FirstChildElement();std::size_t n=0;for(const auto* f=q->FirstChildElement();f;f=f->NextSiblingElement()){if(!f->Attribute("Persona") || std::string(f->Attribute("Persona"))!=persona)return static_cast<std::size_t>(-1);++n;}return n;}
int main(){
    ConfigStore store;auto initial=store.snapshot();std::string output;
    CHECK(render(*initial,"<QueryPresence/>",output)==ResponseResult::response && output==envelope(""));
    CHECK(render(*initial,"<QueryPresence><u>7</u><any>8</any></QueryPresence>",output)==ResponseResult::response);CHECK(output==envelope(friend_xml("7","SURGE")+friend_xml("8","SURGE")));
    CHECK(store.replace(R"("Config2" {})")==ConfigResult::published);CHECK(render(*store.snapshot(),"<QueryPresence><u>7</u></QueryPresence>",output)==ResponseResult::response && output==envelope(friend_xml("7","")));
    CHECK(store.replace(R"("Config2" {"User" {"Username" "old" "Username" "Cenk & <Ω>"} "Game" {"Username" "wrong"}})")==ConfigResult::published);const auto old=store.snapshot();
    CHECK(render(*old,"<QueryPresence><!--command--><u><!--lead-->one<tail/>ignored</u><any><![CDATA[two&<]]><tail/>ignored</any></QueryPresence>",output)==ResponseResult::response);CHECK(output==envelope(friend_xml("one","Cenk &amp; &lt;Ω&gt;")+friend_xml("two&amp;&lt;","Cenk &amp; &lt;Ω&gt;")));
    CHECK(render(*old,"<QueryPresence><u>A&amp;lt;</u><u><![CDATA[A&amp;lt;]]></u><u>A&bogus;</u></QueryPresence>",output)==ResponseResult::response);CHECK(output==envelope(friend_xml("A&amp;lt;","Cenk &amp; &lt;Ω&gt;")+friend_xml("A&amp;amp;lt;","Cenk &amp; &lt;Ω&gt;")+friend_xml("A&amp;bogus;","Cenk &amp; &lt;Ω&gt;")));
    for(const char* command:{"<QueryPresence><u/></QueryPresence>","<QueryPresence><u></u></QueryPresence>","<QueryPresence><u><!--only--></u></QueryPresence>","<QueryPresence><u><first/>later</u></QueryPresence>","<QueryPresence><u><![CDATA[]]></u></QueryPresence>"})CHECK(render(*old,command,output)==ResponseResult::rejected && output.empty());
    CHECK(render(*old,"<QueryPresence><u>valid</u><u><first/>bad</u></QueryPresence>",output)==ResponseResult::rejected && output.empty());
    xml::Request request;CHECK(xml::parse("<LSX><Request id='p'><QueryPresence><u>7</u></QueryPresence></Request></LSX>",request)==xml::Result::ok);std::array<char,16384> out;out.fill('#');std::size_t n=99;CHECK(dispatch_response_with_config(request,*old,out.data(),8,&n)==ResponseResult::rejected && n==0 && out[0]=='#');CHECK(dispatch_response(request,out.data(),out.size(),&n)==ResponseResult::no_response && n==0);request.id={};CHECK(dispatch_response_with_config(request,*old,out.data(),out.size(),&n)==ResponseResult::rejected && n==0);
    const std::string one="<u>7</u>";std::string many="<QueryPresence>";for(int i=0;i<512;++i)many+=one;many+="</QueryPresence>";
    CHECK(render(*old,many,output,300000)==ResponseResult::response && friends(output,"Cenk & <Ω>")==512);CHECK(render(*old,many,output)==ResponseResult::rejected && output.empty());many.insert(many.size()-16,one);CHECK(render(*old,many,output,300000)==ResponseResult::no_response && output.empty());
    const auto saved=old;CHECK(store.replace(R"("Config2" {)")==ConfigResult::invalid && store.snapshot()==old);CHECK(store.replace(R"("Config2" {"User" {"Username" "new"}})")==ConfigResult::published);CHECK(render(*saved,"<QueryPresence><u>7</u></QueryPresence>",output)==ResponseResult::response && friends(output,"Cenk & <Ω>")==1);
    const char* a=R"("Config2" {"User" {"Username" "A"}})";const char* b=R"("Config2" {"User" {"Username" "B"}})";CHECK(store.replace(a)==ConfigResult::published);ConfigStore other;CHECK(other.replace(b)==ConfigResult::published);std::atomic<bool> bad=false;std::thread writer([&]{for(int i=0;i<600;++i)if(store.replace(i%2?a:b)!=ConfigResult::published)bad=true;});
    for(int i=0;i<600;++i){const auto snapshot=store.snapshot();std::string wire;if(render(*snapshot,"<QueryPresence><u>7</u><u>8</u></QueryPresence>",wire)!=ResponseResult::response || (friends(wire,"A")!=2 && friends(wire,"B")!=2))bad=true;if(render(*other.snapshot(),"<QueryPresence><u>9</u></QueryPresence>",wire)!=ResponseResult::response || friends(wire,"B")!=1)bad=true;}
    writer.join();CHECK(!bad);std::cout<<"QUERY_PRESENCE_DOM_FIRST_NODE_CDATA_BOUNDS_SNAPSHOT_PASS\n";
}
