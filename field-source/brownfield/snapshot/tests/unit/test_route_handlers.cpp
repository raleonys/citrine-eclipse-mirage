#include "preloader_l/handlers/route_handlers.hpp"
#include "preloader_l/config/runtime_provider.hpp"
#include "preloader_l/config/settings_values.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include <cstring>
using namespace preloader_l;
static bool parse(const char* s, xml::RequestView* r){return xml::parse_request_view(s,std::strlen(s),r)==xml::RequestStatus::ok;}
static bool has(const char* s,const char* needle){return std::strstr(s,needle)!=nullptr;}
static bool auth8(const char* out){
 const char* p=std::strstr(out,"<AuthCode value=\""); if(!p)return false; p+=17;
 for(size_t i=0;i<8;++i){const char c=p[i]; if(!((c>='0'&&c<='9')||(c>='A'&&c<='Z')||(c>='a'&&c<='z')))return false;}
 return p[8]=='\"';
}
int main(){
 char out[4096]{}; size_t n=0; xml::RequestView r{};
 config::clear_runtime_overrides();
 const char* a="<LSX><Request id=\"1\"><GetGameInfo GameInfoId=\"LANGUAGES\"/></Request></LSX>";
 if(!parse(a,&r)||handlers::handle_get_game_info(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok||!has(out,"en_US")||!has(out,"zh_TW"))return 1;
 const char* b="<LSX><Request id=\"2\"><GetAllGameInfo/></Request></LSX>";
 if(!parse(b,&r)||handlers::handle_get_all_game_info(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok||!has(out,"InstalledVersion=\"1.0.0.0\"")||!has(out,"EntitlementSource=\"ORIGIN\""))return 2;
 const char* c="<LSX><Request id=\"3\"><GetProfile index=\"0\"/></Request></LSX>";
 if(!parse(c,&r)||handlers::handle_get_profile(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok||!has(out,"UserIndex=\"0\""))return 3;
 const char* c_attr="<LSX><Request id=\"3a\"><GetProfile index=\"4\"/></Request></LSX>";
 if(!parse(c_attr,&r)||handlers::handle_get_profile(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok||!has(out,"UserIndex=\"4\""))return 31;
 const char* c_missing="<LSX><Request id=\"3b\"><GetProfile/></Request></LSX>";
 n=9; if(!parse(c_missing,&r)||handlers::handle_get_profile(r,out,sizeof out,&n)!=handlers::HandlerStatus::unsupported||n!=0)return 32;
 const char* c_child="<LSX><Request id=\"3c\"><GetProfile><index>5</index></GetProfile></Request></LSX>";
 n=9; if(!parse(c_child,&r)||handlers::handle_get_profile(r,out,sizeof out,&n)!=handlers::HandlerStatus::unsupported||n!=0)return 33;
 const char* d="<LSX><Request id=\"4\"><GetPresence/></Request></LSX>";
 if(!parse(d,&r)||handlers::handle_get_presence(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok||!has(out,"Presence=\"INGAME\""))return 4;
 const char* e_missing="<LSX><Request id=\"5\"><GetAuthCode/></Request></LSX>";
 n=9;if(!parse(e_missing,&r)||handlers::handle_get_auth_code(r,out,sizeof out,&n)!=handlers::HandlerStatus::unsupported||n!=0)return 5;
 const char* e_child="<LSX><Request id=\"5a\"><GetAuthCode><ClientId>client</ClientId></GetAuthCode></Request></LSX>";
 n=9;if(!parse(e_child,&r)||handlers::handle_get_auth_code(r,out,sizeof out,&n)!=handlers::HandlerStatus::unsupported||n!=0)return 35;
 const char* e_empty="<LSX><Request id=\"5b\"><GetAuthCode ClientId=\"\"/></Request></LSX>";
 if(!parse(e_empty,&r)||handlers::handle_get_auth_code(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok||!auth8(out))return 11;
 const char* e="<LSX><Request id=\"5c\"><GetAuthCode ClientId=\"client\"/></Request></LSX>";
 if(!parse(e,&r)||handlers::handle_get_auth_code(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok||!auth8(out))return 36;
 const char* e_dup="<LSX><Request id=\"5d\"><GetAuthCode ClientId=\"a\" ClientId=\"b\"/></Request></LSX>";
 n=9;if(parse(e_dup,&r))return 37;
 const char* f_missing="<LSX><Request id=\"6m\"><GrantAchievement/></Request></LSX>";
 n=9;if(!parse(f_missing,&r)||handlers::handle_grant_achievement(r,out,sizeof out,&n)!=handlers::HandlerStatus::unsupported||n!=0)return 6;
 const char* f="<LSX><Request id=\"6\"><GrantAchievement><AchievementId>A1</AchievementId></GrantAchievement></Request></LSX>";
 if(!parse(f,&r)||handlers::handle_grant_achievement(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok||!has(out,"<Achievement Id=\"A1\" Total=\"1\" Count=\"1\" Progress=\"1\""))return 34;
 const char* g="<LSX><Request id=\"7\"><QueryContent/></Request></LSX>";
 if(!parse(g,&r)||handlers::handle_query_content(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok||!has(out,"<QueryContentResponse/>"))return 7;
 config::set_runtime_online_override(config::literal_value("false"));
 if(!parse(e,&r)||handlers::handle_get_auth_code(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok||!has(out,"-1577058293"))return 9;
 if(!parse(f,&r)||handlers::handle_grant_achievement(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok||!has(out,"<Achievement Id=\"A1\""))return 10;
 return 0;
}
