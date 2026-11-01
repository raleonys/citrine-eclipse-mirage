#include "preloader_l/core/request_dispatch.hpp"
#include <cstdio>
#include <cstring>
using namespace preloader_l;
int main(){
 const char* commands[]={
  "GetSetting","ShowIGO","IsProgressiveInstallationAvailable","GetAllGameInfo","QueryEntitlements","GetProfile","QueryOffers","GetSettings","GetConfig","StartDownload","GetPresence","GetGameInfo","GrantAchievement","CheckPermission","QueryAchievements","GetInternetConnectedState","QueryFriends","SetPresence","QueryImage","GetAuthCode","QueryContent","ShowIGOWindow"
 };
 char req[512]; char out[8192];
 for(size_t i=0;i<sizeof(commands)/sizeof(commands[0]);++i){
  const char* c=commands[i];
  if(std::strcmp(c,"GetSetting")==0) std::snprintf(req,sizeof req,"<LSX><Request id=\"%zu\"><GetSetting SettingId=\"LANGUAGE\"/></Request></LSX>",i+1);
  else if(std::strcmp(c,"GetGameInfo")==0) std::snprintf(req,sizeof req,"<LSX><Request id=\"%zu\"><GetGameInfo GameInfoId=\"INSTALLED_LANGUAGE\"/></Request></LSX>",i+1);
  else std::snprintf(req,sizeof req,"<LSX><Request id=\"%zu\"><%s/></Request></LSX>",i+1,c);
  auto r=core::dispatch_plaintext_request(req,std::strlen(req),out,sizeof out);
  if(r.status==core::DispatchStatus::unimplemented_route) return int(10+i);
  if(r.status==core::DispatchStatus::unknown_route) return int(40+i);
 }
 return 0;
}
