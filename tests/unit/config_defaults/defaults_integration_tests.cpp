// SPDX-License-Identifier: GPL-3.0-only

#include "../../support/fixture_ascii.hpp"
#include "preloader_l/runtime/application.hpp"
#include "preloader_l/config/startup.hpp"
#include "preloader_l/xml/memory_printer.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>

// Adapt the supplied constructor integration scenarios to the actual product:
// initial_state -> binary FilePass/PEGTL/Config2 -> handlers -> XML bytes.
// Only external file/path/time effects are fixtures. No user files are read.
namespace {
using namespace preloader_l;
namespace config=configuration;
namespace handlers=handlers;
unsigned checks=0;
void require(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
config::Path widen(std::string_view value){return {value.begin(),value.end()};}
struct Reader final:xml::AttributeReader {
    std::map<std::string,std::string,std::less<>> values;
    const char* attribute(handlers::Request,const char* key)override{
        const auto found=values.find(key);return found==values.end()?nullptr:found->second.c_str();
    }
};
struct Information final:handlers::Effects {
    std::vector<std::string> profiles;unsigned times=0;
    void log_profile_name_084f48(std::string_view value)override{profiles.emplace_back(value);}
    std::string current_time_041b60()override{++times;return "FIXED-TIME";}
};
enum class Response {profile,settings,all_game,connected,one_game,one_setting};
struct Rendered {bool accepted;std::string bytes;};
Rendered render(config::State& state,Response response,Information& effects,std::string key={}){
    xml::MemoryPrinter printer;Reader reader;
    reader.values={{"index","0"},{"GameInfoId",key},{"SettingId",key}};
    xml::HandlerWriter writer(printer,reader);
    const handlers::Profile profile{state.username,state.persona_id,state.user_id,state.country,state.currency,
        state.subscriber_level,state.is_subscriber,state.is_under_age,state.is_steam_subscriber};
    const std::uint8_t online=0;
    handlers::StateReferences refs{state.game_attributes,state.emulator_attributes,profile,online,state.pretend_connected};
    bool accepted=false;
    switch(response){
    case Response::profile:accepted=handlers::get_profile_02f530(1,writer,refs,effects);break;
    case Response::settings:accepted=handlers::get_settings_02fa60(writer,refs);break;
    case Response::all_game:accepted=handlers::get_all_game_info_02da90(writer,refs,effects);break;
    case Response::connected:accepted=handlers::get_connected_02f2d0(writer,refs);break;
    case Response::one_game:accepted=handlers::get_game_info_02efd0(1,writer,refs);break;
    case Response::one_setting:accepted=handlers::get_setting_02f7c0(1,writer,refs);break;
    }
    return {accepted,printer.buffer_including_terminator()};
}
bool contains(const Rendered& response,std::string_view value){return response.accepted&&response.bytes.find(value)!=std::string::npos;}
struct Input final:file::Input {
    std::string bytes;bool missing;
    Input(std::string value,bool absent):bytes(std::move(value)),missing(absent){}
    std::uint32_t initial_state()const override{return missing?1:0;}
    void seek_03b610(std::int64_t offset,file::Seek)override{require(offset==0,"binary FilePass seeks from zero");}
    std::uint64_t position_sum_03ba50()override{return bytes.size();}
    std::int32_t peek_043790()override{throw std::logic_error("unexpected peek");}
    std::size_t read_03b2a0(std::span<std::uint8_t> out)override{
        require(out.size()==bytes.size(),"FilePass reads the recorded binary extent");
        std::copy(bytes.begin(),bytes.end(),out.begin());return bytes.size();
    }
    void close_03ae70()override{}
};
struct Files final:file::Factory {
    std::string base=R"("Config2" {})",over=base;bool missing=false;
    std::vector<config::Path> reads;
    std::unique_ptr<file::Input> open_input_039f70(const config::Path& path,std::uint32_t mode,std::uint32_t share)override{
        require(mode==0x20&&share==0x40,"FilePass preserves binary read mode/share");reads.push_back(path);
        return std::make_unique<Input>(path.ends_with(u"surge_override.cfg")?over:base,missing);
    }
    std::unique_ptr<file::Output> open_output_03a1a0(const config::Path&,std::uint32_t,std::uint32_t)override{
        throw std::logic_error("configuration must not write through the input factory");
    }
};
struct Dependencies final:config::Dependencies {
    std::vector<config::Path> logs;
    std::vector<std::pair<std::string,std::string>> environment;
    unsigned creates=0,pngs=0,persistence=0,enumerations=0;
    config::Path executable_path_03c6d0()override{return u"C:/fixture/host.exe";}
    config::Path parent_path_019820(const config::Path& value)override{
        const auto end=value.find_last_of(u"/\\");return end==config::Path::npos?config::Path{}:value.substr(0,end);
    }
    config::Path join_016b00(const config::Path& a,const config::Path& b)override{return a+u"/"+b;}
    config::Path widen_0145d0(std::string_view value)override{return widen(value);}
    config::Path filesystem_widen_010540(std::string_view value)override{return widen(value);}
    std::string narrow_path(const config::Path& value)override{return preloader_l::test_support::fixture_ascii(value);}
    config::Path resolve_path_0192d0(std::string_view value,const config::Path&,bool relative)override{
        require(relative,"configuration keeps relative-path mode");return u"C:/fixture/"+widen(value);
    }
    config::Path program_data_03c920()override{return u"C:/synthetic-programdata";}
    config::Path local_app_data_03c6f0()override{return u"C:/synthetic-local";}
    config::Node load_document(const config::Path&)override{throw std::logic_error("legacy document adapter bypassed");}
    void log_config_path(const config::Path& value)override{logs.push_back(value);}
    bool crt_is_space(unsigned char c)override{return c==' '||(c>='\t'&&c<='\r');}
    int crt_tolower(int c)override{return c>='A'&&c<='Z'?c+32:c;}
    void apply_license_key_02a830(std::string_view)override{throw std::logic_error("license effect outside fixture");}
    void create_directories_018e00(const config::Path& path)override{
        require(path==u"C:/synthetic-local/SURGE","configuration uses its owned data directory");++creates;
    }
    void write_embedded_png_03c020(const config::Path&,config::EmbeddedPng)override{++pngs;}
    std::vector<config::Path> directory_entries(const config::Path&)override{++enumerations;return {};}
    config::Path filename(const config::Path& value)override{return value;}
    void remove_04948c(const config::Path&)override{throw std::logic_error("no fixture entry to remove");}
    void update_achievement_storage_01eb00(config::State&)override{++persistence;}
    bool set_environment_variable(std::string_view name,std::string_view value)override{
        environment.emplace_back(name,value);return true;
    }
};
void default_responses(){
    auto state=application::initial_state();Information effects;
    const auto profile=render(state,Response::profile,effects);
    require(contains(profile,"Persona=\"SURGE\"")&&contains(profile,"PersonaId=\"1144668899\"")&&
        contains(profile,"UserId=\"1000200030000\"")&&contains(profile,"UserIndex=\"0\""),"constructor identity reaches the serialized profile");
    require(contains(profile,"Country=\"US\"")&&contains(profile,"CommerceCurrency=\"USD\"")&&
        contains(profile,"SubscriberLevel=\"2\"")&&contains(profile,"IsSubscriber=\"true\"")&&
        contains(profile,"IsSteamSubscriber=\"false\""),"constructor account defaults reach the profile handler");
    const auto settings=render(state,Response::settings,effects);
    for(const auto* expected:{"Environment=\"production\"","Language=\"en_US\"","IsIGOAvailable=\"false\"",
        "IsTelemetryEnabled=\"false\"","IsManualOffline=\"false\"","IsIGOEnabled=\"false\""})
        require(contains(settings,expected),"all six default settings reach the real serializer");
    const auto game=render(state,Response::all_game,effects);
    require(contains(game,"DisplayName=\"Game\"")&&contains(game,"InstalledVersion=\"1.0.0.0\"")&&
        contains(game,"AvailableVersion=\"1.0.0.0\"")&&contains(game,"EntitlementSource=\"ORIGIN\"")&&
        contains(game,"SystemTime=\"FIXED-TIME\""),"game defaults and injected time reach the aggregate response");
    require(contains(render(state,Response::one_game,effects,"EXPIRATION_DURATION"),"GameInfo=\"36000000\"")&&
        contains(render(state,Response::one_game,effects,"INSTALLED_LANGUAGE"),"GameInfo=\"en_US\""),"seeded game keys reach per-key responses");
    require(contains(render(state,Response::one_setting,effects,"ENVIRONMENT"),"Setting=\"production\""),"seeded setting reaches per-key response");
    require(contains(render(state,Response::connected,effects),"connected=\"1\"")&&state.server_port==0,
        "default pretend-connected response does not require an already bound listener");
    const auto missing=render(state,Response::one_game,effects,"UNKNOWN-KEY");
    require(!missing.accepted&&missing.bytes==std::string(1,'\0')&&state.game_attributes.size()==14,
        "unknown game key does not fabricate output or mutate the default map");
    require(!profile.bytes.empty()&&profile.bytes.back()==0&&effects.profiles==std::vector<std::string>{"SURGE"}&&effects.times==1,
        "profile logging, time sampling and NUL framing remain attached to actual handler execution");
}
void missing_configuration(){
    for(bool absent:{false,true}){
        auto direct=application::initial_state(),parsed=application::initial_state();Files files;Dependencies deps;files.missing=absent;
        direct.executable_path=parsed.executable_path=deps.executable_path_03c6d0();config::Node empty;
        config::apply_config2(direct,empty,false,deps);
        config_startup::FilePass pass(files);pass.load_and_apply(parsed,false,deps);pass.load_and_apply(parsed,true,deps);
        require(parsed.username=="SURGE"&&parsed.persona_id=="1144668899"&&parsed.user_id=="1000200030000"&&
            parsed.country=="US"&&parsed.currency=="USD","absent and empty base/override CFGs preserve constructor identity");
        require(parsed.game_attributes==direct.game_attributes&&parsed.emulator_attributes==direct.emulator_attributes&&
            parsed.game_attributes.size()==14&&parsed.emulator_attributes.size()==6,"FilePass and direct apply agree on default map fallbacks");
        require(parsed.server_port==0&&parsed.pretend_connected==1&&parsed.fake_auth==1&&parsed.extra_dlls.empty()&&parsed.content.empty(),
            "missing values preserve listener/feature/collection defaults");
        Information a,b;
        for(const auto response:{Response::profile,Response::settings,Response::connected})
            require(render(parsed,response,a).bytes==render(direct,response,b).bytes,"missing-value paths produce identical real response bytes");
        require(files.reads==std::vector<config::Path>{u"C:/fixture/surge.cfg",u"C:/fixture/surge_override.cfg"}&&deps.logs==files.reads,
            "binary base/override reads and successful-pass logs retain their order");
        require(parsed.content_id=="0"&&parsed.license_directory.ends_with(u"License/0")&&parsed.denuvo_dll==u"C:/fixture/dbdata.dll",
            "default content identity reaches derived paths without external file access");
    }
}
void configured_responses_and_isolation(){
    auto configured=application::initial_state(),independent=application::initial_state();Files files;Dependencies deps;
    files.base=R"("Config2" {
      "Game" {"Name" "Configured & Game" "Version" "2.5" "ContentId" "fixture-content" "Languages" "en_US,tr_TR" "Language" "tr_TR"}
      "Emulator" {"SDK" "9.8.7" "ServerPort" "70000tail" "Language" "tr_TR" "PretendConnected" "false" "FakeAuth" "false"}
      "User" {"Username" "Configured & User" "PersonaId" "fixture-persona" "UserId" "fixture-user" "Country" "TR" "Currency" "TRY" "SubscriberLevel" "4" "IsUnderAge" "true" "IsSubscriber" "false"}
      "Content" {"fixture-item" {}}
    })";
    files.over=R"("Config2" {
      "Game" {"Name" "IGNORED" "Version" "999" "ContentId" "IGNORED" "Language" ""}
      "Emulator" {"Language" "fr_CA" "SDK" "" "ServerPort" "" "FakeAuth" "not-a-bool"}
      "User" {"Username" "Override & User" "Country" "CA" "Currency" ""}
    })";
    config_startup::initialize_configuration(configured,deps,files);
    require(configured.username=="Override & User"&&configured.persona_id=="fixture-persona"&&configured.user_id=="fixture-user"&&
        configured.country=="CA"&&configured.currency=="TRY","override fields reach real initialization while empty values retain earlier values");
    require(configured.game_attributes.at("DISPLAY_NAME")=="Configured & Game"&&configured.game_attributes.at("INSTALLED_VERSION")=="2.5"&&
        configured.game_attributes.at("AVAILABLE_VERSION")=="2.5"&&configured.content_id=="fixture-content","base-only fields survive override and final version synchronization");
    require(configured.server_port==0&&configured.pretend_connected==0&&configured.fake_auth==1,
        "obsolete port override remains ignored and invalid boolean keeps the selected fallback policy");
    Information effects;
    const auto profile=render(configured,Response::profile,effects),settings=render(configured,Response::settings,effects),game=render(configured,Response::all_game,effects);
    require(contains(profile,"Persona=\"Override &amp; User\"")&&contains(profile,"Country=\"CA\"")&&contains(profile,"CommerceCurrency=\"TRY\"")&&
        contains(profile,"IsSubscriber=\"false\"")&&contains(profile,"IsUnderAge=\"true\""),"configured profile values survive real XML escaping");
    require(contains(settings,"Language=\"fr_CA\"")&&contains(settings,"Environment=\"production\"")&&
        contains(game,"DisplayName=\"Configured &amp; Game\"")&&contains(game,"InstalledLanguage=\"tr_TR\""),"emulator and game language retain their separate semantics in wire responses");
    require(contains(render(configured,Response::connected,effects),"connected=\"0\"")&&configured.content.size()==1&&configured.content[0].name=="GAME",
        "configured connected state and base collection fallback reach the command state");
    require(files.reads==std::vector<config::Path>{u"C:/fixture/surge.cfg",u"C:/fixture/surge_override.cfg"}&&deps.logs==files.reads&&
        deps.creates==1&&deps.pngs==2&&deps.persistence==1&&deps.enumerations==1,"initialization reads both own CFGs and uses only injected directory/avatar/persistence effects");
    require(deps.environment==std::vector<std::pair<std::string,std::string>>{{"ContentId","fixture-content"},{"EAGameLocale","tr_TR"}},
        "final initialization publishes the configured content and game locale");
    Information separate;
    require(contains(render(independent,Response::profile,separate),"Persona=\"SURGE\"")&&contains(render(independent,Response::all_game,separate),"DisplayName=\"Game\"")&&
        independent.content.empty()&&independent.emulator_attributes.at("LANGUAGE")=="en_US"&&independent.executable_path.empty(),
        "configuring one instance cannot contaminate a simultaneously live default instance");
    independent.game_attributes["DISPLAY_NAME"]="SECOND";
    require(contains(render(independent,Response::one_game,separate,"DISPLAY_NAME"),"GameInfo=\"SECOND\"")&&
        contains(render(configured,Response::one_game,effects,"DISPLAY_NAME"),"GameInfo=\"Configured &amp; Game\""),"each instance serializes only its own mutable map");
}
}
int main()try{
    default_responses();missing_configuration();configured_responses_and_isolation();
    std::cout<<"PASS: "<<checks<<" actual constructor/FilePass/Config2/handler/XML integration checks\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
