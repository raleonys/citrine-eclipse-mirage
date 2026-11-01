// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/auth_code_session.hpp"
#include "redfield/config.hpp"
#include "../third_party/tinyxml2/tinyxml2.h"
#include <atomic>
#include <array>
#include <iostream>
#include <thread>
#include <vector>
using namespace redfield;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
struct Seeds {
    unsigned calls=0;bool fail=false;std::uint64_t value=0;
    static bool get(void* c,std::uint64_t& out) noexcept {auto& s=*static_cast<Seeds*>(c);++s.calls;out=s.value++;return !s.fail;}
};
bool parse_fixture(const std::string& command,xml::Request& r){
    std::string input="<LSX><Request id='A&amp;lt;' recipient='SDK&amp;X'>"+command+"</Request></LSX>";
    const auto status=xml::parse(input,r);input.assign(input.size(),'!');return status==xml::Result::ok;
}
ResponseResult render(AuthCodeSession& auth,const ConfigSnapshot& config,const std::string& command,std::string& wire){
    xml::Request r;if(!parse_fixture(command,r))return ResponseResult::rejected;
    std::array<char,16384> out{};std::size_t n=99;const auto result=dispatch_response_with_session(r,config,auth,out.data(),out.size(),&n);wire.assign(out.data(),n);return result;
}
std::string code(const std::string& wire){
    tinyxml2::XMLDocument d;if(d.Parse(wire.data(),wire.size())!=tinyxml2::XML_SUCCESS)return {};
    const auto* response=d.RootElement()->FirstChildElement("Response");if(!response || !response->Attribute("id","A&lt;") || !response->Attribute("sender","SDK&X"))return {};
    const auto* body=response->FirstChildElement("AuthCode");if(!body || !body->Attribute("value"))return {};return body->Attribute("value");
}
bool auth8(const std::string& token){return token.size()==8 && token.find_first_not_of("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz")==std::string::npos;}
int main(){
    ConfigStore store;CHECK(store.replace(R"("Config2" {"Emulator" {"Online" "true"}})")==ConfigResult::published);const auto yes=store.snapshot();
    CHECK(store.replace(R"("Config2" {"Emulator" {"Online" "false"}})")==ConfigResult::published);const auto no=store.snapshot();
    Seeds seeds;AuthCodeSession auth(Seeds::get,&seeds);std::string wire;
    CHECK(render(auth,*yes,"<GetAuthCode/>",wire)==ResponseResult::no_response && wire.empty() && seeds.calls==0 && !auth.ready());
    CHECK(render(auth,*no,"<GetAuthCode/>",wire)==ResponseResult::no_response && wire.empty());
    CHECK(render(auth,*no,"<GetAuthCode ClientId=''/>",wire)==ResponseResult::response);
    CHECK(wire==" <LSX>\n  <Response id=\"A&amp;lt;\" sender=\"SDK&amp;X\">\n    <ErrorSuccess Description=\"You need to be online to get an AuthCode.\" Code=\"-1577058293\"/>\n  </Response>\n </LSX>\n");
    CHECK(seeds.calls==0 && !auth.ready());
    xml::Request r;CHECK(parse_fixture("<GetAuthCode ClientId=''/>",r));std::array<char,16384> out;out.fill('#');std::size_t n=99;
    CHECK(auth.respond(r,*yes,nullptr,out.size(),&n)==ResponseResult::rejected && n==0 && seeds.calls==0);
    CHECK(auth.respond(r,*yes,out.data(),0,&n)==ResponseResult::rejected && n==0 && seeds.calls==0);
    CHECK(auth.respond(r,*yes,out.data(),out.size(),nullptr)==ResponseResult::rejected && seeds.calls==0);
    CHECK(dispatch_response(r,out.data(),out.size(),&n)==ResponseResult::no_response && n==0);
    CHECK(dispatch_response_with_config(r,*yes,out.data(),out.size(),&n)==ResponseResult::no_response && n==0);
    CHECK(auth.respond(r,*yes,out.data(),8,&n)==ResponseResult::rejected && n==0 && out[0]=='#' && !auth.ready() && seeds.calls==1);
    seeds.fail=true;CHECK(auth.respond(r,*yes,out.data(),out.size(),&n)==ResponseResult::rejected && n==0 && !auth.ready() && seeds.calls==2);seeds.fail=false;
    const auto saved_id=r.id;r.id={};CHECK(auth.respond(r,*yes,out.data(),out.size(),&n)==ResponseResult::rejected && n==0 && !auth.ready() && seeds.calls==2);r.id=saved_id;
    const auto saved_tag=r.command_open_tag;r.command_open_tag={"GetAuthCode ClientId=",21};CHECK(auth.respond(r,*yes,out.data(),out.size(),&n)==ResponseResult::rejected && n==0 && seeds.calls==2);r.command_open_tag=saved_tag;
    CHECK(render(auth,*yes,"<GetAuthCode ClientId='first'>ignored<Other/></GetAuthCode>",wire)==ResponseResult::response);const auto first=code(wire);CHECK(auth8(first) && auth.ready() && seeds.calls==3);
    CHECK(render(auth,*yes,"<GetAuthCode ClientId='second' UserId='different'/>",wire)==ResponseResult::response && code(wire)==first && seeds.calls==3);
    CHECK(render(auth,*yes,"<GetAuthCode ClientId=''/>",wire)==ResponseResult::response && code(wire)==first);
    CHECK(render(auth,*no,"<GetAuthCode ClientId=''/>",wire)==ResponseResult::response && wire.find("-1577058293")!=std::string::npos && auth.ready() && seeds.calls==3);
    CHECK(render(auth,*no,"<GoOnline/>",wire)==ResponseResult::response && !no->online);
    CHECK(auth.respond(r,*yes,out.data(),out.size(),&n)==ResponseResult::response);const auto need=n;out.fill('#');CHECK(auth.respond(r,*yes,out.data(),need,&n)==ResponseResult::rejected && n==0 && out[0]=='#' && seeds.calls==3);CHECK(auth.respond(r,*yes,out.data(),need+1,&n)==ResponseResult::response && n==need && out[n]=='\0');
    // A first unsuccessful exact-capacity render must also leave the owner uncommitted.
    Seeds capseeds;AuthCodeSession capauth(Seeds::get,&capseeds);out.fill('#');CHECK(capauth.respond(r,*yes,out.data(),need,&n)==ResponseResult::rejected && n==0 && out[0]=='#' && !capauth.ready());CHECK(capauth.respond(r,*yes,out.data(),need+1,&n)==ResponseResult::response && capauth.ready() && capseeds.calls==2);
    // Two independent owners produce their own deterministic first value, ClientId is not entropy.
    Seeds a,b;AuthCodeSession one(Seeds::get,&a),two(Seeds::get,&b);std::string other;
    CHECK(render(one,*yes,"<GetAuthCode ClientId='A'/>",wire)==ResponseResult::response && render(two,*yes,"<GetAuthCode ClientId='B'/>",other)==ResponseResult::response && code(wire)==code(other) && a.calls==1 && b.calls==1);
    const auto golden=code(wire);CHECK(golden=="lXiIk0EV");
    const char* cases[]={R"("Config2" {})",R"("Config2" {"Emulator" {"Online" "1"}})",R"("Config2" {"Emulator" {"Online" "0"}})",R"("Config2" {"Emulator" {"Online" "TRUE"}})",R"("Config2" {"Game" {"Online" "true"}})"};
    for(const auto* text:cases){CHECK(store.replace(text)==ConfigResult::published);CHECK(render(auth,*store.snapshot(),"<GetAuthCode ClientId=''/>",wire)==ResponseResult::response && wire.find("-1577058293")!=std::string::npos && seeds.calls==3);}
    CHECK(store.replace(R"("Config2" {)")==ConfigResult::invalid);CHECK(render(auth,*yes,"<GetAuthCode ClientId=''/>",wire)==ResponseResult::response && code(wire)==first);
    // Race first generation and all complete renders. A single candidate is committed.
    Seeds raced;AuthCodeSession concurrent(Seeds::get,&raced);std::atomic<bool> bad=false;std::vector<std::thread> threads;
    for(int i=0;i<8;++i)threads.emplace_back([&]{for(int j=0;j<60;++j){std::string x;if(render(concurrent,*yes,"<GetAuthCode ClientId=''/>",x)!=ResponseResult::response || code(x)!=golden)bad=true;}});
    for(auto& t:threads){t.join();}CHECK(!bad && raced.calls==1 && concurrent.ready());
    concurrent.close();concurrent.close();CHECK(!concurrent.ready() && render(concurrent,*yes,"<GetAuthCode ClientId=''/>",wire)==ResponseResult::rejected && wire.empty() && raced.calls==1);CHECK(render(concurrent,*no,"<GetAuthCode ClientId=''/>",wire)==ResponseResult::rejected && wire.empty());
    AuthCodeSession live;CHECK(render(live,*yes,"<GetAuthCode ClientId=''/>",wire)==ResponseResult::response && auth8(code(wire)));live.close();
    std::cout<<"AUTH8_SESSION_RENDER_COMMIT_ABORT_CONCURRENCY_SNAPSHOT_CLOSE_PASS\n";
}
