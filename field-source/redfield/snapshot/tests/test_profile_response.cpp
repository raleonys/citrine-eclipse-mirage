// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/profile_response.hpp"
#include "../third_party/tinyxml2/tinyxml2.h"
#include <array>
#include <atomic>
#include <cstring>
#include <fstream>
#include <iterator>
#include <iostream>
#include <thread>
using namespace preloader_l;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
std::string_view view(ValueView v){return {v.data,v.size};}
bool render(const ConfigSnapshot& config,const char* command,std::string& result){
    xml::Request request;const auto input=std::string("<LSX><Request id='p&amp;lt;' recipient='SDK&amp;X'>")+command+"</Request></LSX>";
    if(xml::parse(input,request)!=xml::Result::ok)return false;
    std::array<char,16384> out;std::size_t n=0;
    if(dispatch_response_with_config(request,config,out.data(),out.size(),&n)!=ResponseResult::response)return false;
    result.assign(out.data(),n);return out[n]=='\0';
}
int main(int argc,char** argv){
    ConfigStore store;auto initial=store.snapshot();CHECK(!initial->overlay_loaded);
    const auto defaults=profile_policy(*initial);CHECK(view(defaults.username)=="SURGE" && view(defaults.user_id)=="1000200030000" && view(defaults.is_subscriber)=="true");
    std::string output;CHECK(render(*initial,"<GetProfile index='A&amp;lt;'/>",output));
    const std::string exact=" <LSX>\n  <Response id=\"p&amp;lt;\" sender=\"SDK&amp;X\">\n    <GetProfileResponse Country=\"US\" GeoCountry=\"US\" PersonaId=\"1144668899\" IsTrialSubscriber=\"false\" SubscriberLevel=\"2\" Persona=\"SURGE\" UserId=\"1000200030000\" CommerceCountry=\"US\" CommerceCurrency=\"USD\" AvatarId=\"\" IsSubscriber=\"true\" UserIndex=\"A&amp;lt;\" IsUnderAge=\"false\" IsSteamSubscriber=\"false\"/>\n  </Response>\n </LSX>\n";
    CHECK(output==exact);
    CHECK(store.replace(R"("Config2" {})")==ConfigResult::published);
    auto empty=store.snapshot();CHECK(empty->overlay_loaded && view(profile_policy(*empty).username).empty());
    CHECK(render(*empty,"<GetProfile index=''/>",output));CHECK(output.find("Persona=\"\"")!=std::string::npos && output.find("UserIndex=\"\"")!=std::string::npos);
    CHECK(view(profile_policy(*initial).username)=="SURGE");
    CHECK(store.replace(R"("Config2" { "User" {"Username" "old" "Username" "Cenk &lt; Ω" "Country" "TR" "Currency" "TRY" "PersonaId" "9" "UserId" "7" "IsSubscriber" "false" "IsSteamSubscriber" "true" "SubscriberLevel" "4" "IsUnderAge" "false"} "Game" {"Username" "wrong" "Name" "Old" "GameName" "Neon & <Field>"} "Other" {"Username" "wrong"} })")==ConfigResult::published);
    auto old=store.snapshot();CHECK(view(profile_policy(*old).username)=="Cenk &lt; Ω");
    CHECK(view(presence_title(*old))=="Neon & <Field>");
    CHECK(render(*old,"<GetProfile index='&#50;'/>",output));
    CHECK(output.find("Persona=\"Cenk &amp;lt; Ω\"")!=std::string::npos && output.find("UserIndex=\"2\"")!=std::string::npos);
    tinyxml2::XMLDocument doc;CHECK(doc.Parse(output.data(),output.size())==tinyxml2::XML_SUCCESS);
    const auto* response=doc.RootElement()->FirstChildElement()->FirstChildElement();CHECK(response && std::strcmp(response->Attribute("Persona"),"Cenk &lt; Ω")==0 && std::strcmp(response->Attribute("CommerceCountry"),"TR")==0);
    CHECK(render(*old,"<GetPresence/>",output));
    const std::string presence=" <LSX>\n  <Response id=\"p&amp;lt;\" sender=\"SDK&amp;X\">\n    <GetPresenceResponse Title=\"\" SessionId=\"\" MultiplayerId=\"\" GroupId=\"\" TitleId=\"\" Presence=\"INGAME\" GamePresence=\"\" Group=\"\" RichPresence=\"Neon &amp; &lt;Field&gt;\" UserId=\"0\"/>\n  </Response>\n </LSX>\n";
    CHECK(output==presence);const auto owned=output;
    CHECK(store.replace(R"("Config2" {)")==ConfigResult::invalid && store.snapshot()==old);
    CHECK(store.replace(R"("Config2" {"Game" {"Name" "New"}})")==ConfigResult::published);
    CHECK(view(presence_title(*store.snapshot()))=="New" && view(presence_title(*old))=="Neon & <Field>" && output==owned);
    xml::Request request;std::size_t n=99;std::array<char,16384> out;
    for(const auto* command:{"<GetProfile/>","<GetProfile><index>0</index></GetProfile>","<GetProfile Index='0'/>"}){
        CHECK(xml::parse(std::string("<LSX><Request id='x'>")+command+"</Request></LSX>",request)==xml::Result::ok);
        CHECK(dispatch_response_with_config(request,*old,out.data(),out.size(),&n)==ResponseResult::no_response && n==0);
    }
    CHECK(xml::parse("<LSX><Request id='x'><GetProfile index='0'/></Request></LSX>",request)==xml::Result::ok);
    out.fill('#');CHECK(dispatch_response_with_config(request,*old,out.data(),8,&n)==ResponseResult::rejected && n==0 && out[0]=='#');
    CHECK(dispatch_response(request,out.data(),out.size(),&n)==ResponseResult::no_response && n==0);
    request.id={};CHECK(dispatch_response_with_config(request,*old,out.data(),out.size(),&n)==ResponseResult::rejected && n==0);
    const auto large=std::string("\"Config2\" {\"User\" {\"Username\" \"")+std::string(20000,'A')+"\"}}";
    CHECK(store.replace(large)==ConfigResult::published);CHECK(xml::parse("<LSX><Request id='x'><GetProfile index='0'/></Request></LSX>",request)==xml::Result::ok);
    CHECK(dispatch_response_with_config(request,*store.snapshot(),out.data(),out.size(),&n)==ResponseResult::rejected && n==0);
    // A request pins one snapshot. Concurrent publication cannot mix profile
    // and title values, and separate stores never share process-global policy.
    const char* a=R"("Config2" {"User" {"Username" "A" "Country" "A"} "Game" {"Name" "A"}})";
    const char* b=R"("Config2" {"User" {"Username" "B" "Country" "B"} "Game" {"Name" "B"}})";
    CHECK(store.replace(a)==ConfigResult::published);ConfigStore independent;CHECK(independent.replace(b)==ConfigResult::published);
    std::atomic<bool> bad=false;
    std::thread writer([&]{for(int i=0;i<1000;++i)if(store.replace(i%2?a:b)!=ConfigResult::published)bad=true;});
    for(int i=0;i<1000;++i){const auto pinned=store.snapshot();const auto p=profile_policy(*pinned);if(view(p.username)!=view(p.country) || view(p.username)!=view(presence_title(*pinned)))bad=true;std::string rendered;if(!render(*pinned,"<GetProfile index='0'/>",rendered) || rendered.find("Persona=\""+std::string(view(p.username))+"\"")==std::string::npos)bad=true;if(view(profile_policy(*independent.snapshot()).username)!="B")bad=true;}
    writer.join();CHECK(!bad);
    // Exercise the distributed example itself. A valid file with omitted User
    // fields intentionally suppresses constructor defaults,so a partial sample
    // can produce an empty profile even though GetConfig succeeds.
    CHECK(argc==2);
    std::ifstream sample(argv[1],std::ios::binary);CHECK(sample.is_open());
    const std::string sample_text{std::istreambuf_iterator<char>(sample),std::istreambuf_iterator<char>()};
    CHECK(!sample.bad());ConfigStore example;
    CHECK(example.replace(sample_text)==ConfigResult::published);
    const auto example_snapshot=example.snapshot();const auto example_profile=profile_policy(*example_snapshot);
    CHECK(view(example_profile.username)=="SURGE" && view(example_profile.persona_id)=="1144668899" &&
          view(example_profile.user_id)=="1000200030000" && view(example_profile.country)=="US" &&
          view(example_profile.currency)=="USD" && view(example_profile.is_subscriber)=="true" &&
          view(example_profile.is_steam_subscriber)=="false" && view(example_profile.subscriber_level)=="2" &&
          view(example_profile.is_under_age)=="false");
    CHECK(render(*example_snapshot,"<GetProfile index='0'/>",output));
    CHECK(output.find("Persona=\"SURGE\"")!=std::string::npos && output.find("UserId=\"1000200030000\"")!=std::string::npos);
    std::cout<<"PROFILE_DEFAULT_OVERLAY_ENTITY_SNAPSHOT_ISOLATION_PASS\n";
}
