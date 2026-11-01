#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/config/runtime_provider.hpp"
#include "preloader_l/core/request_dispatch.hpp"
#include "preloader_l/handlers/route_handlers.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include <cstring>
using namespace preloader_l;
static bool has(const char*s,const char*n){return s&&std::strstr(s,n)!=nullptr;}
static bool parse(const char*s,xml::RequestView*r){return xml::parse_request_view(s,std::strlen(s),r)==xml::RequestStatus::ok;}
using Fn=handlers::HandlerStatus(*)(const xml::RequestView&,char*,size_t,size_t*) noexcept;
static bool direct(const char*q,Fn fn,const char*needle){
 char out[4096]{};size_t n=0;xml::RequestView r{};
 return parse(q,&r)&&fn(r,out,sizeof out,&n)==handlers::HandlerStatus::ok&&n>0&&has(out,needle);
}
static bool dispatch_roundtrip(const char*q,const char*needle){
 const auto measured=core::dispatch_plaintext_request(q,std::strlen(q),nullptr,0);
 if(measured.status!=core::DispatchStatus::capacity||measured.written==0)return false;
 char out[4096]{};
 const auto rendered=core::dispatch_plaintext_request(q,std::strlen(q),out,sizeof out);
 return rendered.status==core::DispatchStatus::ok&&rendered.written==measured.written&&has(out,needle);
}
int main(){
 config::clear_runtime_overrides();config::clear_runtime_configuration();
 if(config::runtime_configuration_loaded())return 1;
 preloader_l::config::ValueView online=preloader_l::config::missing_value();
 const auto& provider=preloader_l::config::runtime_settings_provider();
 (void)preloader_l::config::lookup_setting(provider,preloader_l::config::SettingKey::online,&online);
 if(!preloader_l::config::online_enabled(online))return 2;
 constexpr char qc[]="<LSX><Request id=\"c\"><QueryContent/></Request></LSX>";
 constexpr char qe[]="<LSX><Request id=\"e\"><QueryEntitlements/></Request></LSX>";
 constexpr char qo[]="<LSX><Request id=\"o\"><QueryOffers/></Request></LSX>";
 constexpr char qa[]="<LSX><Request id=\"a\"><QueryAchievements/></Request></LSX>";
 if(!direct(qc,handlers::handle_query_content,"<QueryContentResponse/>"))return 10;
 if(!direct(qe,handlers::handle_query_entitlements,"<QueryEntitlementsResponse/>"))return 11;
 if(!direct(qo,handlers::handle_query_offers,"<QueryOffersResponse/>"))return 12;
 if(!direct(qa,handlers::handle_query_achievements,"<AchievementSet GameName=\"Game\" Name=\"\"/>"))return 13;
 if(!dispatch_roundtrip(qc,"<QueryContentResponse/>"))return 20;
 if(!dispatch_roundtrip(qe,"<QueryEntitlementsResponse/>"))return 21;
 if(!dispatch_roundtrip(qo,"<QueryOffersResponse/>"))return 22;
 if(!dispatch_roundtrip(qa,"<AchievementSets>"))return 23;
 config::clear_runtime_configuration();return 0;
}
