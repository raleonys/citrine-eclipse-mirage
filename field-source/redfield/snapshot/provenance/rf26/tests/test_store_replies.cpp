// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/auth_code_session.hpp"
#include "preloader_l/config.hpp"
#include "preloader_l/response_dispatch.hpp"
#include "../third_party/tinyxml2/tinyxml2.h"
#include <array>
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
using namespace preloader_l;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
struct Route{const char* name;const char* body;};
const Route routes[]={{"QueryOffers","<QueryOffersResponse/>"},{"SelectStore","<ErrorSuccess Description=\"\" Code=\"0\"/>"},{"GetWalletBalance","<GetWalletBalanceResponse Balance=\"0\"/>"}};
bool parse_fixture(const Route& route,const std::string& payload,xml::Request& r){
    std::string input="<LSX><Request id='S&amp;lt;' recipient='Store&amp;X'><"+std::string(route.name)+payload+"</Request></LSX>";
    const auto status=xml::parse(input,r);input.assign(input.size(),'!');return status==xml::Result::ok;
}
std::string envelope(const Route& route){return " <LSX>\n  <Response id=\"S&amp;lt;\" sender=\"Store&amp;X\">\n    "+std::string(route.body)+"\n  </Response>\n </LSX>\n";}
int main(){
    ConfigStore store;const char* yes=R"("Config2" {"Emulator" {"Online" "true"} "Store" {"Id" "unexpected" "Balance" "999" "Currency" "XYZ"}})";const char* no=R"("Config2" {"Emulator" {"Online" "false"}})";
    CHECK(store.replace(yes)==ConfigResult::published);const auto held=store.snapshot();CHECK(store.replace(no)==ConfigResult::published);const auto offline=store.snapshot();
    AuthCodeSession auth;
    for(const auto& route:routes){
        const std::string payloads[]={"/>",std::string(" UserId='invalid' StoreId='not-a-store' Currency='XYZ' ClientId=''>ignored<Other><Nested/></Other></")+route.name+">"};
        for(const auto& payload:payloads){xml::Request r;CHECK(parse_fixture(route,payload,r));std::array<char,16384> out;out.fill('#');std::size_t n=99;
            CHECK(dispatch_response(r,out.data(),out.size(),&n)==ResponseResult::response && std::string(out.data(),n)==envelope(route));const auto need=n;CHECK(out[n]=='\0');
            tinyxml2::XMLDocument d;CHECK(d.Parse(out.data(),n)==tinyxml2::XML_SUCCESS);const auto* response=d.RootElement()->FirstChildElement("Response");CHECK(response && response->Attribute("id","S&lt;") && response->Attribute("sender","Store&X"));
            for(const auto& config:{held,offline}){
                CHECK(dispatch_response_with_config(r,*config,out.data(),out.size(),&n)==ResponseResult::response && std::string(out.data(),n)==envelope(route));
                CHECK(dispatch_response_with_session(r,*config,auth,out.data(),out.size(),&n)==ResponseResult::response && std::string(out.data(),n)==envelope(route) && !auth.ready());
            }
            out.fill('#');CHECK(dispatch_response(r,out.data(),8,&n)==ResponseResult::rejected && n==0 && out[0]=='#');CHECK(dispatch_response(r,out.data(),need,&n)==ResponseResult::rejected && n==0 && out[0]=='#');CHECK(dispatch_response(r,out.data(),need+1,&n)==ResponseResult::response && n==need && out[n]=='\0');
            CHECK(dispatch_response(r,nullptr,out.size(),&n)==ResponseResult::rejected && n==0);CHECK(dispatch_response(r,out.data(),0,&n)==ResponseResult::rejected && n==0);CHECK(dispatch_response(r,out.data(),out.size(),nullptr)==ResponseResult::rejected);
            const auto id=r.id;r.id={};out.fill('#');CHECK(dispatch_response(r,out.data(),out.size(),&n)==ResponseResult::rejected && n==0 && out[0]=='#');r.id=id;
            // Default sender has its own literal contract; empty recipient also uses it.
            r.recipient={};CHECK(dispatch_response(r,out.data(),out.size(),&n)==ResponseResult::response && std::string(out.data(),n).find("sender=\"EbisuSDK\"")!=std::string::npos);
        }
    }
    CHECK(store.snapshot()==offline && !offline->online && held->online && !auth.ready());
    // These replies cannot alter a previously committed Auth8 either.
    xml::Request request;CHECK(xml::parse("<LSX><Request id='a'><GetAuthCode ClientId=''/></Request></LSX>",request)==xml::Result::ok);std::array<char,16384> out{};std::size_t n=0;CHECK(dispatch_response_with_session(request,*held,auth,out.data(),out.size(),&n)==ResponseResult::response);const std::string first(out.data(),n);
    for(const auto& route:routes){xml::Request r;CHECK(parse_fixture(route,"/>",r));CHECK(dispatch_response_with_session(r,*held,auth,out.data(),out.size(),&n)==ResponseResult::response);}
    CHECK(dispatch_response_with_session(request,*held,auth,out.data(),out.size(),&n)==ResponseResult::response && std::string(out.data(),n)==first);
    // Wrong case and stateful achievement operations remain unbound.
    for(const char* command:{"queryoffers","selectstore","getwalletbalance","GrantAchievement"}){xml::Request r;const auto input="<LSX><Request id='x'><"+std::string(command)+"/></Request></LSX>";CHECK(xml::parse(input,r)==xml::Result::ok);CHECK(dispatch_response_with_session(r,*held,auth,out.data(),out.size(),&n)==ResponseResult::no_response && n==0);}
    std::atomic<bool> bad=false;std::vector<std::thread> threads;
    std::thread writer([&]{for(int i=0;i<120;++i)if(store.replace(i%2?yes:no)!=ConfigResult::published)bad=true;});
    for(int i=0;i<4;++i)threads.emplace_back([&]{for(int j=0;j<80;++j){const auto snapshot=store.snapshot();for(const auto& route:routes){xml::Request r;std::array<char,512> bytes{};std::size_t count=0;if(!parse_fixture(route,"/>",r) || dispatch_response_with_session(r,*snapshot,auth,bytes.data(),bytes.size(),&count)!=ResponseResult::response || std::string(bytes.data(),count)!=envelope(route))bad=true;}}});
    for(auto& t:threads){t.join();}writer.join();CHECK(!bad && held->online && auth.ready());auth.close();
    // A closed auth owner is irrelevant to pure replies, never reopened by them.
    xml::Request pure;CHECK(parse_fixture(routes[0],"/>",pure));CHECK(dispatch_response_with_session(pure,*held,auth,out.data(),out.size(),&n)==ResponseResult::response && !auth.ready());
    std::cout<<"STORE_PURE_REPLIES_LITERAL_WIRE_CONFIG_AUTH_ISOLATION_CAPACITY_PASS\n";
}
