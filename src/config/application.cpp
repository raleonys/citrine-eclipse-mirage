// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/config/application.hpp"
#include "preloader_l/core/product_identity.hpp"

namespace preloader_l::config_application {
namespace {
using namespace configuration;
using HashNode=config_hash::Node;
std::string_view text(HashNode& node,std::string_view key) {return node.lookup(key).value;}
void path_if_nonempty(Path& target,std::string_view value,const Path& executable,Dependencies& deps) {
    if(!value.empty())target=deps.resolve_path_0192d0(value,executable,true);
}
void lowercase(std::string& value,Dependencies& deps) {
    for(char& c:value)c=static_cast<char>(deps.crt_tolower(static_cast<signed char>(c)));
}
std::string defaulted(std::string_view value,const char* fallback) {
    return value.empty()?std::string(fallback):std::string(value);
}
void copy_section(HashNode& section,HashNode& config,const char* key,Trace* trace) {
    section.assign_from(config.lookup(key));
    if(trace)trace->copies.push_back({key,section.table.bucket_count,section.table.traversal});
}
}

config_hash::Node read_config2(const configuration::Path& path,file::Factory& files) {
    const auto bytes=file::read_binary_03b3f0(path,files);
    parser::State parsed;
    (void)parser::parse(bytes,parsed); // Raw bytes; parser removes only one BOM.
    if(const auto* selected=parsed.root.find("Config2"))return config_hash::selected_copy(*selected);
    return {}; // Logical value of original root default-insertion.
}

void apply_config2(configuration::State& state,config_hash::Node& config,
                   bool override_pass,configuration::Dependencies& deps,Trace* trace) {
    using namespace configuration;
    HashNode section,record; // local428 and local288; both persist across sections.
    copy_section(section,config,"Game",trace);
    if(!override_pass) {
        const auto name=text(section,"Name");
        assign_if_nonempty(state.game_attributes["DISPLAY_NAME"],name);
        const auto version=text(section,"Version");
        assign_if_nonempty(state.game_attributes["INSTALLED_VERSION"],version);
        assign_if_nonempty(state.content_id,text(section,"ContentId"));
    }
    const auto wide_content=deps.filesystem_widen_010540(state.content_id);
    auto path=deps.join_016b00(deps.program_data_03c920(),u"Electronic Arts");
    path=deps.join_016b00(path,u"EA Services");
    path=deps.join_016b00(path,u"License");
    state.license_directory=deps.join_016b00(path,wide_content);

    assign_if_nonempty(state.denuvo_token,text(section,"DenuvoToken"));
    if(!state.denuvo_token.empty() && state.denuvo_token.size()<=99)state.denuvo_token.clear();
    assign_if_nonempty(state.exe_hash,text(section,"DenuvoExeHash"));lowercase(state.exe_hash,deps);
    assign_if_nonempty(state.dll_hash,text(section,"DenuvoDllHash"));lowercase(state.dll_hash,deps);
    path_if_nonempty(state.denuvo_dll,text(section,"DenuvoDllName"),state.executable_path,deps);
    if(state.denuvo_dll.empty())path_if_nonempty(state.denuvo_dll,"dbdata.dll",state.executable_path,deps);
    const auto license_key=text(section,"KeyForLicense");
    if(!license_key.empty())deps.apply_license_key_02a830(license_key);
    const auto languages=text(section,"Languages");
    assign_if_nonempty(state.game_attributes["LANGUAGES"],languages);
    apply_registry_path(state,text(section,"LanguageRegistryKey"),deps);
    if(state.language_from_arguments==0) {
        const auto language=text(section,"Language");
        assign_if_nonempty(state.game_attributes["INSTALLED_LANGUAGE"],language);
        apply_boolean(state.registry_spoof,text(section,"LanguageRegistrySpoof"),1);
    }

    copy_section(section,config,"Emulator",trace);
    path_if_nonempty(state.documents,text(section,"Documents"),state.executable_path,deps);
    apply_boolean(state.pretend_connected,text(section,"PretendConnected"),1);
    apply_boolean(state.fake_auth,text(section,"FakeAuth"),1);
    const auto emulator_language=text(section,"Language");
    assign_if_nonempty(state.emulator_attributes["LANGUAGE"],emulator_language);
    apply_boolean(state.block_reruns,text(section,"BlockReruns"),0);
    apply_csv_paths(state.extra_dlls_from_main,text(section,"LoadExtraDLLsFromMain"),state.executable_path,deps);
    apply_csv_paths(state.extra_dlls,text(section,"LoadExtraDLLs"),state.executable_path,deps);

    copy_section(section,config,"User",trace);
    assign_if_nonempty(state.username,text(section,"Username"));
    assign_if_nonempty(state.persona_id,text(section,"PersonaId"));
    assign_if_nonempty(state.user_id,text(section,"UserId"));
    path_if_nonempty(state.avatar,text(section,"Avatar"),state.executable_path,deps);
    apply_boolean(state.use_last_online_profile,text(section,"UseLastOnlineProfile"),0);
    assign_if_nonempty(state.country,text(section,"Country"));
    assign_if_nonempty(state.currency,text(section,"Currency"));
    assign_if_nonempty(state.is_subscriber,text(section,"IsSubscriber"));
    assign_if_nonempty(state.is_steam_subscriber,text(section,"IsSteamSubscriber"));
    assign_if_nonempty(state.subscriber_level,text(section,"SubscriberLevel"));
    assign_if_nonempty(state.is_under_age,text(section,"IsUnderAge"));
    if(override_pass)return;

    copy_section(section,config,"Content",trace);
    for(const auto& id:section.table.traversal) {
        record.assign_from(section.lookup(id));
        const auto name=defaulted(text(record,"Name"),"GAME");
        const auto version=defaulted(text(record,"Version"),"0");
        const auto status=defaulted(text(record,"State"),"INSTALLED");
        state.content.push_back({id,name,version,status});
    }
    copy_section(section,config,"Entitlements",trace);
    for(const auto& id:section.table.traversal) {
        record.assign_from(section.lookup(id));
        const auto group=std::string(text(record,"Group"));
        const auto version=defaulted(text(record,"Version"),"0");
        const auto type=defaulted(text(record,"Type"),"DEFAULT");
        const auto tag=defaulted(text(record,"EntitlementTag"),"ONLINE_ACCESS");
        state.entitlements.push_back({id,group,version,type,tag});
    }
    copy_section(section,config,"Achievements",trace);
    // Lookup order is IDs then Set, before 01da80 (01bd67/01bdce).
    const auto ids=text(section,"AchievementIDs");
    const auto set=text(section,"AchievementsSet");
    apply_achievement_ids(state,set,ids);
    auto& names=section.lookup("AchievementNames");
    for(const auto& id:names.table.traversal) {
        const auto& value=names.lookup(id).value;
        state.achievements[id]={1,0,0,"0000-00-00T00:00:00",value,{},0};
    }
    auto& totals=section.lookup("AchievementTotals");
    for(const auto& id:totals.table.traversal) {
        const auto& value=totals.lookup(id).value;
        const auto found=state.achievements.find(id);
        if(!value.empty() && found!=state.achievements.end())found->second.field00=parse_decimal(value,deps);
    }
    assign_if_nonempty(state.steam_app_id,text(section,"SteamAppId"));
    if(!state.steam_app_id.empty()) {
        const auto prefix=text(section,"SteamIdPrefix");
        for(auto& [id,achievement]:state.achievements)achievement.steam_id=std::string(prefix)+id;
    }
    auto& overrides=section.lookup("SteamIdOverrides");
    for(const auto& id:overrides.table.traversal) {
        const auto& value=overrides.lookup(id).value;
        const auto found=state.achievements.find(id);
        if(!value.empty() && found!=state.achievements.end())found->second.steam_id=value;
    }
}

void load_and_apply(configuration::State& state,bool override_pass,
                    configuration::Dependencies& deps,file::Factory& files,Trace* trace) {
    const auto name=override_pass?product_identity::override_filename:product_identity::config_filename;
    const auto filename=deps.filesystem_widen_010540(name);
    const auto parent=deps.parent_path_019820(state.executable_path);
    const auto path=deps.join_016b00(parent,filename);
    auto selected=read_config2(path,files); // Parser/input destroyed before path log.
    deps.log_config_path(path);
    apply_config2(state,selected,override_pass,deps,trace);
}
} // namespace preloader_l::config_application
