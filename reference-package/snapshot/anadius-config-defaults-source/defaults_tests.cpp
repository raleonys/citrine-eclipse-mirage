#include "defaults.hpp"
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>

// Own EXE allocations only. Exhaust each real factory allocation point; track
// live allocations even when their destruction happens after the armed scope.
namespace allocation {
struct alignas(std::max_align_t) Header {bool tracked;};
bool armed=false;std::size_t remaining=0,live=0,attempts=0;
void* acquire(std::size_t size){
    if(armed){++attempts;if(--remaining==0)throw std::bad_alloc();}
    if(size>std::numeric_limits<std::size_t>::max()-sizeof(Header))throw std::bad_alloc();
    auto* p=static_cast<Header*>(std::malloc(sizeof(Header)+(size?size:1)));
    if(!p)throw std::bad_alloc();p->tracked=armed;if(p->tracked)++live;return p+1;
}
void release(void* memory)noexcept{if(memory){auto* p=static_cast<Header*>(memory)-1;if(p->tracked)--live;std::free(p);}}
}
void* operator new(std::size_t n){return allocation::acquire(n);}
void* operator new[](std::size_t n){return allocation::acquire(n);}
void operator delete(void* p)noexcept{allocation::release(p);}
void operator delete[](void* p)noexcept{allocation::release(p);}
void operator delete(void* p,std::size_t)noexcept{allocation::release(p);}
void operator delete[](void* p,std::size_t)noexcept{allocation::release(p);}
namespace {
unsigned checks=0;
void require(bool b,const char* text){++checks;if(!b)throw std::runtime_error(text);}
void native_snapshot(){
    const auto s=anadius::configuration_defaults::make_configuration_defaults_036d20();
    require(s.executable_path.empty()&&s.user_directory.empty()&&s.cookie_path.empty()&&s.last_profile_path.empty()&&s.last_avatar_path.empty()&&s.avatar.empty()&&s.default_avatar.empty(),"036d20 path slots00..d0 are empty independently owned paths");
    require(s.language_from_arguments==0&&s.registry_spoof==0&&s.language_registry_key.empty(),"packed language flags and registry path match native offsetsf0/f1/f8");
    require(s.content_id=="0"&&s.license_directory.empty()&&s.denuvo_token.empty()&&s.exe_hash.empty()&&s.dll_hash.empty()&&s.denuvo_dll.empty(),"native content ID zero is a one-byte string; license/hash/path fields remain empty");
    require(s.username=="anadius"&&s.persona_id=="1144668899"&&s.user_id=="1000200030000","decoded native string immediates map to three distinct modeled profile fields");
    require(s.use_last_online_profile==0&&s.fake_auth==1&&s.pretend_connected==1&&s.server_port==0&&s.block_reruns==0,"native packed10100 and offsets2b0/2b2 preserve exact boolean and zero-port defaults");
    require(s.sdk=="10,5,96,47512"&&s.country=="US"&&s.currency=="USD"&&s.is_subscriber=="true"&&s.is_steam_subscriber=="false"&&s.subscriber_level=="2"&&s.is_under_age=="false","native SDK, locale and subscription text uses exact bytes without boolean conversion");
    require(s.content.empty()&&s.entitlements.empty()&&s.achievements_set.empty()&&s.achievements.empty()&&s.steam_app_id.empty()&&s.extra_dlls_from_main.empty()&&s.extra_dlls.empty()&&s.documents.empty(),"remaining modeled vectors/maps/strings/path are initially empty, including post-cipher fields");
    require(s.emulator_attributes.size()==6&&s.game_attributes.size()==14,"native initializer constructs six emulator and fourteen game entries without extras");
    require(s.emulator_attributes.at("ENVIRONMENT")=="production"&&s.emulator_attributes.at("LANGUAGE")=="en_US"&&s.emulator_attributes.at("IS_IGO_AVAILABLE")=="false"&&s.emulator_attributes.at("IS_IGO_ENABLED")=="false"&&s.emulator_attributes.at("IS_MANUAL_OFFLINE")=="false"&&s.emulator_attributes.at("IS_TELEMETRY_ENABLED")=="false","six emulator key/value pairs preserve native initial state");
    require(s.game_attributes.at("DISPLAY_NAME")=="Game"&&s.game_attributes.at("UPTODATE")=="true"&&s.game_attributes.at("FULLGAME_IS_RELEASED")=="true"&&s.game_attributes.at("FULLGAME_PURCHASED")=="true"&&s.game_attributes.at("FREETRIAL")=="false","native game identity and release/trial text preserve case and exact keys");
    require(s.game_attributes.at("INSTALLED_VERSION")=="1.0.0.0"&&s.game_attributes.at("AVAILABLE_VERSION")=="1.0.0.0"&&s.game_attributes.at("INSTALLED_LANGUAGE")=="en_US"&&s.game_attributes.at("MAX_GROUP_SIZE")=="16","retained counted literal references resolve exact version/language/group defaults");
    require(s.game_attributes.at("FULLGAME_RELEASE_DATE")=="2010-01-01T00:00:00"&&s.game_attributes.at("EXPIRATION")=="0000-00-00T00:00:00"&&s.game_attributes.at("EXPIRATION_DURATION")=="36000000"&&s.game_attributes.at("ENTITLEMENT_SOURCE")=="ORIGIN","native expiration sentinel is preserved as text rather than normalized date");
    require(s.game_attributes.at("LANGUAGES").size()==0xb3&&s.game_attributes.at("LANGUAGES").starts_with("ar_SA,cs_CZ,")&&s.game_attributes.at("LANGUAGES").ends_with(",zh_CN,zh_HK,zh_TW"),"native languages retain counted179-byte string and original order");
}
void independent_values(){
    auto first=anadius::configuration_defaults::make_configuration_defaults_036d20();
    auto second=anadius::configuration_defaults::make_configuration_defaults_036d20();
    first.game_attributes.at("LANGUAGES")[0]='X';first.emulator_attributes.clear();first.username="own modified";first.content.push_back({"own","N","V","S"});
    require(second.username=="anadius"&&second.game_attributes.at("LANGUAGES").starts_with("ar_SA")&&second.emulator_attributes.size()==6&&second.content.empty(),"each factory result owns independent strings/maps/vectors without shared mutable default state");
}
void allocation_failure_unwind(){
    unsigned failures=0;bool completed=false;
    for(std::size_t fail_at=1;fail_at<=256&&!completed;++fail_at){
        allocation::remaining=fail_at;allocation::attempts=0;allocation::armed=true;
        try{auto value=anadius::configuration_defaults::make_configuration_defaults_036d20();allocation::armed=false;completed=value.game_attributes.size()==14&&value.emulator_attributes.size()==6;}
        catch(const std::bad_alloc&){allocation::armed=false;++failures;}
        require(allocation::live==0,"partial factory construction and normal result destruction release every tracked own-EXE allocation");
    }
    require(completed&&failures>20,"allocation exhaustion covered actual string/map construction failures and eventually completed");
    const auto after=anadius::configuration_defaults::make_configuration_defaults_036d20();
    require(after.username=="anadius"&&after.game_attributes.size()==14&&after.emulator_attributes.size()==6,"failed construction publishes no partial default state or shared poison into subsequent calls");
}
}
int main(){try{native_snapshot();independent_values();allocation_failure_unwind();std::cout<<"PASS: "<<checks<<" configuration default state core checks\n";return 0;}catch(const std::exception& e){allocation::armed=false;std::cerr<<e.what()<<'\n';return 1;}}
