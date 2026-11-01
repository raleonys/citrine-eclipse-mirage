#include "tests/support/io.hpp"
// SPDX-License-Identifier: GPL-3.0-only
#include "src/transport/service.hpp"
#include "third_party/tinyxml2/tinyxml2.h"
#include <chrono>
#include <filesystem>
#include <iostream>
#include <map>
#include <stdexcept>
namespace {int checks=0;void require(bool value,const char* label){++checks;if(!value)throw std::runtime_error(label);}}
int main(){try{
 const auto fixture=std::filesystem::path(__FILE__).parent_path().parent_path()/"fixtures/madden_config.cfg";
 const auto root=std::filesystem::temp_directory_path()/("preloader_l_cfg_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
 std::filesystem::create_directories(root);
 preloader_l::write_file(preloader_l::model_path(root/"surge.cfg"),preloader_l::read_file(preloader_l::model_path(fixture),true));
 preloader_l::Environment env{preloader_l::RuntimePaths::fixture(root/"fixture.exe",root/"surge.cfg",root,root/"programdata")} ;
 preloader_l::Service service(env);service.start();const auto& state=service.application().configuration();
 require(state.game_attributes.at("DISPLAY_NAME")=="Madden NFL 27","actual supplied Game.Name shape");
 require(state.game_attributes.at("INSTALLED_VERSION")=="1.0.140.41856","actual supplied Game.Version");
 require(state.content_id=="16425895","actual supplied ContentId");
 require(state.game_attributes.at("INSTALLED_LANGUAGE")=="en_US","actual supplied locale");
 require(state.language_registry_key.find(u"Madden NFL 27")!=std::u16string::npos,"registry path preserved");
 require(state.sdk=="10,5,96,47512","absent SDK keeps input constructor value");
 require(state.achievements_set=="50311_16425895_50844"&&state.achievements.size()==24,"actual 24-entry nested achievement section");
 require(state.steam_app_id=="3940610"&&state.achievements.at("24").steam_id=="Achievement_GOSCC_24","Steam scalar and prefix binding");
 auto connection=service.application().connect();const auto reply=connection->process("<LSX><Request id=\"fixture\" recipient=\"EbisuSDK\"><GetProfile index=\"0\"/></Request></LSX>");
 tinyxml2::XMLDocument document;require(document.Parse(reply.c_str(),reply.size())==tinyxml2::XML_SUCCESS,"profile XML");
 auto* node=document.FirstChildElement("LSX");require(node!=nullptr,"LSX wrapper");node=node->FirstChildElement("Response");require(node!=nullptr,"Response wrapper");node=node->FirstChildElement("GetProfileResponse");require(node!=nullptr,"profile family");
 std::map<std::string,std::string> actual;for(auto* a=node->FirstAttribute();a;a=a->Next())actual.emplace(a->Name(),a->Value());
 const std::map<std::string,std::string> expected={{"Country","US"},{"GeoCountry","US"},{"PersonaId","10001"},{"IsTrialSubscriber","false"},{"SubscriberLevel","2"},{"Persona","FixturePlayer"},{"UserId","20001"},{"CommerceCountry","US"},{"CommerceCurrency","USD"},{"AvatarId",""},{"IsSubscriber","true"},{"UserIndex","0"},{"IsUnderAge","false"},{"IsSteamSubscriber","false"}};
 require(actual==expected,"all 14 profile fields from anonymized supplied config");
 // Observational differential cases from HF19 test_route_defaults.cpp.
 // A passing observation does not mean these HF19 wire contracts are supported.
 for(const auto* command:{"<QueryImage><Width>208</Width><Height>208</Height><ImageId>user:20001</ImageId></QueryImage>","<GrantAchievement><AchievementId>1</AchievementId></GrantAchievement>"}){
  bool rejected=false;try{connection->process(std::string("<LSX><Request id=\"delta\" recipient=\"EbisuSDK\">")+command+"</Request></LSX>");}
  catch(const std::exception& error){rejected=std::string(error.what())=="invalid LSX request";}
  require(rejected,"observed HF19 child-element request compatibility delta remains open");
 }
 const auto log=preloader_l::read_file(preloader_l::model_path(env.paths.diagnostic_log()),preloader_l::disk_diagnostics_enabled());
 require((!preloader_l::disk_diagnostics_enabled() || log.find("event=config_unsupported")!=std::string::npos),"HF19 Online compatibility gap must be visible");
 service.stop();std::filesystem::remove_all(root);
 std::cout<<"PASS supplied CFG parser/bindings checks="<<checks<<"; Online UNSUPPORTED; observed HF19 QueryImage/GrantAchievement child shapes rejected; game NOT_RUN\n";return 0;
}catch(const std::exception& error){std::cerr<<"FAIL supplied CFG: "<<error.what()<<'\n';return 1;}}
