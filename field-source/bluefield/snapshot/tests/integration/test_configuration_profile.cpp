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
void require(bool b,const char* label){++checks;if(!b)throw std::runtime_error(label);}
using Fields=std::map<std::string,std::string,std::less<>>;
struct Result{Fields profile,game,emulator;std::string sdk,log;unsigned fake_auth,pretend;};
Result run(const std::filesystem::path& root,const char* name,const std::string* base,const char* overlay=nullptr){
 const auto dir=root/name;std::filesystem::create_directories(dir);
 if(base)preloader_l::write_file(preloader_l::model_path(dir/"surge.cfg"),*base);
 if(overlay)preloader_l::write_file(preloader_l::model_path(dir/"surge_override.cfg"),overlay);
 preloader_l::Environment env{preloader_l::RuntimePaths::fixture(dir/"fixture.exe",dir/"surge.cfg",dir,dir/"programdata")} ;
 preloader_l::Service service(env);service.start();const auto& s=service.application().configuration();
 Result result{{},s.game_attributes,s.emulator_attributes,s.sdk,{},s.fake_auth,s.pretend_connected};
 auto c=service.application().connect();const auto reply=c->process("<LSX><Request id=\"config-profile\" recipient=\"EbisuSDK\"><GetProfile index=\"0\"/></Request></LSX>");
 tinyxml2::XMLDocument doc;require(doc.Parse(reply.c_str(),reply.size())==tinyxml2::XML_SUCCESS,"profile XML parses");
 auto* outer=doc.FirstChildElement("LSX");require(outer!=nullptr,"LSX envelope");outer=outer->FirstChildElement("Response");require(outer!=nullptr,"Response envelope");outer=outer->FirstChildElement("GetProfileResponse");require(outer!=nullptr,"profile response");
 for(auto* a=outer->FirstAttribute();a;a=a->Next())result.profile.emplace(a->Name(),a->Value());
 require(result.profile.size()==14,"all fourteen profile fields present");
 result.log=preloader_l::read_file(preloader_l::model_path(env.paths.diagnostic_log()),preloader_l::disk_diagnostics_enabled());
 require((!preloader_l::disk_diagnostics_enabled() || result.log.find("event=config_base_")!=std::string::npos),"exact base config path observable");
 require((!preloader_l::disk_diagnostics_enabled() || result.log.find("event=config_override_")!=std::string::npos),"override path observable");
 require((!preloader_l::disk_diagnostics_enabled() || result.log.find("event=profile_local")!=std::string::npos),"provider absence retains local identity");
 service.stop();return result;
}
}
int main(){try{
 const auto project=std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
 const auto root=std::filesystem::temp_directory_path()/("preloader_l_profile_cfg_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
 const auto baseline=run(root,"absent",nullptr);
 const Fields expected={{"Country","US"},{"GeoCountry","US"},{"PersonaId","1144668899"},{"IsTrialSubscriber","false"},{"SubscriberLevel","2"},{"Persona","SURGE"},{"UserId","1000200030000"},{"CommerceCountry","US"},{"CommerceCurrency","USD"},{"AvatarId",""},{"IsSubscriber","true"},{"UserIndex","0"},{"IsUnderAge","false"},{"IsSteamSubscriber","false"}};
 require(baseline.profile==expected,"SURGE product default; source numeric IDs preserved");
 const auto corrected=preloader_l::read_file(preloader_l::model_path(project/"surge.cfg.example"),true);
 const auto fixed=run(root,"corrected",&corrected);
 require(fixed.profile==baseline.profile,"corrected config equals full no-config profile");
 require(fixed.game==baseline.game&&fixed.emulator==baseline.emulator,"no invented game metadata overrides");
 require(fixed.sdk==baseline.sdk&&fixed.sdk=="10,5,96,47512","SDK retains working source value");
 require(fixed.fake_auth==1&&fixed.pretend==1,"working authentication flags retained");
 require((!preloader_l::disk_diagnostics_enabled() || fixed.log.find("event=config_base_present")!=std::string::npos),"configured keys observable");
 require(fixed.log.find("1144668899")==std::string::npos&&fixed.log.find("1000200030000")==std::string::npos,"profile IDs not logged");
 const auto legacy=preloader_l::read_file(preloader_l::model_path(project/"tests/fixtures/legacy_synthetic.cfg"),true);
 const auto old=run(root,"legacy",&legacy);auto old_expected=expected;old_expected["Persona"]="Player";old_expected["PersonaId"]="1";old_expected["UserId"]="1";old_expected["IsSubscriber"]="false";old_expected["SubscriberLevel"]="0";
 require(old.profile==old_expected,"exact legacy upload parsed, not silently ignored");
 require(old.sdk=="9,9,0,0"&&old.fake_auth==0,"legacy SDK and FakeAuth overrides reproduced");
 const std::string custom="\"Config2\"{\"User\"{\"Username\" \"PrivateNameShouldNotAppear\" \"PersonaId\" \"10001\" \"UserId\" \"20001\"}}";
 const auto overridden=run(root,"override",&custom,"\"Config2\"{\"User\"{\"Username\" \"LocalOverride\"}}");
 auto overridden_expected=expected;overridden_expected["Persona"]="LocalOverride";overridden_expected["PersonaId"]="10001";overridden_expected["UserId"]="20001";
 require(overridden.profile==overridden_expected,"base IDs and override name honored");
 require(overridden.log.find("PrivateNameShouldNotAppear")==std::string::npos&&overridden.log.find("LocalOverride")==std::string::npos,"display values not logged");
 require((!preloader_l::disk_diagnostics_enabled() || overridden.log.find("event=config_override_present")!=std::string::npos),"override selection observable");
 const std::string missing="\"Other\"{\"User\"{\"Username\" \"Ignored\"}}";const auto no_root=run(root,"missing_root",&missing);
 require(no_root.profile==expected&&(!preloader_l::disk_diagnostics_enabled() || no_root.log.find("event=config_base_present")!=std::string::npos),"missing root optional with explicit diagnostic");
 std::filesystem::remove_all(root);std::cout<<"PASS config/profile checks="<<checks<<"; full 14-field response; legacy reproduced; corrected config/default parity; override and non-sensitive diagnostics\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL config/profile: "<<e.what()<<'\n';return 1;}}
