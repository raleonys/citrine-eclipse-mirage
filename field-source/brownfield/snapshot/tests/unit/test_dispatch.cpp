#include "preloader_l/core/request_dispatch.hpp"
#include <cstring>
using namespace preloader_l;
int main(){
 char out[4096]{};
 constexpr char ok[]="<LSX><Request id=\"23\" recipient=\"Utility\"><GetConfig/></Request></LSX>";
 auto r=core::dispatch_plaintext_request(ok,sizeof(ok)-1,out,sizeof out);
 if(r.status!=core::DispatchStatus::ok || r.route!=router::RouteId::get_config || r.handler_status!=handlers::HandlerStatus::ok || r.written!=std::strlen(out)) return 1;
 if(!std::strstr(out,"<GetConfigResponse>") || !std::strstr(out,"sender=\"Utility\"")) return 2;

 char permission_out[512]{};
 constexpr char permission[]="<LSX><Request id=\"23\"><CheckPermission/></Request></LSX>";
 r=core::dispatch_plaintext_request(permission,sizeof(permission)-1,permission_out,sizeof permission_out);
 if(r.status!=core::DispatchStatus::ok || r.route!=router::RouteId::check_permission || r.handler_status!=handlers::HandlerStatus::ok || r.written==0) return 3;
 if(!std::strstr(permission_out,"<CheckPermissionResponse Access=\"GRANTED\"/>")) return 4;

 constexpr char profile[]="<LSX><Request id=\"23\"><GetProfile index=\"0\"/></Request></LSX>";
 r=core::dispatch_plaintext_request(profile,sizeof(profile)-1,out,sizeof out);
 if(r.status!=core::DispatchStatus::ok || r.route!=router::RouteId::get_profile || r.written==0) return 5;

 constexpr char unknown[]="<LSX><Request id=\"23\"><NoSuchCommand/></Request></LSX>";
 std::memset(out,'#',sizeof(out));
 r=core::dispatch_plaintext_request(unknown,sizeof(unknown)-1,out,sizeof out);
 if(r.status!=core::DispatchStatus::unknown_route || r.written!=0) return 6;
 for(char c:out) if(c!='#') return 11;

 constexpr char incomplete[]="<LSX><Request id=\"23\"><GetConfig/>";
 r=core::dispatch_plaintext_request(incomplete,sizeof(incomplete)-1,out,sizeof out);
 if(r.status!=core::DispatchStatus::incomplete_request) return 7;

 constexpr char malformed[]="BAD";
 r=core::dispatch_plaintext_request(malformed,sizeof(malformed)-1,out,sizeof out);
 if(r.status!=core::DispatchStatus::malformed_request) return 8;

 r=core::dispatch_plaintext_request(ok,sizeof(ok)-1,nullptr,0);
 if(r.status!=core::DispatchStatus::capacity || r.written==0) return 9;
 const size_t required=r.written;
 char tiny[4]={'#','#','#','#'};
 r=core::dispatch_plaintext_request(ok,sizeof(ok)-1,tiny,sizeof tiny);
 if(r.status!=core::DispatchStatus::capacity || r.written!=required) return 10;
 for(char c:tiny) if(c!='#') return 12;

 // Exact-boundary regression: `written` excludes the terminal NUL, so a
 // capacity exactly equal to `written` is still one byte too small.
 if(required>=sizeof(out)) return 13;
 std::memset(out,'#',sizeof(out));
 r=core::dispatch_plaintext_request(ok,sizeof(ok)-1,out,required);
 if(r.status!=core::DispatchStatus::capacity || r.written!=required) return 14;
 for(size_t i=0;i<required;++i) if(out[i]!='#') return 15;

 // required + 1 is the first sufficient capacity.
 r=core::dispatch_plaintext_request(ok,sizeof(ok)-1,out,required+1);
 if(r.status!=core::DispatchStatus::ok || r.written!=required) return 16;
 if(out[required]!='\0') return 17;
 return 0;
}
