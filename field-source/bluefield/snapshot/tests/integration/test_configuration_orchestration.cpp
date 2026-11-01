// SPDX-License-Identifier: GPL-3.0-only
#include "src/application/application.hpp"
#include "src/configuration/defaults.hpp"
#include "tests/support/io.hpp"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif
namespace c=preloader_l::configuration;
using namespace preloader_l;
namespace {
unsigned checks=0;
void require(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
struct Effects:c::Dependencies {
 std::vector<std::string> trace;std::vector<std::pair<std::string,std::string>> environment;bool fail_storage=false;
 Path executable_path_03c6d0() override{trace.push_back("executable");return u"fixture.exe";}
 Path parent_path_019820(const Path&) override{throw std::runtime_error("unused parent");}
 Path join_016b00(const Path&,const Path&) override{throw std::runtime_error("unused join");}
 Path widen_0145d0(std::string_view) override{throw std::runtime_error("unused widen");}
 Path filesystem_widen_010540(std::string_view) override{throw std::runtime_error("unused filesystem widen");}
 std::string narrow_path(const Path&) override{throw std::runtime_error("unused narrow");}
 Path resolve_path_0192d0(std::string_view,const Path&,bool) override{throw std::runtime_error("unused resolve");}
 Path program_data_03c920() override{throw std::runtime_error("unused program root");}
 Path local_app_data_03c6f0() override{throw std::runtime_error("unused legacy root");}
 Path runtime_path(RuntimeFile file) override{trace.push_back("path:"+std::to_string(static_cast<int>(file)));return u"runtime/"+Path(1,static_cast<char16_t>(u'0'+static_cast<int>(file)));}
 c::Node load_document(const Path&) override{throw std::runtime_error("pass owns parsing");}
 void log_config_path(const Path&) override{throw std::runtime_error("pass owns diagnostics");}
 bool crt_is_space(unsigned char) override{throw std::runtime_error("unused locale");}
 int crt_tolower(int) override{throw std::runtime_error("unused locale");}
 void apply_license_key_02a830(std::string_view) override{throw std::runtime_error("unused license");}
 void create_directories_018e00(const Path&) override{throw std::runtime_error("eager directories forbidden in this pass fixture");}
 void write_embedded_png_03c020(const Path&,c::EmbeddedPng) override{throw std::runtime_error("eager avatar forbidden");}
 std::vector<Path> directory_entries(const Path&) override{throw std::runtime_error("eager enumeration forbidden");}
 Path filename(const Path&) override{throw std::runtime_error("unused filename");}
 void remove_04948c(const Path&) override{throw std::runtime_error("eager removal forbidden");}
 void update_achievement_storage_01eb00(c::State& s) override{trace.push_back("storage");require(s.sdk=="1,2,3","SDK normalized before persistence");require(s.game_attributes["AVAILABLE_VERSION"]=="4.5.6","available version follows installed before persistence");if(fail_storage)throw std::runtime_error("fixture storage failure");}
 bool set_environment_variable(std::string_view name,std::string_view value) override{trace.push_back("environment:"+std::string(name));environment.emplace_back(name,value);return false;}
};
struct Pass:c::ConfigurationPass {
 Effects& effects;int fail=-1;unsigned base=0,overlay=0;explicit Pass(Effects& e):effects(e){}
 void load_and_apply(c::State& s,bool which,c::Dependencies&) override{
  effects.trace.push_back(which?"override":"base");if(which)++overlay;else ++base;
  if(fail==static_cast<int>(which))throw std::runtime_error("fixture layer failure");
  if(!which){s.sdk="1.2.3";s.content_id="source-content";s.game_attributes["INSTALLED_VERSION"]="4.5.6";s.game_attributes["INSTALLED_LANGUAGE"]="tr_TR";}
  else s.username="Overlay";
 }
};
void composition(){
 Effects effects;Pass pass(effects);auto state=c::initial_state();state.executable_path=u"caller-owned.exe";
 c::apply_configuration_layers(state,effects,pass);
 require(pass.base==1&&pass.overlay==1,"layers run once in base-override order");
 require(effects.trace.front()=="base"&&effects.trace[1]=="override"&&effects.trace.back()=="storage","shared source post-configuration sequence");
 require(state.executable_path==u"caller-owned.exe","caller executable setup not repeated");
 require(effects.environment.empty(),"layer preparation does not publish environment");
 require(state.username=="Overlay"&&state.avatar==state.default_avatar,"profile/default avatar preserved");
 effects.trace.push_back("caller-profile-router-ready");
 c::publish_configuration_environment(state,effects);
 require(effects.environment==std::vector<std::pair<std::string,std::string>>{{"ContentId","source-content"},{"EAGameLocale","tr_TR"}},"environment names values and order retained");
 require(effects.trace[effects.trace.size()-3]=="caller-profile-router-ready","publication remains a distinct caller-owned boundary");
 Effects full;Pass full_pass(full);auto complete=c::initial_state();c::initialize_configuration(complete,full,full_pass);
 require(full.trace.front()=="executable"&&full_pass.base==1&&full_pass.overlay==1,"standalone entry delegates same layers after executable");
 require(full.environment==effects.environment,"standalone entry retains full publication");
 for(int boundary=0;boundary<3;++boundary){
  Effects failed;Pass failed_pass(failed);auto partial=c::initial_state();if(boundary<2)failed_pass.fail=boundary;else failed.fail_storage=true;
  bool caught=false;try{c::apply_configuration_layers(partial,failed,failed_pass);}catch(const std::runtime_error&){caught=true;}
  require(caught&&failed.environment.empty(),"layer/storage failure cannot publish environment");
  require(failed_pass.base==1&&failed_pass.overlay==(boundary==0?0u:1u),"failure stops later passes without replay");
 }
}
std::optional<std::string> variable(const char* name){
#ifdef _WIN32
 SetLastError(ERROR_SUCCESS);auto count=GetEnvironmentVariableA(name,nullptr,0);if(!count)return GetLastError()==ERROR_ENVVAR_NOT_FOUND?std::nullopt:std::optional<std::string>("");
 std::string value(count,'\0');auto got=GetEnvironmentVariableA(name,value.data(),count);if(got>=count)throw std::runtime_error("test environment changed");value.resize(got);return value;
#else
 const char* value=std::getenv(name);return value?std::optional<std::string>(value):std::nullopt;
#endif
}
struct Variables {
 std::optional<std::string> content=variable("ContentId"),locale=variable("EAGameLocale");
 ~Variables(){
  for(const auto& item:std::vector<std::pair<const char*,std::optional<std::string>>>{{"ContentId",content},{"EAGameLocale",locale}}){
#ifdef _WIN32
   SetEnvironmentVariableA(item.first,item.second?item.second->c_str():nullptr);
#else
   if(item.second)setenv(item.first,item.second->c_str(),1);else unsetenv(item.first);
#endif
  }
 }
 void sentinel(){publish_environment("ContentId","before-content");publish_environment("EAGameLocale","before-locale");}
 bool unchanged(){return variable("ContentId")=="before-content"&&variable("EAGameLocale")=="before-locale";}
};
RuntimePaths paths(const std::filesystem::path& r){std::filesystem::create_directories(r);return RuntimePaths::fixture(r/"fixture.exe",r/"surge.cfg",r/"local",r/"programdata");}
void actual_application(const std::filesystem::path& root){
 Variables environment;
 {auto p=paths(root/"absent");Environment env(p);Application app(env);environment.sentinel();app.initialize();
  const auto& s=app.configuration();require(s.username=="SURGE"&&s.default_avatar==p.model(RuntimeFile::default_avatar),"no-config profile/official logical avatar unchanged");
  require(variable("ContentId")=="0"&&variable("EAGameLocale")=="en_US","actual no-config environment published");
  require(!std::filesystem::exists(p.config())&&!std::filesystem::exists(p.default_avatar()),"no config manufacture or eager avatar write");}
 {auto p=paths(root/"overlay");write_file(p.config().u16string(),"\"Config2\"{\"Game\"{\"ContentId\" \"base-content\" \"Version\" \"4.5.6\" \"Language\" \"tr_TR\"}\"Emulator\"{\"SDK\" \"1.2.3\"}\"User\"{\"Username\" \"Base\"}}");
  write_file(p.override_config().u16string(),"\"Config2\"{\"User\"{\"Username\" \"Overlay\"}}");Environment env(p);Application app(env);app.initialize();const auto& s=app.configuration();
  require(s.username=="Overlay"&&s.sdk=="1,2,3"&&s.game_attributes.at("AVAILABLE_VERSION")=="4.5.6","actual layered state and normalization");
  require(variable("ContentId")=="base-content"&&variable("EAGameLocale")=="tr_TR","actual environment follows final state");
  auto connection=app.connect();require(connection->process("<LSX><Request id=\"layers\" recipient=\"EbisuSDK\"><GetProfile index=\"0\"/></Request></LSX>").find("Persona=\"Overlay\"")!=std::string::npos,"router sees layered profile");}
 for(bool override_failure:{false,true}){auto p=paths(root/(override_failure?"bad-override":"bad-base"));
  write_file(p.config().u16string(),override_failure?"\"Config2\"{\"Game\"{\"ContentId\" \"base-content\"}}":"\"Config2\"{ broken }");
  if(override_failure)write_file(p.override_config().u16string(),"\"Config2\"{ broken }");Environment env(p);Application app(env);environment.sentinel();bool rejected=false;try{app.initialize();}catch(...){rejected=true;}
  require(rejected&&environment.unchanged(),"actual parse failure precedes environment publication");}
 {auto p=paths(root/"storage-failure");write_file(p.config().u16string(),"\"Config2\"{\"Game\"{\"ContentId\" \"base-content\"}\"Emulator\"{\"SDK\" \"1.2.3\"}\"Achievements\"{\"SteamAppId\" \"42\"}}");
  const auto blocker=p.data()/"achievement_watcher";write_file(blocker.u16string(),"blocker");Environment env(p);Application app(env);environment.sentinel();bool rejected=false;try{app.initialize();}catch(...){rejected=true;}
  require(rejected&&environment.unchanged(),"actual persistence failure cannot publish environment");
  require(app.configuration().sdk=="1,2,3"&&read_file(blocker.u16string(),true)=="blocker","normalization precedes storage failure; existing file preserved");}
}
}
int main(){auto root=std::filesystem::temp_directory_path()/("preloader_l_config_owner_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));try{composition();actual_application(root);std::filesystem::remove_all(root);std::cout<<"PASS shared configuration orchestration checks="<<checks<<"; source order + actual Application overlays/profile/filesystem/environment failure boundaries; no original binary execution\n";return 0;}catch(const std::exception& e){std::cerr<<"FAIL shared configuration orchestration: "<<e.what()<<'\n';std::filesystem::remove_all(root);return 1;}}
