#include "tests/support/io.hpp"
// SPDX-License-Identifier: GPL-3.0-only
#include "src/transport/service.hpp"
#include "third_party/tinyxml2/tinyxml2.h"
#include <chrono>
#include <filesystem>
#include <iostream>
#include <map>
#include <stdexcept>
namespace {
int checks=0;
void require(bool value,const char* label){++checks;if(!value)throw std::runtime_error(label);}
using Fields=std::map<std::string,std::string>;
// Source constructor 036d20 + handler 02f530, with user-selected FIX07
// display name SURGE. Numeric IDs and other defaults remain pinned.
const Fields profile={
 {"Country","US"},{"GeoCountry","US"},{"PersonaId","1144668899"},
 {"IsTrialSubscriber","false"},{"SubscriberLevel","2"},{"Persona","SURGE"},
 {"UserId","1000200030000"},{"CommerceCountry","US"},{"CommerceCurrency","USD"},
 {"AvatarId",""},{"IsSubscriber","true"},{"UserIndex","0"},
 {"IsUnderAge","false"},{"IsSteamSubscriber","false"}
};
Fields response(preloader_l::Connection& connection,const char* command,const char* tag){
 const auto text=connection.process(std::string("<LSX><Request id=\"compat\" recipient=\"EbisuSDK\">")+command+"</Request></LSX>");
 tinyxml2::XMLDocument document;
 require(document.Parse(text.c_str(),text.size())==tinyxml2::XML_SUCCESS,"valid response XML");
 auto* root=document.FirstChildElement("LSX");require(root!=nullptr,"LSX envelope");
 auto* outer=root->FirstChildElement("Response");require(outer!=nullptr,"Response envelope");
 auto* item=outer->FirstChildElement(tag);require(item!=nullptr,"expected response family");
 Fields result;for(auto* a=item->FirstAttribute();a;a=a->Next())result.emplace(a->Name(),a->Value());return result;
}
void run(const std::filesystem::path& root,const char* name,const char* base,const char* overlay,Fields expected){
 const auto dir=root/name;std::filesystem::create_directories(dir);
 if(base)preloader_l::write_file(preloader_l::model_path(dir/"surge.cfg"),base);
 if(overlay)preloader_l::write_file(preloader_l::model_path(dir/"surge_override.cfg"),overlay);
 preloader_l::Environment env{preloader_l::RuntimePaths::fixture(dir/"fixture.exe",dir/"surge.cfg",dir,dir/"programdata")} ;
 preloader_l::Service service(env);service.start();
 auto& state=service.application().configuration();
 require(service.port()!=0,"bound port without mandatory base configuration");
 require(state.sdk=="10,5,96,47512","source constructor SDK retained");
 require(state.game_attributes.size()==14,"all fourteen default game attributes");
 require(state.emulator_attributes.size()==6,"all six default emulator attributes");
 require(state.game_attributes.at("MAX_GROUP_SIZE")=="16","read-only donor constant");
 require(state.game_attributes.at("DISPLAY_NAME")=="Game","original game display default");
 require(state.game_attributes.at("INSTALLED_LANGUAGE")=="en_US","default game locale");
 require(state.cookie_path==preloader_l::model_path(env.profile_root/"remid_cookie"),"source-compatible cookie location");
 require(state.last_profile_path==preloader_l::model_path(env.profile_root/"last_profile"),"source-compatible profile location");
 require(state.last_avatar_path==preloader_l::model_path(env.profile_root/"last_avatar"),"source-compatible avatar location");
 require(!std::filesystem::exists(preloader_l::native_path(state.default_avatar)),"official avatar is lazy, no startup file");
 require(state.default_avatar==env.paths.model(preloader_l::RuntimeFile::default_avatar),"version-owned official asset path");
 if(std::string(name)=="custom_avatar")require(!std::filesystem::exists(preloader_l::native_path(state.avatar)),"custom path never manufactured");
 else require(state.avatar==state.default_avatar,"official logical default");
 auto connection=service.application().connect();
 require(response(*connection,"<GetProfile index=\"0\"/>","GetProfileResponse")==expected,"complete fourteen-field profile contract");
 require(response(*connection,"<GetSettings/>","GetSettingsResponse")==Fields{
  {"Environment","production"},{"IsIGOAvailable","false"},{"IsTelemetryEnabled","false"},
  {"IsManualOffline","false"},{"Language","en_US"},{"IsIGOEnabled","false"}},"complete default settings response");
 auto game=response(*connection,"<GetAllGameInfo/>","GetAllGameInfoResponse");
 require(!game.at("SystemTime").empty(),"dynamic game time is present");game.erase("SystemTime");
 require(game==Fields{{"DisplayName","Game"},{"InstalledVersion","1.0.0.0"},
  {"AvailableVersion","1.0.0.0"},{"UpToDate","true"},{"FullGameReleased","true"},
  {"FullGameReleaseDate","2010-01-01T00:00:00"},{"FullGamePurchased","true"},
  {"FreeTrial","false"},{"Expiration","0000-00-00T00:00:00"},{"EntitlementSource","ORIGIN"},
  {"MaxGroupSize","16"},{"InstalledLanguage","en_US"},{"HasExpiration","false"},
  {"Languages","ar_SA,cs_CZ,da_DK,de_DE,el_GR,en_AU,en_CA,en_GB,en_US,es_ES,es_MX,fi_FI,fr_CA,fr_FR,hu_HU,it_IT,ja_JP,ko_KR,nl_NL,no_NO,pl_PL,pt_BR,pt_PT,ru_RU,sv_SE,th_TH,tr_TR,zh_CN,zh_HK,zh_TW"}},"complete default game information response");
 require(response(*connection,"<GetGameInfo GameInfoId=\"EXPIRATION_DURATION\"/>","GetGameInfoResponse")==Fields{{"GameInfo","36000000"}},"default game attribute outside aggregate response");
 require(std::filesystem::exists(env.configuration_file)==(base!=nullptr),"startup does not manufacture a config file");
 service.stop();
}
}
int main(){try{
 const auto root=std::filesystem::temp_directory_path()/("preloader_l_compat_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
 run(root,"absent",nullptr,nullptr,profile);
 run(root,"empty","",nullptr,profile);
 run(root,"missing_root","\"Other\"{}",nullptr,profile);
 auto partial=profile;partial["Persona"]="Local & Player";
 run(root,"partial","\"Config2\"{\"User\"{\"Username\" \"Local & Player\"}}",nullptr,partial);
 auto override_only=profile;override_only["Persona"]="Override";
 run(root,"override_only",nullptr,"\"Config2\"{\"User\"{\"Username\" \"Override\"}}",override_only);
 auto both=partial;both["Persona"]="Final";both["CommerceCurrency"]="EUR";
 run(root,"base_and_override","\"Config2\"{\"User\"{\"Username\" \"Local & Player\"}}","\"Config2\"{\"Game\"{\"Name\" \"Override must not replace base-only fields\"}\"User\"{\"Username\" \"Final\" \"Currency\" \"EUR\"}}",both);
 run(root,"explicit_empty","\"Config2\"{\"User\"{\"Username\" \"\" \"PersonaId\" \"\" \"UserId\" \"\"}}",nullptr,profile);
 run(root,"custom_avatar","\"Config2\"{\"User\"{\"Avatar\" \"custom-missing.png\"}}",nullptr,profile);
 // A present directory is not treated as an absent optional file.
 const auto dir=root/"unreadable";std::filesystem::create_directories(dir/"surge.cfg");
 preloader_l::Environment env{preloader_l::RuntimePaths::fixture(dir/"fixture.exe",dir/"surge.cfg",dir,dir/"programdata")} ;
 bool rejected=false;try{preloader_l::Service service(env);service.start();}catch(const std::exception&){rejected=true;}
 require(rejected,"existing non-file configuration must fail explicitly");
 std::filesystem::remove_all(root);
 std::cout<<"PASS startup compatibility checks="<<checks<<"; absent/empty/root/partial/override/empty-fields; complete profile/settings/game responses\n";return 0;
}catch(const std::exception& error){std::cerr<<"FAIL startup compatibility: "<<error.what()<<'\n';return 1;}}
