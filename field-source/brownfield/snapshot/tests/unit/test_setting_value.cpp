#include "preloader_l/handlers/setting_value_route.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include <cstring>
using namespace preloader_l;
namespace {
struct F { const char* online; };
bool lookup(const void* p, config::SettingKey k, config::ValueView* out) noexcept {
    if(!p||!out||k!=config::SettingKey::online)return false;
    *out=config::literal_value(static_cast<const F*>(p)->online);return true;
}
}
int main(){
 constexpr char in[]="<LSX><Request id=\"28\"><GetSetting/></Request></LSX>";xml::RequestView req{};
 if(xml::parse_request_view(in,sizeof(in)-1,&req)!=xml::RequestStatus::ok)return 1;
 F f{"1"};config::SettingsProvider p{&f,&lookup};char out[256]{};size_t n=0;
 if(handlers::handle_get_setting(req,config::SettingKey::online,p,out,sizeof out,&n)!=handlers::HandlerStatus::ok)return 2;
 constexpr char expected[]=" <LSX>\n  <Response id=\"28\" sender=\"EbisuSDK\">\n    <GetSettingResponse Setting=\"1\"/>\n  </Response>\n </LSX>\n";
 if(n!=sizeof(expected)-1||std::strcmp(out,expected)!=0)return 3;
 config::SettingsProvider none{nullptr,nullptr};if(handlers::handle_get_setting(req,config::SettingKey::online,none,out,sizeof out,&n)!=handlers::HandlerStatus::unsupported)return 4;
 F special{"A&B<C>\"D'E"};config::SettingsProvider specialp{&special,&lookup};
 if(handlers::handle_get_setting(req,config::SettingKey::online,specialp,out,sizeof out,&n)!=handlers::HandlerStatus::ok)return 5;
 constexpr char escaped[]=" <LSX>\n  <Response id=\"28\" sender=\"EbisuSDK\">\n    <GetSettingResponse Setting=\"A&amp;B&lt;C&gt;&quot;D&apos;E\"/>\n  </Response>\n </LSX>\n";
 if(n!=sizeof(escaped)-1||std::strcmp(out,escaped)!=0)return 6;
 return 0;
}
