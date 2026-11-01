// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/game_info_response.hpp"
#include "redfield/profile_response.hpp"
#include "redfield/settings_response.hpp"
#include "utc_test_support.hpp"
#include "../third_party/tinyxml2/tinyxml2.h"
#include <array>
#include <atomic>
#include <iostream>
#include <thread>
using namespace redfield;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
std::string envelope(const std::string& body){return " <LSX>\n  <Response id=\"I&amp;lt;\" sender=\"SDK&amp;X\">\n    "+body+"\n  </Response>\n </LSX>\n";}
ResponseResult render(const ConfigSnapshot& snapshot,const std::string& command,std::string& output){
    xml::Request r;if(xml::parse("<LSX><Request id='I&amp;lt;' recipient='SDK&amp;X'>"+command+"</Request></LSX>",r)!=xml::Result::ok)return ResponseResult::rejected;
    std::array<char,16384> buffer;std::size_t n=0;const auto result=dispatch_response_with_config(r,snapshot,buffer.data(),buffer.size(),&n);output.assign(buffer.data(),n);return result;
}
std::string attribute(const std::string& output,const char* name){tinyxml2::XMLDocument d;if(d.Parse(output.data(),output.size())!=tinyxml2::XML_SUCCESS)return "!INVALID_XML";const auto* a=d.RootElement()->FirstChildElement()->FirstChildElement()->Attribute(name);return a?a:"!MISSING_ATTRIBUTE";}
int main(){
    ConfigStore store;const auto initial=store.snapshot();
    constexpr const char* languages="ar_SA,cs_CZ,da_DK,de_DE,el_GR,en_AU,en_CA,en_GB,en_US,es_ES,es_MX,fi_FI,fr_CA,fr_FR,hu_HU,it_IT,ja_JP,ko_KR,nl_NL,no_NO,pl_PL,pt_BR,pt_PT,ru_RU,sv_SE,th_TH,tr_TR,zh_CN,zh_HK,zh_TW";
    struct Case{const char* key;const char* expected;};const Case cases[]={{"DISPLAY_NAME","Game"},{"INSTALLED_VERSION","1.0.0.0"},{"AVAILABLE_VERSION","1.0.0.0"},{"UPTODATE","true"},{"FULLGAME_IS_RELEASED","true"},{"FULLGAME_RELEASE_DATE","2010-01-01T00:00:00"},{"FULLGAME_PURCHASED","true"},{"FREETRIAL","false"},{"EXPIRATION","0000-00-00T00:00:00"},{"ENTITLEMENT_SOURCE","ORIGIN"},{"MAX_GROUP_SIZE","16"},{"EXPIRATION_DURATION","36000000"},{"LANGUAGES",languages},{"INSTALLED_LANGUAGE","en_US"}};
    std::string output;
    for(const auto& c:cases){CHECK(render(*initial,std::string("<GetGameInfo GameInfoId='")+c.key+"'/>",output)==ResponseResult::response);CHECK(output==envelope(std::string("<GetGameInfoResponse GameInfo=\"")+c.expected+"\"/>"));}
    CHECK(store.replace(R"("Config2" {})")==ConfigResult::published);for(const auto& c:cases){CHECK(render(*store.snapshot(),std::string("<GetGameInfo GameInfoId='")+c.key+"'/>",output)==ResponseResult::response);CHECK(attribute(output,"GameInfo")==c.expected);}
    CHECK(store.replace(R"("Config2" {"Game" {"Name" "Old" "GameName" "Neon & <Ω>" "Version" "1" "GameVersion" "2 &lt;" "Languages" "tr_TR,en_US" "Language" "tr_TR" "UPTODATE" "false" "AVAILABLE_VERSION" "wrong" "Nested" {"Language" "wrong"}} "User" {"GameName" "wrong" "Language" "wrong"} "Unknown" {"Languages" "wrong"}})")==ConfigResult::published);
    const auto old=store.snapshot();
    for(const auto& c:cases){CHECK(render(*old,std::string("<GetGameInfo GameInfoId='")+c.key+"'/>",output)==ResponseResult::response);const std::string expected=std::string(c.key)=="DISPLAY_NAME"?"Neon & <Ω>":std::string(c.key)=="INSTALLED_VERSION"||std::string(c.key)=="AVAILABLE_VERSION"?"2 &lt;":std::string(c.key)=="LANGUAGES"?"tr_TR,en_US":std::string(c.key)=="INSTALLED_LANGUAGE"?"tr_TR":c.expected;CHECK(attribute(output,"GameInfo")==expected);}
    CHECK(render(*old,"<GetGameInfo GameInfoId='DISPLAY_&#78;AME'/>",output)==ResponseResult::response);CHECK(output==envelope("<GetGameInfoResponse GameInfo=\"Neon &amp; &lt;Ω&gt;\"/>"));
    for(const char* command:{"<GetGameInfo/>","<GetGameInfo GameInfoId=''/>","<GetGameInfo GameInfoId='UNKNOWN'/>","<GetGameInfo GameInfoId='display_name'/>","<GetGameInfo><GameInfoId>DISPLAY_NAME</GameInfoId></GetGameInfo>","<GetGameInfo GameInfoId='DISPLAY_&amp;#78;AME'/>"}){CHECK(render(*old,command,output)==ResponseResult::no_response && output.empty());}
    const auto before=utc_wall_seconds();CHECK(render(*old,"<GetAllGameInfo/>",output)==ResponseResult::response);const auto time=attribute(output,"SystemTime");CHECK(time>=utc_text(before) && time<=utc_text(utc_wall_seconds()));
    const auto exact=envelope("<GetAllGameInfoResponse FreeTrial=\"false\" FullGameReleaseDate=\"2010-01-01T00:00:00\" Languages=\"tr_TR,en_US\" UpToDate=\"true\" FullGamePurchased=\"true\" MaxGroupSize=\"16\" HasExpiration=\"false\" InstalledVersion=\"2 &amp;lt;\" InstalledLanguage=\"tr_TR\" SystemTime=\""+time+"\" DisplayName=\"Neon &amp; &lt;Ω&gt;\" FullGameReleased=\"true\" Expiration=\"0000-00-00T00:00:00\" AvailableVersion=\"2 &amp;lt;\" EntitlementSource=\"ORIGIN\"/>");CHECK(output==exact);
    const auto owned=output;CHECK(render(*old,"<GetSetting SettingId='LANGUAGE'/>",output)==ResponseResult::response);CHECK(attribute(output,"Setting")=="tr_TR");CHECK(render(*old,"<GetPresence/>",output)==ResponseResult::response);CHECK(attribute(output,"RichPresence")=="Neon & <Ω>");
    CHECK(store.replace(R"("Config2" {)")==ConfigResult::invalid && store.snapshot()==old);CHECK(store.replace(R"("Config2" {"Game" {"GameName" "first" "Name" "last" "GameVersion" "first" "Version" "last" "Language" "" "Languages" ""}})")==ConfigResult::published);
    CHECK(render(*store.snapshot(),"<GetGameInfo GameInfoId='DISPLAY_NAME'/>",output)==ResponseResult::response && attribute(output,"GameInfo")=="last");CHECK(render(*store.snapshot(),"<GetGameInfo GameInfoId='AVAILABLE_VERSION'/>",output)==ResponseResult::response && attribute(output,"GameInfo")=="last");CHECK(render(*store.snapshot(),"<GetGameInfo GameInfoId='INSTALLED_LANGUAGE'/>",output)==ResponseResult::response && attribute(output,"GameInfo").empty());CHECK(owned.find("Neon &amp;")!=std::string::npos);
    xml::Request request;CHECK(xml::parse("<LSX><Request id='x'><GetAllGameInfo/></Request></LSX>",request)==xml::Result::ok);std::array<char,16384> out;out.fill('#');std::size_t n=99;CHECK(dispatch_response_with_config(request,*old,out.data(),8,&n)==ResponseResult::rejected && n==0 && out[0]=='#');CHECK(dispatch_response(request,out.data(),out.size(),&n)==ResponseResult::no_response && n==0);request.id={};CHECK(dispatch_response_with_config(request,*old,out.data(),out.size(),&n)==ResponseResult::rejected && n==0);
    CHECK(store.replace(std::string("\"Config2\" {\"Game\" {\"Name\" \"")+std::string(20000,'X')+"\"}}")==ConfigResult::published);CHECK(render(*store.snapshot(),"<GetAllGameInfo/>",output)==ResponseResult::rejected && output.empty());
    const char* a=R"("Config2" {"Game" {"Name" "A" "Version" "A" "Language" "A" "Languages" "A"}})";const char* b=R"("Config2" {"Game" {"Name" "B" "Version" "B" "Language" "B" "Languages" "B"}})";CHECK(store.replace(a)==ConfigResult::published);ConfigStore other;CHECK(other.replace(b)==ConfigResult::published);std::atomic<bool> bad=false;
    std::thread writer([&]{for(int i=0;i<600;++i)if(store.replace(i%2?a:b)!=ConfigResult::published)bad=true;});
    for(int i=0;i<600;++i){auto snapshot=store.snapshot();std::string wire;if(render(*snapshot,"<GetAllGameInfo/>",wire)!=ResponseResult::response){bad=true;continue;}const auto name=attribute(wire,"DisplayName");if(name!=attribute(wire,"InstalledVersion") || name!=attribute(wire,"AvailableVersion") || name!=attribute(wire,"InstalledLanguage") || name!=attribute(wire,"Languages"))bad=true;if(render(*other.snapshot(),"<GetGameInfo GameInfoId='DISPLAY_NAME'/>",wire)!=ResponseResult::response || attribute(wire,"GameInfo")!="B")bad=true;}
    writer.join();CHECK(!bad);CHECK(render(*old,"<GetGameInfo GameInfoId='DISPLAY_NAME'/>",output)==ResponseResult::response && attribute(output,"GameInfo")=="Neon & <Ω>");
    std::cout<<"GAME_INFO_14_SELECTORS_AGGREGATE_ENTITY_SNAPSHOT_PASS\n";
}
