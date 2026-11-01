#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/config/runtime_provider.hpp"
#include "preloader_l/config/settings_provider.hpp"
#include "preloader_l/config/settings_values.hpp"
#include "preloader_l/handlers/route_handlers.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include <cstring>
using namespace preloader_l;
static bool parse(const char*s,xml::RequestView*r){return xml::parse_request_view(s,std::strlen(s),r)==xml::RequestStatus::ok;}
static bool has(const char*s,const char*n){return std::strstr(s,n)!=nullptr;}
int main(){
 config::clear_runtime_overrides();config::clear_runtime_configuration();
 constexpr char cfg[]=R"CFG("Config2" {
  "Game" { "Name" "Demo Game" "Version" "9.9.9" "Language" "tr_TR" }
  "Emulator" { "Online" "true" "Environment" "staging" }
  "User" { "Username" "PLAYER" "UserId" "42" "PersonaId" "77" "Country" "TR" "Avatar" "avatar.png" "AuthCode" "AUTH42" }
 })CFG";
 if(config::set_runtime_configuration_cfg(cfg,sizeof(cfg)-1)!=config::RuntimeConfigurationStatus::ok)return 1;
 if(config::runtime_configuration_present_sections()!=7u)return 2;
 auto lang=config::runtime_configuration_setting("Language");if(!lang.present||lang.size!=5||std::memcmp(lang.data,"tr_TR",5)!=0)return 3;
 config::ValueView online{};if(!config::lookup_setting(config::runtime_settings_provider(),config::SettingKey::online,&online)||!config::value_equals(online,"true"))return 4;
 if(config::set_runtime_current_directory("C:\\Game",7)!=config::RuntimeConfigurationStatus::ok)return 5;
 char out[8192]{};size_t n=0;xml::RequestView r{};
 const char*q1="<LSX><Request id=\"1\"><QueryContent/></Request></LSX>";
 if(!parse(q1,&r)||handlers::handle_query_content(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok||!has(out,"<QueryContentResponse"))return 6;
 const char*q2="<LSX><Request id=\"2\"><QueryEntitlements/></Request></LSX>";
 if(!parse(q2,&r)||handlers::handle_query_entitlements(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok||!has(out,"<QueryEntitlementsResponse"))return 7;
 const char*q3="<LSX><Request id=\"3\"><QueryOffers/></Request></LSX>";
 if(!parse(q3,&r)||handlers::handle_query_offers(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok||!has(out,"<QueryOffersResponse/>")||has(out,"Unknown Error"))return 8;
 const char*q4="<LSX><Request id=\"4\"><QueryAchievements/></Request></LSX>";
 if(!parse(q4,&r)||handlers::handle_query_achievements(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok||!has(out,"<AchievementSets>")||!has(out,"GameName=\"Demo Game\"")||has(out,"-1576468080"))return 9;
 const char*q5="<LSX><Request id=\"5\"><QueryImage><Width>2&#48;8</Width><Height>20&#56;</Height><ImageId>user&#58;42</ImageId></QueryImage></Request></LSX>";
 if(!parse(q5,&r)||handlers::handle_query_image(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok||!has(out,"ImageId=\"user:42\"")||!has(out,"ResourcePath=\"avatar.png\"")||!has(out,"Width=\"208\""))return 10;
 const char*q5c="<LSX><Request id=\"5c\"><QueryImage><!--edge--><Width><![CDATA[208]]></Width><!--between--><Height><![CDATA[208]]></Height><ImageId><![CDATA[user:42]]></ImageId></QueryImage></Request></LSX>";
 if(!parse(q5c,&r)||handlers::handle_query_image(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok||!has(out,"ImageId=\"user:42\"")||!has(out,"Width=\"208\"")||!has(out,"Height=\"208\""))return 15;
 const char*q5m="<LSX><Request id=\"5m\"><QueryImage><Width>2&bogus;8</Width><Height>208</Height><ImageId>user:42</ImageId></QueryImage></Request></LSX>";
 // A02-B: stock TinyXML2 accepts unknown named references as literal text;
 // keep the response XML escaped and never reinterpret the Width as a valid 208.
 if(!parse(q5m,&r)||handlers::handle_query_image(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok||
    !has(out,"Width=\"2&amp;bogus;8\"") || has(out,"Width=\"208\""))return 14;
 const char*q5b="<LSX><Request id=\"5b\"><QueryImage><Width>not-a-number</Width><Height>64</Height><ImageId>plain-id</ImageId></QueryImage></Request></LSX>";
 if(!parse(q5b,&r)||handlers::handle_query_image(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok||!has(out,"ImageId=\"plain-id\"")||!has(out,"ResourcePath=\"avatar.png\"")||!has(out,"Width=\"not-a-number\""))return 13;
 const char*q6="<LSX><Request id=\"6\"><GetAllGameInfo/></Request></LSX>";
 if(!parse(q6,&r)||handlers::handle_get_all_game_info(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok||!has(out,"InstalledVersion=\"9.9.9\"")||!has(out,"InstalledLanguage=\"tr_TR\"")||!has(out,"DisplayName=\"Demo Game\""))return 11;
 const char*q7="<LSX><Request id=\"7\"><GetProfile index=\"0\"/></Request></LSX>";
 if(!parse(q7,&r)||handlers::handle_get_profile(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok||!has(out,"Persona=\"PLAYER\"")||!has(out,"UserId=\"42\"")||!has(out,"Country=\"TR\""))return 12;
 config::clear_runtime_configuration();return 0;
}
