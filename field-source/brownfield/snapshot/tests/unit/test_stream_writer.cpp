#include "preloader_l/config/settings_provider.hpp"
#include "preloader_l/config/runtime_provider.hpp"
#include "preloader_l/core/request_dispatch.hpp"
#include "preloader_l/handlers/route_handlers.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include <cstring>
using namespace preloader_l;
using RouteHandler=handlers::HandlerStatus(*)(const xml::RequestView&,char*,size_t,size_t*) noexcept;
static bool parse(const char*s,xml::RequestView*r){return xml::parse_request_view(s,std::strlen(s),r)==xml::RequestStatus::ok;}
static bool direct_contract(const char* request,RouteHandler handler){
 xml::RequestView r{};if(!parse(request,&r))return false;
 size_t required=0;
 if(handler(r,nullptr,0,&required)!=handlers::HandlerStatus::response_error||required==0||required>=8192u)return false;
 char tiny[8];std::memset(tiny,'X',sizeof tiny);size_t n=0;
 if(handler(r,tiny,sizeof tiny,&n)!=handlers::HandlerStatus::response_error||n!=required||tiny[0]!='\0')return false;
 char exact[8192];std::memset(exact,'Y',sizeof exact);n=0;
 if(handler(r,exact,required,&n)!=handlers::HandlerStatus::response_error||n!=required||exact[0]!='\0')return false;
 char full[8192];std::memset(full,'Z',sizeof full);n=0;
 return handler(r,full,required+1u,&n)==handlers::HandlerStatus::ok&&n==required&&full[required]=='\0';
}
static bool dispatch_contract(const char* request){
 const size_t len=std::strlen(request);
 const auto measured=core::dispatch_plaintext_request(request,len,nullptr,0);
 if(measured.status!=core::DispatchStatus::capacity||measured.written==0||measured.written>=8192u)return false;
 char tiny[8];std::memset(tiny,'Q',sizeof tiny);
 const auto small=core::dispatch_plaintext_request(request,len,tiny,sizeof tiny);
 if(small.status!=core::DispatchStatus::capacity||small.written!=measured.written||tiny[0]!='\0')return false;
 char exact[8192];std::memset(exact,'R',sizeof exact);
 const auto boundary=core::dispatch_plaintext_request(request,len,exact,measured.written);
 if(boundary.status!=core::DispatchStatus::capacity||boundary.written!=measured.written||exact[0]!='\0')return false;
 char full[8192];std::memset(full,'S',sizeof full);
 const auto rendered=core::dispatch_plaintext_request(request,len,full,measured.written+1u);
 return rendered.status==core::DispatchStatus::ok&&rendered.written==measured.written&&full[measured.written]=='\0';
}
int main(){
 config::set_runtime_online_override(config::literal_value("true"));
 constexpr char content[]="<LSX><Request id=\"1\"><QueryContent/></Request></LSX>";
 constexpr char achievements[]="<LSX><Request id=\"2\"><QueryAchievements/></Request></LSX>";
 if(!direct_contract(content,handlers::handle_query_content))return 1;
 if(!direct_contract(achievements,handlers::handle_query_achievements))return 2;
 if(!dispatch_contract(content))return 3;
 if(!dispatch_contract(achievements))return 4;
 return 0;
}
