// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/handlers/settings_snapshot_route.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include <cstring>
using namespace preloader_l;
namespace {
struct F { const char* offline;const char* env;const char* telemetry;const char* available;const char* enabled;const char* language; };
bool lookup(const void* p,config::SettingKey k,config::ValueView* out) noexcept {
 const auto* f=static_cast<const F*>(p);if(!f||!out)return false;const char* v=nullptr;
 switch(k){case config::SettingKey::launch_offline_mode:v=f->offline;break;case config::SettingKey::environment:v=f->env;break;case config::SettingKey::telemetry_enabled:v=f->telemetry;break;case config::SettingKey::igo_available:v=f->available;break;case config::SettingKey::igo_enabled:v=f->enabled;break;case config::SettingKey::language:v=f->language;break;default:return false;}
 *out=config::literal_value(v);return true;
}
}
int main(){
 constexpr char in[]="<LSX><Request id=\"29\"><GetSettings/></Request></LSX>";xml::RequestView req{};if(xml::parse_request_view(in,sizeof(in)-1,&req)!=xml::RequestStatus::ok)return 1;
 F f{"1","production","false","true","0","en_US"};config::SettingsProvider p{&f,&lookup};char out[512]{};size_t n=0;
 if(handlers::handle_get_settings(req,p,out,sizeof out,&n)!=handlers::HandlerStatus::ok)return 2;
 constexpr char expected[]=" <LSX>\n  <Response id=\"29\" sender=\"EbisuSDK\">\n    <GetSettingsResponse IsManualOffline=\"true\" Environment=\"production\" IsTelemetryEnabled=\"false\" IsIGOAvailable=\"true\" IsIGOEnabled=\"false\" Language=\"en_US\" IsAutomaticGameUpdatesEnabled=\"false\"/>\n  </Response>\n </LSX>\n";
 if(n!=sizeof(expected)-1||std::strcmp(out,expected)!=0)return 3;
 F special{"1","stage&dev<qa>","false","true","0","tr_\"TR'"};config::SettingsProvider sp{&special,&lookup};
 if(handlers::handle_get_settings(req,sp,out,sizeof out,&n)!=handlers::HandlerStatus::ok)return 4;
 if(!std::strstr(out,"Environment=\"stage&amp;dev&lt;qa&gt;\"")||!std::strstr(out,"Language=\"tr_&quot;TR&apos;\""))return 5;
 F raw{"1","production","1","true","0","en_US"};config::SettingsProvider rp{&raw,&lookup};
 if(handlers::handle_get_settings(req,rp,out,sizeof out,&n)!=handlers::HandlerStatus::ok)return 6;
 if(!std::strstr(out,"IsTelemetryEnabled=\"1\""))return 7;
 F escaped{"1","production","A&B","true","0","en_US"};config::SettingsProvider ep{&escaped,&lookup};
 if(handlers::handle_get_settings(req,ep,out,sizeof out,&n)!=handlers::HandlerStatus::ok)return 8;
 if(!std::strstr(out,"IsTelemetryEnabled=\"A&amp;B\""))return 9;
 F bad{"maybe","production","false","true","false","en_US"};config::SettingsProvider q{&bad,&lookup};if(handlers::handle_get_settings(req,q,out,sizeof out,&n)!=handlers::HandlerStatus::unsupported)return 10;
 return 0;
}
