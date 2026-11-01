#include "storage_test_fixture.hpp"
#include <cctype>

namespace {
using namespace b8_test;
LowerState* process_paths=nullptr;
DWORD copy_path(const Path& p,LPWSTR output,DWORD capacity,bool environment){
    if(p.size()>=capacity){if(!environment)std::copy_n(p.begin(),capacity,output);return static_cast<DWORD>(p.size()+(environment?1:0));}
    std::copy(p.begin(),p.end(),output);output[p.size()]=0;return static_cast<DWORD>(p.size());
}
DWORD WINAPI module_path(HMODULE module,LPWSTR output,DWORD capacity){
    require(!module&&output&&capacity,"full configuration SDK path query keeps current-process and live buffer operands");
    return copy_path(process_paths->executable,output,capacity,false);
}
DWORD WINAPI environment_path(LPCWSTR name,LPWSTR output,DWORD capacity){
    const auto key=std::wstring_view(name);require(key==L"ProgramData"||key==L"LocalAppData","full configuration uses exact retained environment names");
    return copy_path(key==L"ProgramData"?process_paths->program:process_paths->local,output,capacity,true);
}
BOOL WINAPI set_environment(LPCSTR name,LPCSTR value){process_paths->environment.emplace_back(name,value);process_paths->events.emplace_back("environment");return FALSE;}
bootstrap_policies::ConfigPlatformApi config_sdk(){return {module_path,environment_path,::GetLastError,::SetLastError};}
bootstrap_providers::ConfigApi config_crt(){return {&::isspace,&::tolower,set_environment};}
struct Stack {
    std::shared_ptr<LowerState> lower=std::make_shared<LowerState>();
    std::shared_ptr<cs::Filesystem> filesystem=std::make_shared<cs::Filesystem>();
    std::unique_ptr<bootstrap_providers::Configuration> selected;
    cs::ConfigurationPersistence* persistence=nullptr;
    explicit Stack(const std::filesystem::path& root){
        lower->executable=(root/"game"/"host.exe").u16string();lower->program=(root/"program").u16string();lower->local=(root/"data").u16string();process_paths=lower.get();
        auto platform=std::make_unique<bootstrap_policies::ConfigurationPlatform>(std::make_unique<Lower>(lower),std::make_unique<encoding_recovered::NativeCalls>(),std::make_unique<encoding_recovered::PortableErrors>(),config_sdk());
        auto directory=std::make_unique<cs::ConfigurationFilesystem>(filesystem,std::move(platform));
        auto storage=std::make_unique<cs::ConfigurationPersistence>(filesystem,achievement_storage::Paths{},std::move(directory));persistence=storage.get();
        selected=std::make_unique<bootstrap_providers::Configuration>(std::move(storage),std::make_unique<encoding_recovered::NativeCalls>(),std::make_unique<encoding_recovered::PortableErrors>(),config_crt());
    }
    ~Stack(){selected.reset();process_paths=nullptr;}
};
void actual_filesystem(const std::filesystem::path& root){
    cs::Filesystem fs;const auto parent=root/"native-filesystem";fs.create_directories((parent/"nested"/"deep").u16string());
    require(std::filesystem::is_directory(parent/"nested"/"deep"),"actual source recursive directory creation reaches own nested fixture");
    fs.create_directories((parent/"nested"/"deep").u16string());require(std::filesystem::is_directory(parent/"nested"/"deep"),"actual existing-directory status path remains idempotent");
    const auto unicode=parent/"nested"/std::filesystem::path(u"caf\u00e9.txt");write_own(unicode,"own-unicode");
    const auto plain=parent/"nested"/"plain.txt";write_own(plain,"own-plain");const auto entries=fs.directory_entries((parent/"nested").u16string());
    const auto contains=[&](const auto& p){return std::find(entries.begin(),entries.end(),p.u16string())!=entries.end();};
    require(entries.size()==3&&contains(unicode)&&contains(plain)&&contains(parent/"nested"/"deep"),"actual SDK enumeration returns owned Unicode file and directory paths without dot entries");
    fs.remove(unicode.u16string());require(!std::filesystem::exists(unicode)&&std::filesystem::exists(plain),"actual removal deletes only chosen own file");
    fs.remove(unicode.u16string());require(!std::filesystem::exists(unicode),"actual missing-file removal follows normal first-open no-op");
    fs.remove((parent/"nested"/"deep").u16string());require(!std::filesystem::exists(parent/"nested"/"deep"),"actual removal supports selected empty own directory without recursion");
    const auto readonly=parent/"readonly.txt";write_own(readonly,"own-readonly");const std::wstring wreadonly=readonly.wstring();
    require(::SetFileAttributesW(wreadonly.c_str(),FILE_ATTRIBUTE_READONLY)!=FALSE,"own readonly fixture attribute set");
    fs.remove(readonly.u16string());require(!std::filesystem::exists(readonly),"actual native disposition removes selected readonly own file");
    write_own(parent/"blocker","own-file");rejects<std::system_error>([&]{fs.create_directories((parent/"blocker"/"child").u16string());},"existing file blocks real directory recursion with explicit failure");
    require(read_own(parent/"blocker")=="own-file"&&!std::filesystem::exists(parent/"blocker"/"child"),"failed real directory prefix does not modify blocker or invent child");
}
constexpr std::string_view base_cfg=R"("Config2" {"Game" {"Name" "own-game" "Version" "1" "ContentId" "fixture-content" "Language" "tr"} "Emulator" {"SDK" "1.2.3"} "Achievements" {"AchievementsSet" "fixture" "AchievementIDs" "known" "SteamAppId" "42" "SteamIdPrefix" "steam_" "AchievementNames" {"known" "configured-name"} "AchievementTotals" {"known" "5"}}})";
constexpr std::string_view override_cfg=R"("Config2" {"User" {"Username" "override-own-user"}})";
void full_configuration(const std::filesystem::path& root){
    Stack graph(root);const auto game=root/"game";graph.filesystem->create_directories(game.u16string());
    const auto user=root/"data"/"anadius"/"LSX emu";graph.filesystem->create_directories(user.u16string());
    write_own(game/"anadius.cfg",base_cfg);write_own(game/"anadius_override.cfg",override_cfg);
    write_own(user/"achievement-fixture.xml",R"(<root><row Id="known" Count="3" Progress="2" Grant="1970-01-01T00:00:00" Name="xml-name"/><row Id="new" Count="4" Grant="1970-01-01T00:00:02" Name="own-new"/></root>)");
    write_own(user/"remove_AVATAR_fixture.png","own-removal-marker");write_own(user/"keep_avatar_fixture.png","own-lowercase-marker");write_own(user/"keep.txt","own-marker");
    file_recovered::NativeFactory files;configuration::State state{};
    config_startup::initialize_configuration(state,*graph.selected,files);
    require(state.username=="override-own-user"&&state.sdk=="1,2,3"&&state.executable_path==(game/"host.exe").u16string(),"actual FilePass/parser base+override and normalization reach one configuration state");
    require(state.user_directory==user.u16string()&&state.license_directory==(root/"program"/"Electronic Arts"/"EA Services"/"License"/"fixture-content").u16string(),"real source joins concrete fixture process/environment paths before storage");
    require(graph.lower->events==std::vector<std::string>({"log-base","log-override","environment","environment"})&&graph.lower->pngs==0&&graph.lower->persistence==0&&graph.lower->creates==0&&graph.lower->enumerations==0&&graph.lower->removes==0,"all five B8 routes use concrete owners while unselected logging/environment remain explicit");
    require(graph.lower->environment==std::vector<std::pair<std::string,std::string>>({{"ContentId","fixture-content"},{"EAGameLocale","tr"}}),"actual final environment operands occur after selected source storage");
    require(!std::filesystem::exists(user/"remove_AVATAR_fixture.png")&&read_own(user/"keep_avatar_fixture.png")=="own-lowercase-marker"&&read_own(user/"keep.txt")=="own-marker","actual source case-sensitive avatar filter removes only matching own entry");
    const auto user_png=cs::embedded_png(configuration::EmbeddedPng::default_user_1800809d0),avatar_png=cs::embedded_png(configuration::EmbeddedPng::default_avatar_180082710);
    require(read_own(user/"anadius.png")==std::string(reinterpret_cast<const char*>(user_png.data()),user_png.size())&&read_own(user/"default.png")==std::string(reinterpret_cast<const char*>(avatar_png.data()),avatar_png.size()),"actual file backend writes both exact retained PNG payloads under own fixture root");
    const auto& known=state.achievements.at("known");const auto& added=state.achievements.at("new");
    require(known.field00==5&&known.field04==3&&known.field08==2&&known.timestamp=="1970-01-01T00:00:00"&&known.display_name=="configured-name"&&known.steam_id=="steam_known"&&known.field70==0,"actual XML import preserves configured total/name/Steam id while applying progress/grant and recovered epoch");
    require(added.field00==1&&added.field04==4&&added.steam_id.empty()&&added.display_name=="own-new"&&added.field70==2,"actual unknown XML record gets source defaults without retroactive Steam prefix");
    const auto ini=user/"achievement_watcher"/"42"/"achievements.ini";
    const std::string expected="[steam_known]\r\nAchieved=3\r\nCurProgress=2\r\nMaxProgress=5\r\nUnlockTime=0\r\n\r\n[]\r\nAchieved=4\r\nUnlockTime=2\r\n\r\n";
    require(read_own(ini)==expected&&graph.persistence->paths().xml_path==(user/"achievement-fixture.xml").u16string()&&graph.persistence->paths().ini_path==ini.u16string(),"actual source XML/INI graph owns computed path cells and writes independently expected exact CRLF bytes");
    write_own(ini,"preserved-existing-ini");write_own(user/"achievement-fixture.xml",R"(<r><a Id="known" Count="9" Progress="7" Grant="1970-01-01T00:00:03"/></r>)");
    graph.persistence->update_achievement_storage_01eb00(state);
    require(state.achievements.at("known").field04==9&&state.achievements.at("known").field70==3&&read_own(ini)=="preserved-existing-ini","real existing regular INI skips rewrite after XML and timestamp state updates");
    write_own(user/"achievement-fixture.xml","<malformed");const auto before=state.achievements.at("known").field04;graph.persistence->update_achievement_storage_01eb00(state);
    require(state.achievements.at("known").field04==before&&read_own(ini)=="preserved-existing-ini","actual malformed XML takes source parser-error branch without fabricated rollback or INI rewrite");
    const auto missing_parent=root/"never-created"/"asset.png";graph.persistence->write_embedded_png_03c020(missing_parent.u16string(),configuration::EmbeddedPng::default_user_1800809d0);
    require(!std::filesystem::exists(missing_parent.parent_path()),"actual PNG open failure is ignored by original caller without invented mkdir or output");
}
void real_persistence_failure_prefix(const std::filesystem::path& root){
    auto lower=std::make_shared<LowerState>();auto fs=std::make_shared<cs::Filesystem>();const auto user=root/"blocked-persistence";fs->create_directories(user.u16string());
    write_own(user/"achievement_watcher","own-blocker");cs::ConfigurationPersistence persistence(fs,{},std::make_unique<Lower>(lower));
    configuration::State state{};state.user_directory=user.u16string();state.steam_app_id="42";state.achievements["one"]={1,1,0,"1970-01-01T00:00:01",{},"one",88};
    rejects<std::system_error>([&]{persistence.update_achievement_storage_01eb00(state);},"real INI parent blocked by own regular file propagates source filesystem failure");
    require(state.achievements.at("one").field70==1&&!persistence.paths().ini_path.empty()&&read_own(user/"achievement_watcher")=="own-blocker","real failed writer prefix retains earlier epoch/path updates and unchanged blocker");
    const auto cfgroot=root/"blocked-config";std::filesystem::create_directories(cfgroot/"game");std::filesystem::create_directories(cfgroot/"data");write_own(cfgroot/"data"/"anadius","own-parent-blocker");
    write_own(cfgroot/"game"/"anadius.cfg",base_cfg);write_own(cfgroot/"game"/"anadius_override.cfg",override_cfg);
    Stack graph(cfgroot);file_recovered::NativeFactory files;configuration::State cfg{};
    rejects<std::system_error>([&]{config_startup::initialize_configuration(cfg,*graph.selected,files);},"real post-parser user-directory failure propagates through actual five-route configuration stack");
    require(cfg.username=="override-own-user"&&cfg.user_directory==(cfgroot/"data"/"anadius"/"LSX emu").u16string()&&cfg.cookie_path.empty()&&graph.lower->environment.empty(),"real config failure retains parse/path prefix and does not invent later PNG/storage/environment success");
}
}
int main(int argc,char** argv){try{
    const auto root=own_root(argc,argv,"integration");actual_filesystem(root);full_configuration(root);real_persistence_failure_prefix(root);
    std::cout<<"PASS: "<<b8_test::checks<<" configuration-storage integration checks; actual own Win32 directories/removal, source config FilePass and XML-to-INI/PNG IO, persistent state and real failure prefixes; only own synthetic fixtures\n";
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
