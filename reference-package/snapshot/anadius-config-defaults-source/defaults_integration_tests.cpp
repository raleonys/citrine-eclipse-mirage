#include "defaults.hpp"
#include "../anadius-config-storage-source/storage_test_fixture.hpp"
#include "../anadius-xml-source/memory_printer.hpp"
#include <array>

// Explicit defaults opt-in, actual configuration/FilePass/parser/handler source,
// injected path/OS effects and own memory files. No user CFG/profile/license,
// original DLL/game, real filesystem or environment operation is accessed.
namespace {
using namespace anadius;namespace config=configuration;namespace handlers=handlers_recovered;
unsigned checks=0;
void require(bool value,const char* message){++checks;if(!value){std::cerr<<"FAILED: "<<message<<'\n';throw std::runtime_error(message);}}
struct Reader final:xml_recovered::AttributeReader {
    std::map<std::string,std::string,std::less<>> values;
    const char* attribute(handlers::Request,const char* key)override{const auto found=values.find(key);return found==values.end()?nullptr:found->second.c_str();}
};
struct Information final:handlers::Effects {
    std::vector<std::string> profile_logs;unsigned times=0;
    void log_profile_name_084f48(std::string_view value)override{profile_logs.emplace_back(value);}
    std::string current_time_041b60()override{++times;return "OWN-FIXED-TIME";}
};
enum class Response {Profile,Settings,AllGame,Connected,OneGame,OneSetting};
struct Rendered {bool accepted;std::string bytes;};
Rendered render(config::State& state,Response response,Information& effects,std::string key={}){
    xml_recovered::MemoryPrinter printer;Reader reader;reader.values["index"]="0";
    reader.values["GameInfoId"]=key;reader.values["SettingId"]=key;xml_recovered::HandlerWriter writer(printer,reader);
    // Caller-owned projection already required by the existing handler API;
    // this test does not construct or alias a native global/profile object.
    const handlers::Profile profile{state.username,state.persona_id,state.user_id,state.country,state.currency,state.subscriber_level,state.is_subscriber,state.is_under_age,state.is_steam_subscriber};
    const std::uint8_t online=0;handlers::StateReferences refs{state.game_attributes,state.emulator_attributes,profile,online,state.pretend_connected};
    bool accepted=false;
    switch(response){
    case Response::Profile:accepted=handlers::get_profile_02f530(1,writer,refs,effects);break;
    case Response::Settings:accepted=handlers::get_settings_02fa60(writer,refs);break;
    case Response::AllGame:accepted=handlers::get_all_game_info_02da90(writer,refs,effects);break;
    case Response::Connected:accepted=handlers::get_connected_02f2d0(writer,refs);break;
    case Response::OneGame:accepted=handlers::get_game_info_02efd0(1,writer,refs);break;
    case Response::OneSetting:accepted=handlers::get_setting_02f7c0(1,writer,refs);break;
    }
    return {accepted,printer.buffer_including_terminator()};
}
bool contains(const Rendered& response,std::string_view value){return response.accepted&&response.bytes.find(value)!=std::string::npos;}
struct Files {
    std::shared_ptr<b8_test::LowerState> lower_state=std::make_shared<b8_test::LowerState>();b8_test::Lower dependencies{lower_state};
    std::shared_ptr<b8_test::MemoryFilesState> memory=std::make_shared<b8_test::MemoryFilesState>();b8_test::MemoryFiles selected{memory};
    config::Path base=u"C:\\fixture\\anadius.cfg",override_path=u"C:\\fixture\\anadius_override.cfg";
    Files(){memory->inputs[base]="\"Config2\" {}";memory->inputs[override_path]="\"Config2\" {}";}
};
void default_handler_responses(){
    auto state=configuration_defaults::make_configuration_defaults_036d20();Information effects;
    const auto profile=render(state,Response::Profile,effects);
    require(contains(profile,"Persona=\"anadius\"")&&contains(profile,"PersonaId=\"1144668899\"")&&contains(profile,"UserId=\"1000200030000\"")&&contains(profile,"UserIndex=\"0\""),"Native default identity reaches actual GetProfile serializer without a fabricated zero seed");
    require(contains(profile,"Country=\"US\"")&&contains(profile,"CommerceCurrency=\"USD\"")&&contains(profile,"SubscriberLevel=\"2\"")&&contains(profile,"IsSubscriber=\"true\"")&&contains(profile,"IsSteamSubscriber=\"false\""),"Default country/currency/subscription fields produce observed profile attributes");
    const auto settings=render(state,Response::Settings,effects);
    require(contains(settings,"Environment=\"production\"")&&contains(settings,"Language=\"en_US\"")&&contains(settings,"IsIGOAvailable=\"false\"")&&contains(settings,"IsTelemetryEnabled=\"false\"")&&contains(settings,"IsManualOffline=\"false\"")&&contains(settings,"IsIGOEnabled=\"false\""),"All six native emulator defaults survive actual GetSettings lookup and serialization");
    const auto game=render(state,Response::AllGame,effects);
    require(contains(game,"DisplayName=\"Game\"")&&contains(game,"InstalledVersion=\"1.0.0.0\"")&&contains(game,"AvailableVersion=\"1.0.0.0\"")&&contains(game,"EntitlementSource=\"ORIGIN\"")&&contains(game,"SystemTime=\"OWN-FIXED-TIME\""),"Native titlecase Game and version defaults reach existing GetAllGameInfo source");
    require(contains(render(state,Response::OneGame,effects,"EXPIRATION_DURATION"),"GameInfo=\"36000000\"")&&contains(render(state,Response::OneGame,effects,"INSTALLED_LANGUAGE"),"GameInfo=\"en_US\""),"Seeded game keys support actual per-key command lookups without absent-map failure");
    require(contains(render(state,Response::Connected,effects),"connected=\"1\"")&&state.fake_auth==1&&state.server_port==0&&state.sdk=="10,5,96,47512","Offline source with native PretendConnected1 reports connected while preserving independent initial port/SDK/fake-auth fields");
    const auto unknown=render(state,Response::OneGame,effects,"OWN-UNKNOWN-KEY");
    require(!unknown.accepted&&unknown.bytes==std::string(1,'\0')&&state.game_attributes.size()==14,"Unknown command key remains absent and produces no fabricated handler response");
    require(!profile.bytes.empty()&&profile.bytes.back()==0&&effects.profile_logs==std::vector<std::string>{"anadius"}&&effects.times==1,"Existing printer retains final NUL and handler log/time effects use only explicit synthetic providers");
}
void missing_values_through_both_config_sources(){
    auto direct=configuration_defaults::make_configuration_defaults_036d20(),parsed=configuration_defaults::make_configuration_defaults_036d20();Files files;
    direct.executable_path=parsed.executable_path=files.lower_state->executable;config::Node empty;
    config::apply_config2(direct,empty,false,files.dependencies);
    config_startup::FilePass pass(files.selected);pass.load_and_apply(parsed,false,files.dependencies);pass.load_and_apply(parsed,true,files.dependencies);
    require(parsed.username=="anadius"&&parsed.persona_id=="1144668899"&&parsed.user_id=="1000200030000"&&parsed.country=="US"&&parsed.currency=="USD","Real binary-file/parser/hash base and override passes preserve native identity when scalar values are missing");
    require(parsed.game_attributes==direct.game_attributes&&parsed.emulator_attributes==direct.emulator_attributes&&parsed.game_attributes.size()==14&&parsed.emulator_attributes.size()==6,"Actual selected FilePass and semantic apply retain the same native map fallbacks on empty Config2");
    require(parsed.server_port==0&&parsed.pretend_connected==1&&parsed.fake_auth==1&&parsed.sdk=="10,5,96,47512"&&parsed.extra_dlls.empty()&&parsed.content.empty(),"Missing numeric/boolean/SDK/collection inputs retain constructor state rather than introducing zero-seed behavior");
    Information a,b;
    require(render(parsed,Response::Profile,a).bytes==render(direct,Response::Profile,b).bytes&&render(parsed,Response::Settings,a).bytes==render(direct,Response::Settings,b).bytes&&render(parsed,Response::Connected,a).bytes==render(direct,Response::Connected,b).bytes,"Real handler responses agree across both missing-value configuration paths");
    require(files.memory->reads==std::vector<config::Path>{files.base,files.override_path}&&files.memory->writes.empty()&&files.lower_state->events==std::vector<std::string>{"log-base","log-override"},"FilePass reads only own memory CFG bytes in base/override order and bypasses legacy document provider");
    require(parsed.content_id=="0"&&parsed.license_directory.ends_with(u"License\\0")&&parsed.denuvo_dll==u"C:\\fixture\\dbdata.dll","Existing config algorithm derives pure license/DLL paths from native content-id default with no license or module access");
}
void configured_overrides_and_instance_isolation(){
    auto configured=configuration_defaults::make_configuration_defaults_036d20(),independent=configuration_defaults::make_configuration_defaults_036d20();Files files;
    files.memory->inputs[files.base]=R"("Config2" {
      "Game" {"Name" "Configured & Game" "Version" "2.5" "ContentId" "OWN-content" "Languages" "en_US,tr_TR" "Language" "tr_TR"}
      "Emulator" {"SDK" "9.8.7" "ServerPort" "70000tail" "Language" "tr_TR" "PretendConnected" "false" "FakeAuth" "false"}
      "User" {"Username" "Configured & User" "PersonaId" "OWN-persona" "UserId" "OWN-user" "Country" "TR" "Currency" "TRY" "SubscriberLevel" "4" "IsUnderAge" "true" "IsSubscriber" "false"}
      "Content" {"OWN-item" {}}
    })";
    files.memory->inputs[files.override_path]=R"("Config2" {
      "Game" {"Name" "IGNORED-override" "Version" "999" "ContentId" "IGNORED-content" "Language" ""}
      "Emulator" {"Language" "fr_CA" "SDK" "" "ServerPort" "" "FakeAuth" "not-a-bool"}
      "User" {"Username" "Override & User" "Country" "CA" "Currency" ""}
    })";
    config_startup::initialize_configuration(configured,files.dependencies,files.selected);
    require(configured.username=="Override & User"&&configured.persona_id=="OWN-persona"&&configured.user_id=="OWN-user"&&configured.country=="CA"&&configured.currency=="TRY","Actual initialize source applies explicit override fields and preserves earlier nonempty values for empty overrides");
    require(configured.game_attributes.at("DISPLAY_NAME")=="Configured & Game"&&configured.game_attributes.at("INSTALLED_VERSION")=="2.5"&&configured.game_attributes.at("AVAILABLE_VERSION")=="2.5"&&configured.content_id=="OWN-content","Base-only game metadata ignores override Name/Version/ContentId; final source initialization synchronizes available version");
    require(configured.sdk=="9,8,7"&&configured.server_port==4464&&configured.pretend_connected==0&&configured.fake_auth==1,"Existing SDK normalization, low16 decimal port and invalid-boolean fallback remain active after native defaults opt-in");
    Information effects;const auto profile=render(configured,Response::Profile,effects),settings=render(configured,Response::Settings,effects),game=render(configured,Response::AllGame,effects);
    require(contains(profile,"Persona=\"Override &amp; User\"")&&contains(profile,"Country=\"CA\"")&&contains(profile,"CommerceCurrency=\"TRY\"")&&contains(profile,"IsSubscriber=\"false\"")&&contains(profile,"IsUnderAge=\"true\""),"Real profile handler reflects configured fields and existing serializer escapes explicit ampersand");
    require(contains(settings,"Language=\"fr_CA\"")&&contains(settings,"Environment=\"production\"")&&contains(game,"DisplayName=\"Configured &amp; Game\"")&&contains(game,"InstalledLanguage=\"tr_TR\""),"Emulator override language and game installed language remain distinct in actual settings/game responses");
    require(contains(render(configured,Response::Connected,effects),"connected=\"0\"")&&configured.content.size()==1&&configured.content[0].name=="GAME","Explicit PretendConnected false changes source response; base collection fallback GAME remains distinct from default game title Game");
    require(files.memory->reads==std::vector<config::Path>{files.base,files.override_path}&&files.memory->writes.empty()&&files.lower_state->creates==1&&files.lower_state->pngs==2&&files.lower_state->persistence==1&&files.lower_state->enumerations==1,"Actual full initialization runs both memory FilePass reads and only injected directory/PNG/persistence effects");
    require(files.lower_state->environment==std::vector<std::pair<std::string,std::string>>{{"ContentId","OWN-content"},{"EAGameLocale","tr_TR"}},"Final configuration publishes exact explicit content and game locale only through own environment recorder");
    Information separate;const auto untouched_profile=render(independent,Response::Profile,separate),untouched_game=render(independent,Response::AllGame,separate);
    require(contains(untouched_profile,"Persona=\"anadius\"")&&contains(untouched_game,"DisplayName=\"Game\"")&&independent.content.empty()&&independent.emulator_attributes.at("LANGUAGE")=="en_US"&&independent.executable_path.empty(),"Second simultaneously live defaults instance retains native state after complete first-instance configuration and collection mutations");
    independent.game_attributes["DISPLAY_NAME"]="SECOND-instance";
    require(contains(render(independent,Response::OneGame,separate,"DISPLAY_NAME"),"GameInfo=\"SECOND-instance\"")&&contains(render(configured,Response::OneGame,effects,"DISPLAY_NAME"),"GameInfo=\"Configured &amp; Game\""),"Mutating second-instance map changes only its own actual command response");
}
}
int main(){
    try{
        default_handler_responses();missing_values_through_both_config_sources();configured_overrides_and_instance_isolation();
        std::cout<<"PASS: "<<checks<<" configuration defaults integration checks; actual configuration/FilePass and serialized handler responses\n";return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
