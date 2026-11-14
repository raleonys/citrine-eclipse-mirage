// SPDX-License-Identifier: GPL-3.0-only

#include "../../support/fixture_ascii.hpp"
#include "storage_fixture.hpp"
#include "laureondo/config/startup.hpp"

namespace {
struct Configuration final:achievement_storage::ConfigurationDependencies {
    std::vector<std::pair<std::string,std::string>> environment;
    std::vector<std::string> seen_names;
    explicit Configuration(achievement_storage::Storage& storage):ConfigurationDependencies(storage){}
    Path executable_path_03c6d0()override{return u"C:\\fixture\\host.exe";}
    Path parent_path_019820(const Path& p)override{return path::parent_path_019820(p);}
    Path join_016b00(const Path& a,const Path& b)override{return join(a,b);}
    Path widen_0145d0(std::string_view s)override{return wide(s);}
    Path filesystem_widen_010540(std::string_view s)override{return wide(s);}
    std::string narrow_path(const Path& p)override{return laureondo::test_support::fixture_ascii(p);}
    Path resolve_path_0192d0(std::string_view s,const Path&,bool)override{return wide(s);}
    Path program_data_03c920()override{return u"PROGRAM";}
    Path local_app_data_03c6f0()override{return u"LOCAL";}
    configuration::Node load_document(const Path&)override{throw std::logic_error("old document callback bypassed");}
    void log_config_path(const Path&)override{}
    bool crt_is_space(unsigned char c)override{return c==' ' || (c>='\t' && c<='\r');}
    int crt_tolower(int c)override{return c>='A' && c<='Z'?c+32:c;}
    void apply_license_key_02a830(std::string_view)override{throw std::logic_error("license effect forbidden");}
    void create_directories_018e00(const Path&)override{}
    void write_embedded_png_03c020(const Path&,configuration::EmbeddedPng)override{}
    std::vector<Path> directory_entries(const Path&)override{return {};}
    Path filename(const Path& p)override{return p;}
    void remove_04948c(const Path&)override{throw std::logic_error("remove forbidden");}
    bool set_environment_variable(std::string_view key,std::string_view value)override{environment.emplace_back(key,value);return false;}
};
void startup_integration() {
    Fixture f;Configuration deps(f.storage);
    f.files.input[u"C:\\fixture\\surge.cfg"]=R"("Config2" {"Game" {"Version" "1" "ContentId" "fixture-content" "Language" "tr"} "Emulator" {"SDK" "1.2"} "Achievements" {"AchievementsSet" "fixture-set" "AchievementIDs" "known" "SteamAppId" "42" "SteamIdPrefix" "steam_"}})";
    f.files.input[u"C:\\fixture\\surge_override.cfg"]=R"("Config2" {"User" {"Username" "override-user"}})";
    const auto xml=join(join(u"LOCAL",u"SURGE"),u"achievement-fixture-set.xml");
    f.files.input[xml]=R"(<root><entry Id="known" Count="3" Name="from-file" Grant="file-time"/><entry Id="new" Count="2"/></root>)";
    config_startup::initialize_configuration(f.state,deps,f.files);
    require(f.state.username=="override-user","real Config2 startup before persistence");
    require(f.files.reads==std::vector<Path>{u"C:\\fixture\\surge.cfg",u"C:\\fixture\\surge_override.cfg",xml},"base/override then actual persistence XML read");
    require(f.state.achievements.at("known").field04==3 && f.state.achievements.at("known").display_name=="from-file" && f.state.achievements.at("new").field04==2,"final persistence method delegates to actual storage");
    require(f.state.achievements.at("known").steam_id=="steam_known" && f.state.achievements.at("new").steam_id.empty(),"new XML IDs do not retroactively inherit earlier Steam prefix pass");
    require(f.effects.epochs==std::vector<std::string>{"file-time","0000-00-00T00:00:00"} && deps.environment==std::vector<std::pair<std::string,std::string>>{{"ContentId","fixture-content"},{"EAGameLocale","tr"}},"real storage completes before final environment boundaries");
}
}
int main() {
    try {startup_integration();std::cout<<"PASS: "<<checks<<" configuration startup/storage model integration checks\n";}
    catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
