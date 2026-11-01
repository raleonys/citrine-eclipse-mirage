#include "preloader_l/core/request_dispatch.hpp"
#include "preloader_l/config/runtime_provider.hpp"
#include "preloader_l/config/settings_values.hpp"
#include <cstring>
using namespace preloader_l;
static bool has(const char* s,const char* n){return s&&n&&std::strstr(s,n)!=nullptr;}
int main(){
 char out[4096]{};
 config::clear_runtime_overrides();

 const char p[]="<LSX><Request id=\"201\"><GetPresenceVisibility UserId=\"7\"/></Request></LSX>";
 auto r=core::dispatch_plaintext_request(p,sizeof(p)-1,out,sizeof(out));
 if(r.status!=core::DispatchStatus::ok||r.route!=router::RouteId::get_presence_visibility||!has(out,"<GetPresenceVisibilityResponse Visible=\"false\"/>"))return 1;

 const char a[]="<LSX><Request id=\"202\"><AreChunksInstalled ItemId=\"GAME\"><ChunkIds>4</ChunkIds><ChunkIds>9</ChunkIds></AreChunksInstalled></Request></LSX>";
 r=core::dispatch_plaintext_request(a,sizeof(a)-1,out,sizeof(out));
 if(r.status!=core::DispatchStatus::ok||r.route!=router::RouteId::are_chunks_installed)return 2;
 if(!has(out,"<AreChunksInstalledResponse ItemId=\"\" Installed=\"true\">")||!has(out,"<ChunkIds>4</ChunkIds><ChunkIds>9</ChunkIds>"))return 3;

 const char restart[]="<LSX><Request id=\"203\"><RestartGame UserId=\"7\" Options=\"NORMAL\"/></Request></LSX>";
 r=core::dispatch_plaintext_request(restart,sizeof(restart)-1,out,sizeof(out));
 if(r.status!=core::DispatchStatus::ok||r.route!=router::RouteId::restart_game)return 4;
 if(!has(out,"<ErrorSuccess Description=\"Origin is waiting for the game to shutdown.\" Code=\"1\"/>"))return 5;

 const char voip[]="<LSX><Request id=\"204\"><GetVoipStatus/></Request></LSX>";
 r=core::dispatch_plaintext_request(voip,sizeof(voip)-1,out,sizeof(out));
 if(r.status!=core::DispatchStatus::ok||r.route!=router::RouteId::get_voip_status)return 6;
 if(!has(out,"<ErrorSuccess Description=\"The function you are calling is not implemented.\" Code=\"-1593835518\"/>"))return 7;

 const char chunk[]="<LSX><Request id=\"205\"><QueryChunkStatus ItemId=\"GAME\"/></Request></LSX>";
 r=core::dispatch_plaintext_request(chunk,sizeof(chunk)-1,out,sizeof(out));
 if(r.status!=core::DispatchStatus::no_response_required||r.route!=router::RouteId::query_chunk_status||r.handler_status!=handlers::HandlerStatus::no_response_required||r.written!=0)return 8;

 const char block[]="<LSX><Request id=\"206\"><GetBlockList/></Request></LSX>";
 r=core::dispatch_plaintext_request(block,sizeof(block)-1,out,sizeof(out));
 if(r.status!=core::DispatchStatus::ok||r.route!=router::RouteId::get_block_list||!has(out,"<GetBlockListResponse Return=\"Success\"/>"))return 9;
 if(has(out,"<User"))return 10;
 // Real socket exchanges always carry a LocalSocialSession pointer, even
 // when Config2 is not loaded. That unbound pointer must preserve the
 // donor-compatible empty response used by frame_coalescing (exit 201).
 core::DispatchEnvironment env{};
 runtime::LocalSocialSession unbound{};
 env.local_social_session=&unbound;
 r=core::dispatch_plaintext_request_with_environment(block,sizeof(block)-1,out,sizeof(out),&env);
 if(r.status!=core::DispatchStatus::ok||!has(out,"<GetBlockListResponse Return=\"Success\"/>"))return 15;
 const char friends[]="<LSX><Request id=\"206f\"><QueryFriends/></Request></LSX>";
 r=core::dispatch_plaintext_request_with_environment(friends,sizeof(friends)-1,out,sizeof(out),&env);
 if(r.status!=core::DispatchStatus::ok||!has(out,"QueryFriendsResponse"))return 16;
 // A bound-but-invalid or stale principal must NEVER downgrade to empty
 // Success merely because its snapshot cannot be trusted.
 runtime::LocalSocialSession forged{101,1,1,true};
 env.local_social_session=&forged;
 r=core::dispatch_plaintext_request_with_environment(block,sizeof(block)-1,out,sizeof(out),&env);
 if(r.status!=core::DispatchStatus::handler_error||r.handler_status!=handlers::HandlerStatus::unsupported)return 17;
 r=core::dispatch_plaintext_request_with_environment(friends,sizeof(friends)-1,out,sizeof(out),&env);
 if(r.status!=core::DispatchStatus::handler_error||r.handler_status!=handlers::HandlerStatus::unsupported)return 18;

 const char unknown[]="<LSX><Request id=\"206u\"><StillUnknown/></Request></LSX>";
 r=core::dispatch_plaintext_request(unknown,sizeof(unknown)-1,out,sizeof(out));
 if(r.status!=core::DispatchStatus::unknown_route||r.written!=0)return 11;

 // StartDownload is route-specific success independent of Online after HF02.
 config::set_runtime_online_override(config::literal_value("false"));
 const char download[]="<LSX><Request id=\"207\"><StartDownload/></Request></LSX>";
 r=core::dispatch_plaintext_request(download,sizeof(download)-1,out,sizeof(out));
 if(r.status!=core::DispatchStatus::ok||!has(out,"Description=\"Requested a download of a DLC offer.\" Code=\"0\""))return 12;
 config::clear_runtime_overrides();
 return 0;
}
