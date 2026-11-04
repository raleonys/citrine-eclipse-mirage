// SPDX-License-Identifier: GPL-3.0-only

#include "../../support/fixture_ascii.hpp"
#include "preloader_l/config/configuration.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace preloader_l::configuration;
namespace {
int assertions = 0;
void require(bool condition, const char* label) {
    ++assertions;
    if (!condition) throw std::runtime_error(label);
}
template<class F> void rejects_runtime(F operation,const char* label) {
    bool caught=false;
    try {operation();} catch(const std::runtime_error&) {caught=true;}
    require(caught,label); // The assertion must be outside the operation's catch.
}
Path utf16(std::string_view text) { return {text.begin(), text.end()}; }
std::string ascii(const Path& path) { return preloader_l::test_support::fixture_ascii(path); }
void fixture_ascii_contract() {
    require(ascii(u"ASCII/fixture")=="ASCII/fixture","ASCII fixture path preserves bytes");
    bool refused=false;
    try { (void)ascii(u"\u00E9"); } catch (const std::invalid_argument&) { refused=true; }
    require(refused,"fixture must reject non-ASCII input rather than silently truncate UTF-16");
}
void setting(Node& root, const char* section, const char* key, std::string value) {
    root.children[section].children[key].value = std::move(value);
}
State seeded_state() {
    State state{}; // SYNTHETIC seed only; not a claim about original binary defaults.
    state.executable_path = u"C:/synthetic/host.exe";
    state.content_id = "old-content";
    state.game_attributes = {{"DISPLAY_NAME","old-game"},{"INSTALLED_VERSION","old-version"},
        {"INSTALLED_LANGUAGE","old-language"},{"LANGUAGES","old-languages"}};
    state.emulator_attributes = {{"LANGUAGE","old-sdk-language"}};
    state.username="old-name"; state.persona_id="old-persona"; state.user_id="old-user";
    state.server_port=7777; state.fake_auth=1; state.pretend_connected=1;
    state.block_reruns=1; state.registry_spoof=1; state.use_last_online_profile=1;
    state.country="old-country"; state.currency="old-currency";
    state.is_subscriber="old-sub"; state.is_steam_subscriber="old-steam-sub";
    state.subscriber_level="old-level"; state.is_under_age="old-age";
    state.extra_dlls={u"old-dll"}; state.extra_dlls_from_main={u"old-main-dll"};
    return state;
}
struct Mock : Dependencies {
    Node base_document, override_document;
    std::vector<std::string> events;
    std::vector<Path> loaded, removed, writes;
    std::vector<std::string> resolved;
    std::vector<std::string> filesystem_widened, utf8_widened;
    std::vector<std::pair<std::string,std::string>> environment;
    std::vector<EmbeddedPng> images;
    std::vector<Path> directory_listing;
    unsigned license_calls=0;
    bool fail_widen=false, fail_directory=false, fail_load=false;
    std::string fail_resolve;
    bool custom_space=false;
    Path executable_path_03c6d0() override { events.emplace_back("executable"); return u"C:/synthetic/host.exe"; }
    Path parent_path_019820(const Path&) override {return u"C:/synthetic";}
    Path join_016b00(const Path& a,const Path& b) override {return a+u"/"+b;}
    Path widen_0145d0(std::string_view text) override {
        utf8_widened.emplace_back(text);
        if(fail_widen) throw std::runtime_error("synthetic conversion failure");
        return utf16(text);
    }
    Path filesystem_widen_010540(std::string_view text) override {
        filesystem_widened.emplace_back(text);return utf16(text);
    }
    std::string narrow_path(const Path& path) override {return ascii(path);}
    Path resolve_path_0192d0(std::string_view text,const Path& executable,bool relative) override {
        require(executable==u"C:/synthetic/host.exe" && relative,"path resolution receives executable and flag");
        if(text==fail_resolve) throw std::runtime_error("synthetic path failure");
        resolved.emplace_back(text);return utf16("RESOLVED/"+std::string(text));
    }
    Path program_data_03c920() override {return u"PROGRAMDATA";}
    Path local_app_data_03c6f0() override {events.emplace_back("localappdata");return u"LOCALAPPDATA";}
    Node load_document(const Path& path) override {
        loaded.push_back(path);events.emplace_back("load");
        if(fail_load) throw std::runtime_error("synthetic parse/I/O failure");
        return path.ends_with(u"surge_override.cfg")?override_document:base_document;
    }
    void log_config_path(const Path&) override {events.emplace_back("config-log");}
    bool crt_is_space(unsigned char c) override {
        return c==' ' || (c>='\t' && c<='\r') || (custom_space && c=='_');
    }
    int crt_tolower(int c) override {return c>='A' && c<='Z'?c+('a'-'A'):c;}
    void apply_license_key_02a830(std::string_view key) override {
        require(key=="synthetic-license-input","synthetic license boundary input");
        ++license_calls;events.emplace_back("license");
    }
    void create_directories_018e00(const Path& path) override {
        require(path==u"LOCALAPPDATA/SURGE","derived user directory");
        events.emplace_back("mkdir");if(fail_directory)throw std::runtime_error("synthetic directory failure");
    }
    void write_embedded_png_03c020(const Path& path,EmbeddedPng image) override {
        writes.push_back(path);images.push_back(image);events.emplace_back("png");
    }
    std::vector<Path> directory_entries(const Path&) override {events.emplace_back("enumerate");return directory_listing;}
    Path filename(const Path& path) override {
        const auto last=path.find_last_of(u"/\\");return last==Path::npos?path:path.substr(last+1);
    }
    void remove_04948c(const Path& path) override {removed.push_back(path);events.emplace_back("remove");}
    void update_achievement_storage_01eb00(State& state) override {
        events.emplace_back("achievement-storage");
    }
    bool set_environment_variable(std::string_view name,std::string_view value) override {
        environment.emplace_back(name,value);events.emplace_back("environment");return false;
    }
};
void helpers() {
    std::uint8_t b=7;
    apply_boolean(b,"",1);require(b==7,"empty bool retains exact old byte");
    for(const auto* text:{"true","1"}) {apply_boolean(b,text,0);require(b==1,"true variants");}
    for(const auto* text:{"false","0"}) {apply_boolean(b,text,1);require(b==0,"false variants");}
    for(const auto* text:{"TRUE","False"," true","1suffix"}) {
        apply_boolean(b,text,7);require(b==7,"unknown bool uses supplied fallback");
    }
    std::string retained="old";assign_if_nonempty(retained,"");require(retained=="old","empty string retains");
    assign_if_nonempty(retained,"new");require(retained=="new","nonempty string replaces");
    Mock m;
    for(const auto& [text,expected]:std::vector<std::pair<std::string,std::int32_t>>{
        {"0",0},{"+42",42},{"-42",-42},{" \t\r\n123suffix",123},{"2147483647",2147483647},
        {"-2147483648",-2147483647-1},{"0x10",0},{"-0",0},{"000007",7}})
        require(parse_decimal(text,m)==expected,"signed decimal conversion");
    for(const auto* text:{""," ","+","-x","abc"}) {
        try{(void)parse_decimal(text,m);require(false,"expected invalid number");}
        catch(const std::invalid_argument& e){require(std::string_view(e.what())=="invalid stoi argument","invalid message");}
    }
    for(const auto* text:{"2147483648","-2147483649","9999999999999999999999999999999999"}) {
        try{(void)parse_decimal(text,m);require(false,"expected range error");}
        catch(const std::out_of_range& e){require(std::string_view(e.what())=="stoi argument out of range","range message");}
    }
    const std::string nul("12\0ignored",10);
    require(parse_decimal(nul,m)==12,"C-string NUL stopping");
    m.custom_space=true;require(parse_decimal("__17",m)==17,"CRT locale whitespace is injected");
    require(split_delimited(",a,,b,",',')==std::vector<std::string_view>{"","a","","b",""},"split empty segments");
    std::vector<Path> paths{u"old"};
    apply_csv_paths(paths,"",u"C:/synthetic/host.exe",m);require(paths==std::vector<Path>{u"old"},"empty CSV retains list");
    apply_csv_paths(paths,",first,, spaced ,",u"C:/synthetic/host.exe",m);
    require(paths==std::vector<Path>{u"RESOLVED/first",u"RESOLVED/ spaced "},"CSV skips empties but preserves whitespace");
    apply_csv_paths(paths,",,",u"C:/synthetic/host.exe",m);require(paths.empty(),"nonempty all-empty CSV clears old list");
    m.fail_resolve="bad";
    rejects_runtime([&]{apply_csv_paths(paths,"good,bad,later",u"C:/synthetic/host.exe",m);},"path failure propagates");
    require(paths==std::vector<Path>{u"RESOLVED/good"},"failed path leaves observed partial vector");
    auto s=seeded_state();s.registry_spoof=0;m.fail_widen=true;
    rejects_runtime([&]{apply_registry_path(s,"synthetic-key",m);},"registry conversion failure propagates");
    require(s.registry_spoof==1 && s.language_registry_key.empty(),"registry flag precedes conversion");
}
Node all_settings() {
    Node n;
    for(auto [key,value]:std::vector<std::pair<const char*,std::string>>{
        {"Name","synthetic-game"},{"Version","synthetic-version"},{"ContentId","synthetic-content"},
        {"DenuvoToken",std::string(100,'T')},{"DenuvoExeHash","AbCdE"},{"DenuvoDllHash","DeFa"},
        {"DenuvoDllName","synthetic-data.dll"},{"KeyForLicense","synthetic-license-input"},
        {"Languages","synthetic-languages"},{"Language","synthetic-game-language"},
        {"LanguageRegistryKey","synthetic-registry-key"},{"LanguageRegistrySpoof","false"}})
        setting(n,"Game",key,std::move(value));
    for(auto [key,value]:std::vector<std::pair<const char*,const char*>>{
        {"Documents","synthetic-documents"},{"ServerPort","70000suffix"},{"PretendConnected","false"},
        {"FakeAuth","false"},{"SDK","1.2.3"},{"Language","synthetic-sdk-language"},{"BlockReruns","false"},
        {"LoadExtraDLLsFromMain","one.dll,two.dll"},{"LoadExtraDLLs","three.dll"}})
        setting(n,"Emulator",key,value);
    for(auto [key,value]:std::vector<std::pair<const char*,const char*>>{
        {"Username","synthetic-name"},{"PersonaId","synthetic-persona"},{"UserId","synthetic-user"},
        {"Avatar","synthetic.png"},{"UseLastOnlineProfile","false"},{"Country","synthetic-country"},
        {"Currency","synthetic-currency"},{"IsSubscriber","synthetic-sub"},{"IsSteamSubscriber","synthetic-steam-sub"},
        {"SubscriberLevel","synthetic-level"},{"IsUnderAge","synthetic-age"}})
        setting(n,"User",key,value);
    auto& content=n.children["Content"].children;
    content["2"].children["Name"].value="named-content";
    content["10"]={};
    // Synthetic captured hash-list traversal deliberately differs from std::map.
    n.children["Content"].iteration_order={"2","10"};
    auto& ent=n.children["Entitlements"].children;
    ent["default"]={};
    auto& e=ent["specific"];
    e.children["Group"].value="group";e.children["Version"].value="v";
    e.children["Type"].value="type";e.children["EntitlementTag"].value="tag";
    n.children["Entitlements"].iteration_order={"default","specific"};
    auto& a=n.children["Achievements"];
    a.children["AchievementsSet"].value="synthetic-set";
    a.children["AchievementIDs"].value="from-id;;overlap;";
    a.children["AchievementNames"].children["overlap"].value="display";
    a.children["AchievementNames"].children["empty-name"].value="";
    a.children["AchievementNames"].iteration_order={"overlap","empty-name"};
    a.children["AchievementTotals"].children["overlap"].value="12tail";
    a.children["AchievementTotals"].children["absent"].value="bad-number-is-not-parsed";
    a.children["AchievementTotals"].children["from-id"].value="";
    a.children["AchievementTotals"].iteration_order={"overlap","absent","from-id"};
    a.children["SteamAppId"].value="synthetic-app-id";
    a.children["SteamIdPrefix"].value="prefix_";
    a.children["SteamIdOverrides"].children["overlap"].value="override-id";
    a.children["SteamIdOverrides"].children["empty-name"].value="";
    a.children["SteamIdOverrides"].children["absent"].value="unused";
    a.children["SteamIdOverrides"].iteration_order={"overlap","empty-name","absent"};
    return n;
}
void all_settings_and_overrides() {
    Mock m;auto s=seeded_state();const auto n=all_settings();
    apply_config2(s,n,false,m);
    require(s.game_attributes["DISPLAY_NAME"]=="synthetic-game" && s.game_attributes["INSTALLED_VERSION"]=="synthetic-version","base game metadata");
    require(s.content_id=="synthetic-content","base ContentId");
    require(s.license_directory==u"PROGRAMDATA/Electronic Arts/EA Services/License/synthetic-content","license path composition");
    require(m.filesystem_widened==std::vector<std::string>{"synthetic-content"} &&
        m.utf8_widened==std::vector<std::string>{"synthetic-registry-key"},
        "ContentId uses filesystem code page, registry uses UTF-8, wide constants require no conversion");
    require(s.denuvo_token.size()==100,"100-byte token retained without copying real token");
    require(s.exe_hash=="abcde" && s.dll_hash=="defa","hash lowercasing");
    require(s.denuvo_dll==u"RESOLVED/synthetic-data.dll" && m.license_calls==1,"path and license boundary");
    require(s.game_attributes["LANGUAGES"]=="synthetic-languages" && s.game_attributes["INSTALLED_LANGUAGE"]=="synthetic-game-language","game languages");
    require(s.language_registry_key==u"synthetic-registry-key" && s.registry_spoof==0,"explicit spoof false overrides registry key flag");
    require(s.documents==u"RESOLVED/synthetic-documents" && s.server_port==7777,"obsolete explicit port ignored");
    require(s.fake_auth==0 && s.pretend_connected==0 && s.block_reruns==0,"emulator false fields");
    require(s.emulator_attributes["LANGUAGE"]=="synthetic-sdk-language","emulator language with obsolete SDK key ignored");
    require(s.extra_dlls_from_main.size()==2 && s.extra_dlls.size()==1,"two separate DLL lists");
    require(s.username=="synthetic-name" && s.persona_id=="synthetic-persona" && s.user_id=="synthetic-user","synthetic identity settings");
    require(s.avatar==u"RESOLVED/synthetic.png" && s.use_last_online_profile==0,"avatar and profile flag");
    require(s.country=="synthetic-country" && s.currency=="synthetic-currency" &&
        s.is_subscriber=="synthetic-sub" && s.is_steam_subscriber=="synthetic-steam-sub" &&
        s.subscriber_level=="synthetic-level" && s.is_under_age=="synthetic-age","user string attributes");
    require(s.content.size()==2 && s.content[0].id=="2" && s.content[0].name=="named-content" && s.content[1].id=="10","adapter hash-list order retained instead of lexical map order");
    require(s.content[1].name=="GAME" && s.content[1].version=="0" && s.content[1].state=="INSTALLED","content fallback literals");
    require(s.entitlements.size()==2 && s.entitlements[0].group.empty() && s.entitlements[0].version=="0" &&
        s.entitlements[0].type=="DEFAULT" && s.entitlements[0].tag=="ONLINE_ACCESS","entitlement defaults");
    require(s.entitlements[1].group=="group" && s.entitlements[1].version=="v" &&
        s.entitlements[1].type=="type" && s.entitlements[1].tag=="tag","explicit entitlement fields");
    require(s.achievements_set=="synthetic-set" && s.achievements.size()==3,"achievement IDs and name insertion");
    const auto& overlap=s.achievements.at("overlap");
    require(overlap.field00==12 && overlap.field04==0 && overlap.field08==0 && overlap.field70==0,"achievement total and reset integers");
    require(overlap.timestamp=="0000-00-00T00:00:00" && overlap.display_name=="display" && overlap.steam_id=="override-id","achievement fields and final override");
    require(s.achievements.at("from-id").field00==1 && s.achievements.at("empty-name").steam_id=="prefix_empty-name","empty totals and overrides preserve");
    require(!s.achievements.contains("absent"),"totals and Steam overrides never create absent record");
    Node override_node=n;setting(override_node,"Game","Name","ignored-name");
    setting(override_node,"Game","Version","ignored-version");setting(override_node,"Game","ContentId","ignored-content");
    setting(override_node,"User","Username","overridden-name");
    apply_config2(s,override_node,true,m);
    require(s.game_attributes["DISPLAY_NAME"]=="synthetic-game" && s.content_id=="synthetic-content","base-only fields survive override");
    require(s.username=="overridden-name","ordinary setting applies in override");
    require(s.content.size()==2 && s.entitlements.size()==2 && s.achievements.size()==3,"all collection blocks skipped during override");
    require(m.license_calls==2,"license side-effect can occur on both passes");
    apply_config2(s,n,false,m);
    require(s.content.size()==4 && s.entitlements.size()==4,"repeated base invocation appends vectors");
}
void edge_conditions() {
    Mock m;auto s=seeded_state();Node empty;
    s.denuvo_token=std::string(99,'X');s.exe_hash="OLDHASH";
    apply_config2(s,empty,true,m);
    require(s.username=="old-name" && s.persona_id=="old-persona" && s.fake_auth==1 && s.server_port==7777,"missing ordinary keys retain seed");
    require(s.denuvo_token.empty() && s.exe_hash=="oldhash","short retained token cleared and retained hash normalized");
    require(s.denuvo_dll==u"RESOLVED/dbdata.dll","empty DLL path receives observed fallback");
    require(s.extra_dlls==std::vector<Path>{u"old-dll"},"missing DLL list retains seed");
    for(std::size_t length:{0U,1U,99U,100U,101U}) {
        Node n;setting(n,"Game","DenuvoToken",std::string(length,'Z'));
        auto seed=seeded_state();seed.denuvo_token.clear();apply_config2(seed,n,true,m);
        require(seed.denuvo_token.size()==(length<=99?0:length),"token length threshold");
    }
    for(const auto* key:{"ServerPort","SDK"})for(const auto* text:{"default","random","-1","65536","invalid","70000suffix"}) {
        Node n;setting(n,"Emulator",key,text);s.server_port=7777;apply_config2(s,n,true,m);
        require(s.server_port==7777,"obsolete fixed setting cannot mutate runtime port or abort application");
    }
    Node cli;setting(cli,"Game","Language","ignored");setting(cli,"Game","LanguageRegistrySpoof","false");
    setting(cli,"Game","LanguageRegistryKey","still-applied");s.language_from_arguments=1;
    apply_config2(s,cli,true,m);
    require(s.game_attributes["INSTALLED_LANGUAGE"]=="old-language" && s.registry_spoof==1 &&
        s.language_registry_key==u"still-applied","CLI lock skips language and spoof but not registry key");
    s.achievements_set="old-set";s.achievements["id"]={9,8,7,"old-time","old-display","old-id",6};
    apply_achievement_ids(s,"","id;new");
    require(s.achievements.at("id").field00==9 && !s.achievements.contains("new"),"empty achievement set suppresses ID processing");
    apply_achievement_ids(s,"new-set","id;;new;");
    require(s.achievements.at("id").field00==1 && s.achievements.at("id").display_name.empty() && s.achievements.at("id").field70==0,"ID processing resets existing record");
    Node no_app;no_app.children["Achievements"].children["SteamIdPrefix"].value="prefix";
    s.steam_app_id.clear();s.achievements["id"].steam_id="retain";
    apply_config2(s,no_app,false,m);require(s.achievements.at("id").steam_id=="retain","no app skips prefix pass");
    s.steam_app_id="retained-app";apply_config2(s,Node{},false,m);
    require(s.achievements.at("id").steam_id=="id","retained app and missing prefix sets raw map key");
}
void collection_order_contract() {
    Node root;root.children["a"]={};root.children["z"]={};
    try{(void)root.ordered_children();require(false,"missing captured order must fail");}
    catch(const std::logic_error&) {require(true,"missing order rejected");}
    for(auto order:std::vector<std::vector<std::string>>{{"a","a"},{"a","missing"}}) {
        root.iteration_order=std::move(order);
        try{(void)root.ordered_children();require(false,"invalid captured order must fail");}
        catch(const std::logic_error&) {require(true,"duplicate or unknown captured key rejected");}
    }
    root.iteration_order={"z","a"};
    const auto ordered=root.ordered_children();
    require(ordered[0]->first=="z" && ordered[1]->first=="a","explicit hash-list order preserved");
    Mock m;auto s=seeded_state();Node config;
    config.children["Content"]=root;
    apply_config2(s,config,false,m);
    require(s.content[0].id=="z" && s.content[1].id=="a","collection order reaches output vector");
    config.children["Content"].iteration_order.clear();
    apply_config2(s,config,true,m);
    require(s.content.size()==2,"override never enumerates base-only collections");

    Node totals;auto& a=totals.children["Achievements"];
    a.children["AchievementsSet"].value="synthetic-order";
    a.children["AchievementIDs"].value="a;z";
    auto& t=a.children["AchievementTotals"];
    t.children["a"].value="invalid";t.children["z"].value="7";t.iteration_order={"z","a"};
    try{apply_config2(s,totals,false,m);require(false,"expected second total failure");}
    catch(const std::invalid_argument&) {}
    require(s.achievements.at("z").field00==7 && s.achievements.at("a").field00==1,
        "captured order preserves partial updates before conversion error");
}
void orchestration() {
    Mock m;auto s=seeded_state();
    m.base_document.children["Config2"]=all_settings();
    m.base_document.children["Config2"].children["User"].children["Avatar"].value.clear();
    setting(m.override_document.children["Config2"],"User","Username","override-user");
    m.directory_listing={u"base/plain.txt",u"base/a_AVATAR_x.png",u"base/lower_avatar_x.png",u"base/_AVATAR_dir"};
    initialize_configuration(s,m);
    require(m.loaded==std::vector<Path>{u"C:/synthetic/surge.cfg",u"C:/synthetic/surge_override.cfg"},"host-adjacent two-pass filename order");
    require(m.filesystem_widened==std::vector<std::string>{"surge.cfg","synthetic-content",
        "surge_override.cfg","synthetic-content","SURGE","remid_cookie",
        "last_profile","last_avatar","SURGE.png","default.png"},"filesystem code-page conversion order");
    require(m.events[0]=="executable" && m.events[1]=="load" && m.events[2]=="config-log","module path and config read/log order");
    require(s.username=="override-user","override applied with obsolete SDK key ignored");
    require(s.cookie_path==u"LOCALAPPDATA/SURGE/remid_cookie" &&
        s.last_profile_path==u"LOCALAPPDATA/SURGE/last_profile" &&
        s.last_avatar_path==u"LOCALAPPDATA/SURGE/last_avatar","per-user paths");
    require(m.writes==std::vector<Path>{u"LOCALAPPDATA/SURGE/SURGE.png",u"LOCALAPPDATA/SURGE/default.png"},"default avatar write order");
    require(m.images==std::vector<EmbeddedPng>{EmbeddedPng::default_user_1800809d0,EmbeddedPng::default_avatar_180082710},"embedded asset identity");
    require(m.removed==std::vector<Path>{u"base/a_AVATAR_x.png",u"base/_AVATAR_dir"},"case-sensitive avatar substring cleanup no extension filtering");
    require(s.game_attributes["AVAILABLE_VERSION"]=="synthetic-version","available version copies installed");
    require(m.environment==std::vector<std::pair<std::string,std::string>>{
        {"ContentId","synthetic-content"},{"EAGameLocale","synthetic-game-language"}},"environment calls ordered even false result");
    require(m.events[m.events.size()-3]=="achievement-storage" && m.events.back()=="environment","achievement persistence before environment");
    {Mock custom;auto st=seeded_state();st.avatar=u"custom-avatar";
     initialize_configuration(st,custom);require(custom.images.size()==1 && st.avatar==u"custom-avatar","custom avatar avoids user-default write only");}
    {Mock fail;auto st=seeded_state();fail.fail_directory=true;
     rejects_runtime([&]{initialize_configuration(st,fail);},"directory failure propagates");
     require(fail.loaded.size()==2 && fail.writes.empty() && fail.environment.empty(),"directory failure after configs blocks later work");}
    {Mock fail;auto st=seeded_state();fail.fail_load=true;
     rejects_runtime([&]{initialize_configuration(st,fail);},"load failure propagates");
     require(fail.loaded.size()==1 && fail.events.size()==2,"load failure blocks log override and filesystem work");}
}
void streaming_cleanup() {
    struct Stream final:Mock {
        std::vector<std::string> trace;
        int advances=0,vectors=0;
        bool fail_next=false;
        std::string failure;
        std::vector<Path> directory_entries(const Path&) override{++vectors;throw std::logic_error("stream was collected");}
        void visit_directory_entries(const Path&,DirectoryVisitor visitor) override {
            if(!visitor.visit)throw std::invalid_argument("missing visitor");
            for(const auto& entry:directory_listing){
                trace.push_back("visit");visitor.visit(visitor.context,entry);
                trace.push_back("next");++advances;
                if(fail_next)throw std::runtime_error("late enumeration failure");
            }
        }
        Path filename(const Path& p) override {
            trace.push_back("filename");if(failure=="filename")throw std::runtime_error(failure);return Mock::filename(p);
        }
        std::string narrow_path(const Path& p) override {
            trace.push_back("narrow");if(failure=="narrow")throw std::runtime_error(failure);return Mock::narrow_path(p);
        }
        void remove_04948c(const Path& p) override {
            trace.push_back("remove");if(failure=="remove")throw std::runtime_error(failure);Mock::remove_04948c(p);
        }
    };
    {
        Stream m;auto s=seeded_state();m.directory_listing={u"first_AVATAR_x.png",u"later_AVATAR_y.png"};m.fail_next=true;
        bool failed=false;try{initialize_configuration(s,m);}catch(const std::runtime_error&){failed=true;}
        require(failed&&m.removed==std::vector<Path>{u"first_AVATAR_x.png"},"late iteration failure retains completed first removal");
        require(m.trace==std::vector<std::string>{"visit","filename","narrow","remove","next"},"cleanup before advancement");
        require(m.vectors==0&&m.advances==1&&m.environment.empty(),"streaming initialization bypasses collection and stops later effects");
        require(s.game_attributes.find("AVAILABLE_VERSION")==s.game_attributes.end(),"late failure precedes available-version publication");
    }
    for(const auto* failure:{"filename","narrow","remove"}) {
        Stream m;auto s=seeded_state();m.directory_listing={u"first_AVATAR_x.png",u"later_AVATAR_y.png"};m.failure=failure;
        bool failed=false;try{initialize_configuration(s,m);}catch(const std::runtime_error&){failed=true;}
        require(failed&&m.advances==0&&m.removed.empty()&&m.environment.empty(),"callback failure prevents next entry and later effects");
    }
    Mock legacy;legacy.directory_listing={u"one_AVATAR_x",u"lower_avatar_x",u"_AVATAR_directory"};
    cleanup_cached_avatars(u"fixture",legacy);
    require(legacy.removed==std::vector<Path>{u"one_AVATAR_x",u"_AVATAR_directory"},"legacy vector fallback retains exact case-sensitive filter");
    bool rejected=false;try{legacy.visit_directory_entries(u"fixture",{});}catch(const std::invalid_argument&){rejected=true;}
    require(rejected&&std::count(legacy.events.begin(),legacy.events.end(),"enumerate")==1,"null visitor rejected before directory enumeration");
}
} // namespace
int main() {
    try {fixture_ascii_contract();helpers();all_settings_and_overrides();edge_conditions();collection_order_contract();orchestration();streaming_cleanup();
        std::cout<<"PASS: "<<assertions<<" synthetic/mock configuration assertions. No original DLL/EXE/CFG execution, environment writes or filesystem effects.\n";return 0;}
    catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}
}
