#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/config/runtime_provider.hpp"
#include "preloader_l/handlers/route_handlers.hpp"
#include "preloader_l/runtime/auth_code_service.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include <cstring>
using namespace preloader_l;
static bool has(const char*s,const char*n){return std::strstr(s,n)!=nullptr;}
static bool parse(const char*s,xml::RequestView*r){return xml::parse_request_view(s,std::strlen(s),r)==xml::RequestStatus::ok;}
static bool load(const char*s){return config::set_runtime_configuration_cfg(s,std::strlen(s))==config::RuntimeConfigurationStatus::ok;}
static handlers::HandlerStatus auth(const char* attrs,const char* body,char*out,size_t cap,size_t*n){
 char q[512]{};const char* pre="<LSX><Request id=\"1\"><GetAuthCode";const char* mid=">";const char* post="</GetAuthCode></Request></LSX>";size_t p=0;
 for(const char*c=pre;*c;++c) q[p++]=*c;
 for(const char*c=attrs;*c;++c) q[p++]=*c;
 for(const char*c=mid;*c;++c) q[p++]=*c;
 for(const char*c=body;*c;++c) q[p++]=*c;
 for(const char*c=post;*c;++c) q[p++]=*c;
 q[p]='\0';
 xml::RequestView r{};if(!parse(q,&r)){if(n)*n=0;return handlers::HandlerStatus::invalid_request;}return handlers::handle_get_auth_code(r,out,cap,n);
}
static bool auth8(const char* out,char code[9]){
 const char* p=std::strstr(out,"<AuthCode value=\"");if(!p)return false;p+=17;
 for(size_t i=0;i<8;++i){const char c=p[i];const bool ok=(c>='0'&&c<='9')||(c>='A'&&c<='Z')||(c>='a'&&c<='z');if(!ok)return false;code[i]=c;}code[8]='\0';return p[8]=='\"';
}
int main(){
 char out[2048]{};size_t n=0;char first[9]{},second[9]{};
 config::clear_runtime_overrides();config::clear_runtime_configuration();runtime::clear_auth_code_service();
 // Measurement and failed render must not choose/commit the session Auth8 token.
 if(runtime::session_auth_code_ready())return 9;
 size_t measured=0;
 if(auth(" ClientId=\"client-A\"","",nullptr,0,&measured)!=handlers::HandlerStatus::response_error||measured==0||runtime::session_auth_code_ready())return 10;
 char tiny[1]={'X'};size_t tiny_n=0;
 if(auth(" ClientId=\"client-A\"","",tiny,sizeof tiny,&tiny_n)!=handlers::HandlerStatus::response_error||tiny_n!=measured||runtime::session_auth_code_ready())return 11;
 char exact[2048]{};size_t exact_n=0;
 if(measured>=sizeof exact)return 12;
 if(auth(" ClientId=\"client-A\"","",exact,measured,&exact_n)!=handlers::HandlerStatus::response_error||exact_n!=measured||runtime::session_auth_code_ready())return 13;
 // ClientId is required as an XML attribute; present-empty is valid. The value does not affect Auth8.
 n=9; if(auth("","",out,sizeof out,&n)!=handlers::HandlerStatus::unsupported||n!=0||runtime::session_auth_code_ready())return 1;
 if(auth(" ClientId=\"\"","",out,sizeof out,&n)!=handlers::HandlerStatus::ok||!auth8(out,first)||!runtime::session_auth_code_ready())return 14;
 if(auth("","<ClientId>client-A</ClientId>",out,sizeof out,&n)!=handlers::HandlerStatus::unsupported)return 15;
 n=9; if(auth(" ClientId=\"a\" ClientId=\"b\"","",out,sizeof out,&n)!=handlers::HandlerStatus::invalid_request||n!=0)return 16;
 if(has(out,"c0dec0de"))return 2;
 // ClientId presence/content does not affect the session Auth8 value.
 if(auth(" ClientId=\"client-A\"","",out,sizeof out,&n)!=handlers::HandlerStatus::ok||!auth8(out,second)||std::strcmp(first,second)!=0)return 3;
 if(auth(" ClientId=\"client-B\"","",out,sizeof out,&n)!=handlers::HandlerStatus::ok||!auth8(out,second)||std::strcmp(first,second)!=0)return 4;
 constexpr char off[]=R"CFG("Config2" { "Emulator" { "Online" "false" } })CFG";
 if(!load(off)||auth(" ClientId=\"client-A\"","",out,sizeof out,&n)!=handlers::HandlerStatus::ok||!has(out,"Code=\"-1577058293\""))return 5;
 constexpr char on[]=R"CFG("Config2" { "Emulator" { "Online" "true" } })CFG";
 if(!load(on)||auth(" ClientId=\"client-A\"","",out,sizeof out,&n)!=handlers::HandlerStatus::ok||!auth8(out,second)||std::strcmp(first,second)!=0)return 6;
 // Config2 AuthCode is not the source for the local session adaptation.
 constexpr char ignored[]=R"CFG("Config2" { "Emulator" { "Online" "true" } "User" { "AuthCode" "LOCAL" } })CFG";
 if(!load(ignored)||auth(" ClientId=\"client-A\"","",out,sizeof out,&n)!=handlers::HandlerStatus::ok||!auth8(out,second)||has(out,"LOCAL"))return 7;
 config::clear_runtime_configuration();runtime::clear_auth_code_service();
 if(auth(" ClientId=\"client-A\"","",out,sizeof out,&n)!=handlers::HandlerStatus::ok||!auth8(out,second))return 8;
 runtime::clear_auth_code_service();return 0;
}
