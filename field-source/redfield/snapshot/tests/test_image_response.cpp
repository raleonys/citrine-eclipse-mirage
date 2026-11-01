// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/image_response.hpp"
#include "../third_party/tinyxml2/tinyxml2.h"
#include <array>
#include <atomic>
#include <iostream>
#include <thread>
using namespace preloader_l;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
const std::string command="<QueryImage><Width> -417 </Width><Height>nonnumeric</Height><ImageId>avatar:42</ImageId></QueryImage>";
ResponseResult render(const ConfigSnapshot& s,const std::string& body,std::string& wire,std::size_t cap=16384){
    xml::Request r;auto source="<LSX><Request id='I&amp;lt;' recipient='SDK&amp;X'>"+body+"</Request></LSX>";
    if(xml::parse(source,r)!=xml::Result::ok)return ResponseResult::rejected;
    source.assign(source.size(),'!');std::vector<char> out(cap,'#');std::size_t n=99;
    const auto result=dispatch_response_with_config(r,s,out.data(),out.size(),&n);wire.assign(out.data(),n);return result;
}
bool attributes(const std::string& wire,const std::string& resource,const char* width="-417",const char* height="nonnumeric",const char* id="avatar:42"){
    tinyxml2::XMLDocument d;if(d.Parse(wire.data(),wire.size())!=tinyxml2::XML_SUCCESS)return false;
    const auto* r=d.RootElement()->FirstChildElement("Response");if(!r || !r->Attribute("id","I&lt;") || !r->Attribute("sender","SDK&X"))return false;
    const auto* q=r->FirstChildElement("QueryImageResponse");if(!q || !q->Attribute("Result","0"))return false;
    const auto* i=q->FirstChildElement("Image");return i && i->Attribute("ResourcePath",resource.c_str()) && i->Attribute("Width",width) && i->Attribute("Height",height) && i->Attribute("ImageId",id);
}
int main(){
    ConfigStore store("/DLL Ω & dir/surge.png");const auto initial=store.snapshot();std::string wire;
    CHECK(render(*initial,command,wire)==ResponseResult::response && attributes(wire,initial->avatar_fallback_path));
    CHECK(wire==" <LSX>\n  <Response id=\"I&amp;lt;\" sender=\"SDK&amp;X\">\n    <QueryImageResponse Result=\"0\">\n     <Image ImageId=\"avatar:42\" Height=\"nonnumeric\" ResourcePath=\"/DLL Ω &amp; dir/surge.png\" Width=\"-417\"/>\n    </QueryImageResponse>\n  </Response>\n </LSX>\n");
    CHECK(store.replace(R"("Config2" {})")==ConfigResult::published);CHECK(store.snapshot()->avatar_fallback_path==initial->avatar_fallback_path);
    CHECK(render(*store.snapshot(),command,wire)==ResponseResult::response && attributes(wire,initial->avatar_fallback_path));
    CHECK(store.replace(R"("Config2" {"User" {"Avatar" "old" "Avatar" "relative & <Ω>.png" "Nested" {"Avatar" "wrong"}} "Game" {"Avatar" "wrong"} "Avatar" "wrong"})")==ConfigResult::published);
    const auto old=store.snapshot();CHECK(render(*old,command,wire)==ResponseResult::response && attributes(wire,"relative & <Ω>.png"));
    CHECK(render(*old,"<QueryImage><Width><![CDATA[ &lt; ]]></Width><Height> +2147483648 </Height><ImageId>A&amp;lt;:x&amp;y</ImageId></QueryImage>",wire)==ResponseResult::response && attributes(wire,"relative & <Ω>.png","&lt;","+2147483648","A&lt;:x&y"));
    CHECK(render(*old,"<QueryImage><!--ok--><Width/><Height/><ImageId><![CDATA[A&amp;lt;:]]></ImageId><Width>ignored</Width></QueryImage>",wire)==ResponseResult::response && attributes(wire,"relative & <Ω>.png","","","A&amp;lt;:"));
    CHECK(render(*old,"<QueryImage><Width>W&bogus;</Width><Height>H&amp;lt;</Height><ImageId/></QueryImage>",wire)==ResponseResult::response && attributes(wire,"relative & <Ω>.png","W&bogus;","H&lt;",""));
    for(const char* q:{"<QueryImage/>","<QueryImage Width='1' Height='2' ImageId='x'/>","<QueryImage><Nested><Width>1</Width></Nested><Height>2</Height><ImageId>x</ImageId></QueryImage>"})CHECK(render(*old,q,wire)==ResponseResult::no_response && wire.empty());
    for(const char* q:{"<QueryImage><Width><!--c-->1</Width><Height>2</Height><ImageId>x</ImageId></QueryImage>","<QueryImage><Width>1<tail/></Width><Height>2</Height><ImageId>x</ImageId></QueryImage>","<QueryImage>bad<Width>1</Width><Height>2</Height><ImageId>x</ImageId></QueryImage>"})CHECK(render(*old,q,wire)==ResponseResult::rejected && wire.empty());
    CHECK(store.replace(R"("Config2" {)")==ConfigResult::invalid && store.snapshot()==old);
    CHECK(store.replace(R"("Config2" {"User" {"Avatar" ""}})")==ConfigResult::published);CHECK(render(*store.snapshot(),command,wire)==ResponseResult::response && attributes(wire,""));
    CHECK(render(*old,command,wire)==ResponseResult::response && attributes(wire,"relative & <Ω>.png"));
    xml::Request r;CHECK(xml::parse("<LSX><Request id='i'>"+command+"</Request></LSX>",r)==xml::Result::ok);
    std::array<char,16384> out;out.fill('#');std::size_t n=99;CHECK(dispatch_response_with_config(r,*old,out.data(),8,&n)==ResponseResult::rejected && n==0 && out[0]=='#');
    CHECK(dispatch_response(r,out.data(),out.size(),&n)==ResponseResult::no_response && n==0);r.id={};CHECK(dispatch_response_with_config(r,*old,out.data(),out.size(),&n)==ResponseResult::rejected && n==0);
    ConfigStore no_context;CHECK(render(*no_context.snapshot(),command,wire)==ResponseResult::rejected && wire.empty());
    CHECK(no_context.replace(R"("Config2" {"User" {"Avatar" ""}})")==ConfigResult::published);CHECK(render(*no_context.snapshot(),command,wire)==ResponseResult::response && attributes(wire,""));
    ConfigSnapshot invalid=*old;invalid.leaves.back()={{"User"},"Avatar",std::string(1,'\1')};CHECK(render(invalid,command,wire)==ResponseResult::rejected && wire.empty());
    CHECK(store.replace("\"Config2\" {\"User\" {\"Avatar\" \""+std::string(17000,'x')+"\"}}")==ConfigResult::published);CHECK(render(*store.snapshot(),command,wire)==ResponseResult::rejected && wire.empty());
    const char* a=R"("Config2" {"User" {"Avatar" "A"}})";const char* b=R"("Config2" {"User" {"Avatar" "B"}})";
    CHECK(store.replace(a)==ConfigResult::published);std::atomic<bool> bad=false;std::thread writer([&]{for(int i=0;i<600;++i)if(store.replace(i%2?a:b)!=ConfigResult::published)bad=true;});
    for(int i=0;i<600;++i){const auto s=store.snapshot();if(s->avatar_fallback_path!=initial->avatar_fallback_path || render(*s,command,wire)!=ResponseResult::response || (!attributes(wire,"A") && !attributes(wire,"B")))bad=true;}
    writer.join();CHECK(!bad);std::cout<<"IMAGE_DOM_PROVIDER_DISABLED_SEMANTICS_SNAPSHOT_CAPACITY_PASS\n";
}
