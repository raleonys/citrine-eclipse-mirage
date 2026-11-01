// SPDX-License-Identifier: GPL-3.0-only
#include "src/runtime/runtime_paths.hpp"
#include "src/files/atomic_io.hpp"
#include "src/application/avatars.hpp"
#include "src/application/avatar_assets.hpp"
#include "src/profiles/online_session.hpp"
#include "src/transport/service.hpp"
#include "third_party/tinyxml2/tinyxml2.h"
#include <chrono>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <type_traits>
static_assert(!std::is_invocable_v<decltype(&preloader_l::Environment::log),preloader_l::Environment*,std::string_view>);
namespace {
namespace fs=std::filesystem;
using namespace preloader_l;
int checks=0;
void require(bool v,const char* s){++checks;if(!v)throw std::runtime_error(s);}
template<class F> void rejected(F f,const char* label){bool failed=false;try{f();}catch(const std::exception&){failed=true;}require(failed,label);}
std::string get(const fs::path& p){return read_file(p.u16string(),true);}
std::string official(){return {reinterpret_cast<const char*>(assets::surge_png),sizeof assets::surge_png};}
RuntimePaths paths(const fs::path& r){return RuntimePaths::from_system(r/"module/preloader_l.dll",r/"game/game.exe",r/"local",r/"program");}
std::string request(Connection& c,std::string command){return c.process("<LSX><Request id=\"migration\" recipient=\"EbisuSDK\">"+command+"</Request></LSX>");}
std::string image(Connection& c,const std::string& owner){auto xml=request(c,"<QueryImage Width=\"208\" Height=\"208\" ImageId=\"avatar:"+owner+"\"/>");tinyxml2::XMLDocument d;require(d.Parse(xml.data(),xml.size())==tinyxml2::XML_SUCCESS,"image XML");auto* i=d.FirstChildElement("LSX")->FirstChildElement("Response")->FirstChildElement("QueryImageResponse");require(i&&i->FirstChildElement("Image"),"image response");auto* p=i->FirstChildElement("Image")->Attribute("ResourcePath");require(p!=nullptr,"resource path");return p;}
void check_paths(const fs::path& r){
 auto p=paths(r);require(!fs::exists(r),"paths have no creation effects");require(p.data()==r/"local/SURGE","single root");require(p.config()==r/"module/surge.cfg","module config");require(p.override_config()==r/"module/surge_override.cfg","module override");require(p.provider()==r/"module/preloader_l_online.dll","trusted provider");require(p.sidecar_avatar()==r/"module/surge.png","readonly sidecar");
 for(auto s:{"","relative"})rejected([&]{RuntimePaths::from_system(r/"m.dll",r/"g.exe",s,r/"program");},"bad local root");
 auto nul=(r/"nul").native();nul.push_back(fs::path::value_type(0));nul+=fs::path("tail").native();rejected([&]{RuntimePaths::from_system(r/"m.dll",r/"g.exe",fs::path(nul),r/"program");},"NUL root");
 atomic_write(r/"file","occupied",nullptr);rejected([&]{RuntimePaths::from_system(r/"m.dll",r/"g.exe",r/"file",r/"program");},"local file not directory");
 for(const auto& bad:{r/"local/BLUEFIELD/LSX emu/x",r/"local/anadius/LSX emu/x",p.data()/"../escape",r/"module/surge.cfg"})rejected([&]{atomic_write(bad,"no",&p);},"unowned writes rejected");
 require(p.default_avatar()==p.data()/"surge.png" && p.profile_cache()==p.data(),"flat source-compatible layout");
 require(p.avatar_cache("123","00208").filename()=="_AVATAR_123_00208","original width spelling retained");
 require(p.avatar_cache("name.with spaces","8193").filename()=="_AVATAR_name.with spaces_8193","no invented owner/width restrictions");
 require(p.avatar_cache(std::string(160,'a'),"-1").filename().string().size()>128,"no invented 128 character cap");
 require(p.external_contract(r/"program/Electronic Arts/EA Services/License"/fs::path(u8"İçerik uzun değer.dlf")),"no invented ASCII narrowing in external owner check");
 require(p.external_contract(r/"program/Electronic Arts/EA Services/License/ABC_cached.dlf"),"external license contract retained");require(p.external_contract(r/"game/Denuvo_ticket_123.txt"),"external ticket contract retained");require(!p.external_contract(r/"game/other.txt"),"external write scope bounded");
 fs::create_directories(p.data());fs::create_directories(r/"elsewhere");std::error_code ec;fs::create_directory_symlink(r/"elsewhere",p.data()/"redirect",ec);if(!ec){rejected([&]{atomic_write(p.data()/"redirect/x","no",&p);},"symlink write blocked");require(!fs::exists(r/"elsewhere/x"),"redirect untouched");}
}
void check_assets(const fs::path& r){
 auto p=paths(r);AvatarStore a(p);const auto logical=a.logical_default();require(!fs::exists(p.data()),"lazy constructor");require(fs::path(a.resolve(logical))==p.default_avatar(),"embedded fallback path");require(get(p.default_avatar())==official(),"exact embedded official bytes");
 atomic_write(p.default_avatar(),"user collision",nullptr);require(a.resolve(logical)==p.default_avatar().u16string(),"existing image accepted without byte seal");require(get(p.default_avatar())=="user collision","existing image intact");
 const auto custom=r/"custom.png";atomic_write(custom,"CUSTOM",nullptr);require(a.resolve(custom.u16string())==custom.u16string(),"explicit avatar readonly");require(get(custom)=="CUSTOM","custom intact");
 auto q=paths(r/"sidecar");AvatarStore side(q);atomic_write(q.sidecar_avatar(),official(),nullptr);require(side.resolve(side.logical_default())==q.sidecar_avatar().u16string(),"official module sidecar chosen");require(!fs::exists(q.data()),"sidecar creates no app files");atomic_write(q.default_avatar(),"existing-default",nullptr);require(side.resolve(side.logical_default())==q.sidecar_avatar().u16string(),"previous sidecar priority retained without byte gate");require(get(q.default_avatar())=="existing-default","sidecar selection does not overwrite prior default");require(get(q.sidecar_avatar())==official(),"sidecar intact");
 auto bad=paths(r/"bad_sidecar");AvatarStore wrong(bad);atomic_write(bad.sidecar_avatar(),"user data",nullptr);require(wrong.resolve(wrong.logical_default())==bad.sidecar_avatar().u16string(),"sidecar accepted without invented hash requirement");require(get(bad.sidecar_avatar())=="user data","unsealed sidecar preserved");
}
struct Fault:BatchHooks {void before_commit(std::size_t index)override{if(index==1)throw std::runtime_error("second commit injected");}};
void check_atomic(const fs::path& r){auto p=paths(r);auto a=p.data()/"a",b=p.data()/"b";atomic_write(a,"old-a",&p);atomic_write(b,"old-b",&p);Fault fault;
 rejected([&]{atomic_batch({{a,"new-a"},{b,"new-b"}},p,&fault);},"batch fault observed");require(get(a)=="old-a"&&get(b)=="old-b","both preimages restored");
 auto fresh=p.data()/"new";rejected([&]{atomic_batch({{fresh,"new"},{b,"new-b"}},p,&fault);},"new target rollback");require(!fs::exists(fresh)&&get(b)=="old-b","new target removed, old target untouched");
 atomic_batch({{a,"new-a"},{b,"new-b"}},p);require(get(a)=="new-a"&&get(b)=="new-b","successful batch");
 rejected([&]{atomic_write(a,"overwrite",&p,true);},"create-only collision");require(get(a)=="new-a","create-only original intact");
 rejected([&]{atomic_batch({{a,"one"},{a,"two"}},p);},"duplicate targets rejected");require(get(a)=="new-a","duplicate precommit no change");
 for(const auto& e:fs::recursive_directory_iterator(p.data()))require(!e.path().filename().string().starts_with(".preloader_l-"),"no abandoned temporaries");
 struct RecoveryFault:BatchHooks {fs::path target;explicit RecoveryFault(fs::path p):target(std::move(p)){} void before_commit(std::size_t i)override{if(i==1){fs::remove(target);fs::create_directory(target);throw std::runtime_error("rollback target unavailable");}}} recovery{a};
 bool recovery_required=false;try{atomic_batch({{a,"later-a"},{b,"later-b"}},p,&recovery);}catch(const PersistenceRecoveryRequired&){recovery_required=true;}
 require(recovery_required,"incomplete rollback never reports ordinary success");bool retained=false;for(const auto& e:fs::directory_iterator(p.data()))if(e.path().filename().string().starts_with(".preloader_l-")&&get(e.path())=="new-a")retained=true;require(retained,"recovery preimage retained");
}
void check_logs(const fs::path& r){Environment env{paths(r)};env.log(LogEvent::startup);
 if(disk_diagnostics_enabled()){require(fs::is_regular_file(env.paths.diagnostic_log()),"debug log exists");auto text=get(env.paths.diagnostic_log());require(text.find("event=startup")!=std::string::npos,"fixed safe event");atomic_write(env.paths.diagnostic_log(),std::string(300000,'x'),&env.paths);env.log(LogEvent::network_receive);require(fs::file_size(env.paths.diagnostic_log())>300000,"no invented append cutoff");}
 else require(!fs::exists(env.data_root),"production log creates no directory");
 atomic_write(env.paths.diagnostic_log(),"prior-user-log",&env.paths);env.log(LogEvent::stopped);if(!disk_diagnostics_enabled())require(get(env.paths.diagnostic_log())=="prior-user-log","production preserves old log");
}
void check_cache(const fs::path& r){Environment env{paths(r)};configuration::State state;std::uint8_t online=0;OnlineSession session(state,env,online);require(session.load_copy_0450d0((r/"module/anadius64online.dll").u16string())==0,"old provider basename rejected");require(session.load_copy_0450d0(env.paths.provider().u16string())==0,"missing provider offline");auto file=env.paths.file(RuntimeFile::last_profile);
 for(const auto& bad:std::vector<std::string>{"wrong","1;2;","1;2;name;extra","18446744073709551616;2;name","1;2;bad\nname",std::string("1;2;bad\0name",12),std::string("1;2;\xc0\xaf",6),"1;2;\x01"}){atomic_write(file,bad,&env.paths);require(session.read_binary_03b3f0(file.u16string())==bad,"source raw cache reader, no additional acceptance rules");}
 atomic_write(file,std::string(8192,'x'),&env.paths);require(session.read_binary_03b3f0(file.u16string()).size()==8192,"no 4KiB cap");
 atomic_write(file,"1;2;Oyuncu \xc4\xb0",&env.paths);require(!session.read_binary_03b3f0(file.u16string()).empty(),"UTF-8 profile accepted");
}
void check_application(const fs::path& r){Environment env{paths(r)};
 const auto old=r/"local/anadius/LSX emu/last_avatar";atomic_write(old,"OLD-USER-DATA",nullptr);
 Service service(env);service.start();require(service.is_ready(),"no config ready");require(!fs::exists(env.paths.default_avatar()),"no eager avatar");if(!disk_diagnostics_enabled())require(!fs::exists(env.data_root),"production no-config no app files");
 auto c=service.application().connect();auto profile=request(*c,"<GetProfile index=\"0\"/>");require(profile.find("Persona=\"SURGE\"")!=std::string::npos,"official display");require(profile.find("AvatarId=\"\"")!=std::string::npos,"protocol avatar id preserved");auto resource=image(*c,"1000200030000");require(fs::path(utf8_path(resource))==env.paths.default_avatar(),"own response selects official");require(get(env.paths.default_avatar())==official(),"response materialized exact official");require(get(old)=="OLD-USER-DATA","old data not migrated or deleted");service.stop();
 // Actual application rollback when persistence fails before commit.
 const auto f=paths(r/"failure");atomic_write(f.config(),"\"Config2\"{\"Achievements\"{\"AchievementsSet\" \"fixture\" \"AchievementIDs\" \"a1\" \"SteamAppId\" \"42\"}}",nullptr);Environment broken{f};Service b(broken);b.start();auto client=b.application().connect();fs::create_directories(f.data()/"achievement-fixture.xml");auto before=b.application().configuration().achievements.at("a1").field04;const auto ini=f.data()/"achievement_watcher/42/achievements.ini";require(fs::is_regular_file(ini),"source startup creates missing INI");const auto ini_before=get(ini);rejected([&]{request(*client,"<GrantAchievement AchievementId=\"a1\" Progress=\"1\"/>");},"actual persistence failure propagated");require(b.application().configuration().achievements.at("a1").field04==before,"actual memory rollback");require(get(ini)==ini_before,"failed grant preserves startup INI");b.stop();
}
}
int main(){auto root=fs::temp_directory_path()/("preloader_l_fix13_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));try{check_paths(root/"paths");check_assets(root/"assets");check_atomic(root/"atomic");check_logs(root/"logs");check_cache(root/"cache");check_application(root/"app");fs::remove_all(root);std::cout<<"PASS FIX13 rollback checks="<<checks<<"; disk_diagnostics="<<disk_diagnostics_enabled()<<"\n";return 0;}catch(const std::exception& e){std::cerr<<"FAIL FIX13 rollback: "<<e.what()<<"\n";fs::remove_all(root);return 1;}}
