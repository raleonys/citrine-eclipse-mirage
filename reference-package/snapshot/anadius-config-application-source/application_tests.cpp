#include "application.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

using namespace anadius;
namespace {
int checks=0;
void require(bool ok,const char* label){++checks;if(!ok)throw std::runtime_error(label);}
template<class E,class F>void rejects(F f,const char* label){bool caught=false;try{f();}catch(const E&){caught=true;}require(caught,label);}
configuration::Path wide(std::string_view s){return {s.begin(),s.end()};}
struct Effects final:configuration::Dependencies {
    std::vector<configuration::Path> logs;
    bool fail_log=false;
    std::string fail_path;
    configuration::State* state_for_callback=nullptr;
    configuration::Path executable_path_03c6d0()override{throw std::logic_error("not invoked");}
    configuration::Path parent_path_019820(const configuration::Path& p)override{require(p==u"C:/fixture/host.exe","state executable path");return u"C:/fixture";}
    configuration::Path join_016b00(const configuration::Path& a,const configuration::Path& b)override{return a+u"/"+b;}
    configuration::Path widen_0145d0(std::string_view s)override{if(state_for_callback)state_for_callback->language_from_arguments=1;return wide(s);}
    configuration::Path filesystem_widen_010540(std::string_view s)override{return wide(s);}
    std::string narrow_path(const configuration::Path&)override{throw std::logic_error("not invoked");}
    configuration::Path resolve_path_0192d0(std::string_view s,const configuration::Path&,bool relative)override{
        require(relative,"relative path flag");if(s==fail_path)throw std::runtime_error("synthetic path failure");return wide(s);
    }
    configuration::Path program_data_03c920()override{return u"SYNTHETIC";}
    configuration::Path local_app_data_03c6f0()override{throw std::logic_error("not invoked");}
    configuration::Node load_document(const configuration::Path&)override{throw std::logic_error("old adapter bypassed");}
    void log_config_path(const configuration::Path& p)override{logs.push_back(p);if(fail_log)throw std::runtime_error("synthetic log failure");}
    bool crt_is_space(unsigned char c)override{return c==' ' || (c>='\t' && c<='\r');}
    int crt_tolower(int c)override{return c>='A' && c<='Z'?c+32:c;}
    void apply_license_key_02a830(std::string_view)override{throw std::logic_error("licensing effect forbidden");}
    void create_directories_018e00(const configuration::Path&)override{throw std::logic_error("OS effect forbidden");}
    void write_embedded_png_03c020(const configuration::Path&,configuration::EmbeddedPng)override{throw std::logic_error("OS effect forbidden");}
    std::vector<configuration::Path> directory_entries(const configuration::Path&)override{throw std::logic_error("OS effect forbidden");}
    configuration::Path filename(const configuration::Path&)override{throw std::logic_error("not invoked");}
    void remove_04948c(const configuration::Path&)override{throw std::logic_error("OS effect forbidden");}
    void update_achievement_storage_01eb00(configuration::State&)override{throw std::logic_error("OS effect forbidden");}
    bool set_environment_variable(std::string_view,std::string_view)override{throw std::logic_error("OS effect forbidden");}
};
struct Input final:file_recovered::Input {
    std::string bytes;bool fail=false;std::uint32_t initial=0;
    Input(std::string b,bool f,std::uint32_t i):bytes(std::move(b)),fail(f),initial(i){}
    std::uint32_t initial_state()const override{return initial;}
    void seek_03b610(std::int64_t offset,file_recovered::Seek)override{require(offset==0,"seek zero");}
    std::uint64_t position_sum_03ba50()override{return bytes.size();}
    std::int32_t peek_043790()override{throw std::logic_error("unexpected peek");}
    std::size_t read_03b2a0(std::span<std::uint8_t> into)override{
        if(fail)throw std::runtime_error("synthetic read failure");std::copy(bytes.begin(),bytes.end(),into.begin());return bytes.size();
    }
    void close_03ae70()override{}
};
struct Files final:file_recovered::Factory {
    std::string base,over;bool fail=false;std::uint32_t initial=0;
    std::vector<configuration::Path> opened;
    std::unique_ptr<file_recovered::Input> open_input_039f70(const configuration::Path& p,std::uint32_t mode,std::uint32_t share)override{
        require(mode==0x20 && share==0x40,"binary open mode/share");opened.push_back(p);
        return std::make_unique<Input>(p.ends_with(u"anadius_override.cfg")?over:base,fail,initial);
    }
    std::unique_ptr<file_recovered::Output> open_output_03a1a0(const configuration::Path&,std::uint32_t,std::uint32_t)override{throw std::logic_error("write forbidden");}
};
configuration::State seed(){configuration::State s{};s.executable_path=u"C:/fixture/host.exe";s.username="retained";s.server_port=7777;return s;}
config_hash::Node parsed(std::string_view raw){parser::State p;(void)parser::parse(raw,p);const auto* n=p.root.find("Config2");require(n!=nullptr,"synthetic Config2 present");return config_hash::selected_copy(*n);}
std::vector<std::string> content_ids(const configuration::State& state){std::vector<std::string> ids;for(const auto& row:state.content)ids.push_back(row.id);return ids;}
const config_application::CopyEvent& event(const config_application::Trace& trace,std::string_view section){for(const auto& e:trace.copies)if(e.section==section)return e;throw std::logic_error("missing trace event");}
void scalar_and_collections() {
    auto n=parsed(R"("Config2" {
      "Game" {"Name" "fixture-game" "Version" "2" "ContentId" "fixture-content" "Languages" "fixture-languages" "Language" "tr_TR" "LanguageRegistryKey" "fixture-registry" "LanguageRegistrySpoof" "false"}
      "Emulator" {"Documents" "fixture-docs" "ServerPort" "70000suffix" "SDK" "1.2.3" "PretendConnected" "false" "FakeAuth" "false" "Language" "fixture-language" "BlockReruns" "false" "LoadExtraDLLsFromMain" "first.dll,second.dll" "LoadExtraDLLs" "third.dll"}
      "User" {"Username" "fixture-name" "PersonaId" "fixture-persona" "UserId" "fixture-user" "Avatar" "fixture.png" "UseLastOnlineProfile" "false" "Country" "TR" "Currency" "TRY" "IsSubscriber" "fixture-sub" "IsSteamSubscriber" "fixture-steam" "SubscriberLevel" "fixture-level" "IsUnderAge" "false"}
      "Content" {"a" {"Name" "named-content"} "!" {}}
      "Entitlements" {"a" {"Group" "fixture-group" "Version" "v" "Type" "fixture-type" "EntitlementTag" "fixture-tag"} "!" {}}
      "Achievements" {"AchievementsSet" "fixture-set" "AchievementIDs" "a;!;from-id" "AchievementNames" {"a" "display-a" "!" "display-b"} "AchievementTotals" {"a" "12tail" "!" "7" "absent" "invalid-skipped"} "SteamAppId" "fixture-app" "SteamIdPrefix" "prefix_" "SteamIdOverrides" {"a" "explicit-id" "!" "" "absent" "unused"}}
    })");
    auto s=seed();Effects deps;config_application::Trace trace;
    config_application::apply_config2(s,n,false,deps,&trace);
    require(s.game_attributes.at("DISPLAY_NAME")=="fixture-game" && s.content_id=="fixture-content","game scalar metadata");
    require(s.license_directory==u"SYNTHETIC/Electronic Arts/EA Services/License/fixture-content","derived path");
    require(s.registry_spoof==0 && s.language_registry_key==u"fixture-registry" && s.game_attributes.at("INSTALLED_LANGUAGE")=="tr_TR","registry/language helpers");
    require(s.documents==u"fixture-docs" && s.server_port==4464 && s.sdk=="1.2.3","emulator values and low16 conversion");
    require(s.fake_auth==0 && s.pretend_connected==0 && s.block_reruns==0,"boolean values");
    require(s.extra_dlls_from_main.size()==2 && s.extra_dlls.size()==1,"CSV helpers");
    require(s.username=="fixture-name" && s.persona_id=="fixture-persona" && s.user_id=="fixture-user" && s.avatar==u"fixture.png","user values");
    require(s.country=="TR" && s.currency=="TRY" && s.is_subscriber=="fixture-sub" && s.is_steam_subscriber=="fixture-steam" && s.subscriber_level=="fixture-level","remaining user scalars");
    require(trace.copies.size()==6 && event(trace,"Content").bucket_count==64,"reused section grown by scalar missing-key lookups");
    require(content_ids(s)==std::vector<std::string>{"!","a"},"actual Content order after reused64bucket copy");
    require(s.content[0].name=="GAME" && s.content[0].version=="0" && s.content[0].state=="INSTALLED" && s.content[1].name=="named-content","content defaults and explicit fields");
    require(s.entitlements.size()==2 && s.entitlements[0].id=="!" && s.entitlements[0].type=="DEFAULT" && s.entitlements[0].tag=="ONLINE_ACCESS","entitlements ordered defaults");
    require(s.entitlements[1].group=="fixture-group" && s.entitlements[1].version=="v" && s.entitlements[1].type=="fixture-type" && s.entitlements[1].tag=="fixture-tag","entitlements explicit values");
    require(s.achievements.size()==3 && s.achievements_set=="fixture-set","IDs and names merged");
    require(s.achievements.at("a").field00==12 && s.achievements.at("!").field00==7,"achievement totals from source traversal");
    require(s.achievements.at("a").display_name=="display-a" && s.achievements.at("a").steam_id=="explicit-id" && s.achievements.at("!").steam_id=="prefix_!","names/prefix/override behavior");
    require(s.achievements.at("a").timestamp=="0000-00-00T00:00:00" && s.achievements.at("a").field70==0,"reset fields");
    require(!s.achievements.contains("absent"),"totals/overrides never create absent achievement");
    config_application::Trace again;config_application::apply_config2(s,n,false,deps,&again);
    require(s.content.size()==4 && s.entitlements.size()==4,"base invocation appends collections");
}
void capacity_and_override() {
    std::string game;
    for(int i=0;i<70;++i)game+="\"extra"+std::to_string(i)+"\" \"fixture\" ";
    auto small=parsed(R"("Config2" {"Content" {"a" {} "!" {}}})");
    auto large=parsed("\"Config2\" {\"Game\" {"+game+"} \"Content\" {\"a\" {} \"!\" {}}}");
    Effects deps;auto a=seed(),b=seed();config_application::Trace ta,tb;
    config_application::apply_config2(a,small,false,deps,&ta);config_application::apply_config2(b,large,false,deps,&tb);
    require(event(ta,"Content").bucket_count==64 && event(tb,"Content").bucket_count==128,"fresh70-key Game assignment rounds to128 then persists across smaller sections");
    require(content_ids(a)==std::vector<std::string>{"!","a"} && content_ids(b)==std::vector<std::string>{"a","!"},"same Content source changes order with prior retained capacity");
    auto over=parsed(R"("Config2" {"Game" {"Name" "ignored" "Language" "skip-language" "LanguageRegistrySpoof" "false"} "User" {"Username" "override-name"} "Content" {"item" {}} "Achievements" {"AchievementTotals" {"a" "invalid"}}})");
    b.language_from_arguments=1;b.registry_spoof=7;b.game_attributes["INSTALLED_LANGUAGE"]="retained-language";
    const auto count=b.content.size();config_application::Trace t;
    config_application::apply_config2(b,over,true,deps,&t);
    require(t.copies.size()==3 && b.content.size()==count,"override stops after User; collections accepted and skipped");
    require(b.username=="override-name" && b.game_attributes["DISPLAY_NAME"].empty(),"override ordinary/base-only behavior");
    require(b.registry_spoof==7 && b.game_attributes["INSTALLED_LANGUAGE"]=="retained-language","CLI language lock");
    auto dynamic=parsed(R"("Config2" {"Game" {"LanguageRegistryKey" "fixture-registry" "Language" "must-skip"}})");
    auto d=seed();d.game_attributes["INSTALLED_LANGUAGE"]="before-callback";deps.state_for_callback=&d;
    config_application::apply_config2(d,dynamic,true,deps);deps.state_for_callback=nullptr;
    require(d.language_from_arguments==1 && d.game_attributes["INSTALLED_LANGUAGE"]=="before-callback","later conditional observes actual state after provider callback");
    std::string content;
    for(int i=0;i<70;++i)content+="\"id"+std::to_string(i)+"\" {} ";
    auto many=parsed("\"Config2\" {\"Content\" {"+content+"}}");auto m=seed();
    config_application::apply_config2(m,many,false,deps);
    require(m.content.size()==70,"more than8 collection records applied");
    std::vector<std::string> ids=content_ids(m);std::sort(ids.begin(),ids.end());
    require(std::adjacent_find(ids.begin(),ids.end())==ids.end(),"grown collection has no duplicate/lost records");
}
void partial_failure() {
    Effects deps;auto s=seed();
    auto invalid=parsed(R"("Config2" {"Game" {"Name" "before-error"} "Emulator" {"ServerPort" "invalid"} "User" {"Username" "must-not-apply"} "Content" {"item" {}}})");
    config_application::Trace t;
    rejects<std::invalid_argument>([&]{config_application::apply_config2(s,invalid,false,deps,&t);},"bad numeric port propagates");
    require(s.game_attributes.at("DISPLAY_NAME")=="before-error" && s.username=="retained" && s.content.empty() && t.copies.size()==2,"actual execution preserves partial state before error");
    auto totals=parsed(R"("Config2" {"Achievements" {"AchievementNames" {"a" "first" "i" "second"} "AchievementTotals" {"a" "7" "i" "invalid"}}})");
    s=seed();rejects<std::invalid_argument>([&]{config_application::apply_config2(s,totals,false,deps);},"ordered total failure propagates");
    // Two firstcopy/sectioncopy rebuilds after parsed insert: a/i bucket8 collide.
    require(s.achievements.at("i").field00==1 && s.achievements.at("a").field00==1,"failure follows final child traversal, not lexical order");
    std::string fields;for(int i=0;i<65;++i)fields+="\"field"+std::to_string(i)+"\" \"fixture\" ";
    auto reused=parsed("\"Config2\" {\"Entitlements\" {\"first\" {"+fields+"} \"second\" {"+fields+"}} \"Achievements\" {\"AchievementNames\" {\"a\" \"first\" \"i\" \"second\"} \"AchievementTotals\" {\"a\" \"7\" \"i\" \"invalid\"}}}");
    s=seed();rejects<std::invalid_argument>([&]{config_application::apply_config2(s,reused,false,deps);},"reused descendant totals failure");
    require(s.achievements.at("a").field00==7 && s.achievements.at("i").field00==1,"positional reuse retains512bucket child table and changes partial-update order");
}
void file_path_and_native(const std::filesystem::path& dir) {
    Files files;Effects deps;auto s=seed();config_application::Trace trace;
    files.base="\xef\xbb\xbf\"Config2\" {\"User\" {\"Username\" \"base-user\"} \"Content\" {\"a\" {} \"!\" {}}}";
    files.over=R"("Config2" {"User" {"Username" "override-user"} "Content" {"ignored" {}}})";
    config_application::load_and_apply(s,false,deps,files,&trace);
    config_application::load_and_apply(s,true,deps,files);
    require(s.username=="override-user" && s.content.size()==2,"full raw-byte pipeline supports base collections and override skip");
    require(files.opened==std::vector<configuration::Path>{u"C:/fixture/anadius.cfg",u"C:/fixture/anadius_override.cfg"} && deps.logs==files.opened,"file paths/log contract");
    const auto logs=deps.logs.size();files.base="\xef\xbb\xbf\xef\xbb\xbf\"Config2\" {}";
    rejects<parser::ParseError>([&]{config_application::load_and_apply(s,false,deps,files);},"BOM only once");
    require(deps.logs.size()==logs && s.username=="override-user","parse exception precedes log/apply");
    files.base=R"("Config2" {})";files.fail=true;
    rejects<std::runtime_error>([&]{config_application::load_and_apply(s,false,deps,files);},"read exception propagates");
    require(deps.logs.size()==logs,"I/O exception precedes log");files.fail=false;
    deps.fail_log=true;rejects<std::runtime_error>([&]{config_application::load_and_apply(s,false,deps,files);},"log exception precedes apply");
    require(s.username=="override-user","log failure leaves scalars");deps.fail_log=false;
    files.initial=1;auto empty=config_application::read_config2(u"ignored",files);require(empty.children.empty(),"initial-state failure empty value");files.initial=0;
    std::filesystem::create_directories(dir);const auto path=dir/"application-generated-fixture.cfg";
    {std::ofstream out(path,std::ios::binary|std::ios::trunc);out<<files.over;require(static_cast<bool>(out),"generated native fixture written");}
    file_recovered::NativeFactory native;auto own=config_application::read_config2(path.u16string(),native);
    require(own.lookup("User").lookup("Username").value=="override-user","native own-file stream/parser/copy");
    auto state=seed();config_application::apply_config2(state,own,false,deps);require(state.content.size()==1,"native own fixture reaches collection application");
}
}
int main(int argc,char** argv) {
    try {if(argc!=2)throw std::invalid_argument("requires own fixture directory");scalar_and_collections();capacity_and_override();partial_failure();file_path_and_native(std::filesystem::path(argv[1]));std::cout<<"PASS: "<<checks<<" Config2 application checks; no original execution or real user CFG\n";}
    catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
