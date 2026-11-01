#include "preloader_l/handlers/settings_query.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include <cstring>
using namespace preloader_l;
namespace {
struct F { const char* online;const char* offline;const char* lang;const char* env;const char* telemetry;const char* avail;const char* enabled; };
bool lookup(const void* p,config::SettingKey k,config::ValueView* out) noexcept {
 const auto* f=static_cast<const F*>(p);if(!f||!out)return false;const char* v=nullptr;
 switch(k){case config::SettingKey::online:v=f->online;break;case config::SettingKey::launch_offline_mode:v=f->offline;break;case config::SettingKey::language:v=f->lang;break;case config::SettingKey::environment:v=f->env;break;case config::SettingKey::telemetry_enabled:v=f->telemetry;break;case config::SettingKey::igo_available:v=f->avail;break;case config::SettingKey::igo_enabled:v=f->enabled;break;}
 if(!v)return false;
 *out=config::literal_value(v);
 return true;
}
bool parse(const char* p,size_t n,xml::RequestView* r){return xml::parse_request_view(p,n,r)==xml::RequestStatus::ok;}
}
int main(){
 F f{"false","false","tr_TR","test","false","true","false"};config::SettingsProvider provider{&f,&lookup};char out[512]{};size_t n=0;xml::RequestView req{};
 constexpr char a[]="<LSX><Request id=\"30a\"><GetSetting/></Request></LSX>";if(!parse(a,sizeof(a)-1,&req))return 1;
 if(handlers::handle_settings_query(req,provider,true,config::SettingKey::online,out,sizeof out,&n)!=handlers::HandlerStatus::ok||!std::strstr(out,"Setting=\"false\""))return 2;
 if(handlers::handle_settings_query(req,provider,false,config::SettingKey::online,out,sizeof out,&n)!=handlers::HandlerStatus::unsupported)return 3;
 constexpr char b[]="<LSX><Request id=\"30b\"><GetSettings/></Request></LSX>";if(!parse(b,sizeof(b)-1,&req))return 4;
 if(handlers::handle_settings_query(req,provider,false,config::SettingKey::online,out,sizeof out,&n)!=handlers::HandlerStatus::ok)return 5;
 if(!std::strstr(out,"IsManualOffline=\"false\"")||!std::strstr(out,"Environment=\"test\"")||!std::strstr(out,"Language=\"tr_TR\""))return 6;
 constexpr char c[]="<LSX><Request id=\"30c\"><GetInternetConnectedState/></Request></LSX>";if(!parse(c,sizeof(c)-1,&req))return 7;
 if(handlers::handle_settings_query(req,provider,false,config::SettingKey::online,out,sizeof out,&n)!=handlers::HandlerStatus::ok||!std::strstr(out,"connected=\"false\""))return 8;
 F missing{nullptr,"false","tr_TR","test","false","1","0"};config::SettingsProvider p2{&missing,&lookup};
 if(handlers::handle_settings_query(req,p2,false,config::SettingKey::online,out,sizeof out,&n)!=handlers::HandlerStatus::ok||!std::strstr(out,"connected=\"true\""))return 9;
 return 0;
}
