#include "teardown.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <new>
#include <stdexcept>

// Own EXE allocations only. The observer never allocates or calls a provider.
// Track actual payload/backing pointers, not native container nodes/sentinels.
namespace deletion {
struct Entry {std::uintptr_t address;const char* label;unsigned frees;std::size_t order;};
std::array<Entry,128> entries{};
std::size_t count=0,sequence=0;bool active=false;
void watch(const void* pointer,const char* label){
    const auto address=reinterpret_cast<std::uintptr_t>(pointer);
    if(!address||count==entries.size())std::abort();
    for(std::size_t i=0;i<count;++i)if(entries[i].address==address)std::abort();
    entries[count++]={address,label,0,0};
}
template<class T>void watch_data(const T& value,const char* label){watch(value.data(),label);}
void release(void* pointer)noexcept{
    if(active&&pointer){const auto address=reinterpret_cast<std::uintptr_t>(pointer);
        for(std::size_t i=0;i<count;++i)if(entries[i].address==address){auto& entry=entries[i];++entry.frees;if(entry.frees==1)entry.order=++sequence;break;}}
    std::free(pointer);
}
void* acquire(std::size_t size){if(auto* pointer=std::malloc(size?size:1))return pointer;throw std::bad_alloc();}
}
void* operator new(std::size_t size){return deletion::acquire(size);}
void* operator new[](std::size_t size){return deletion::acquire(size);}
void operator delete(void* pointer)noexcept{deletion::release(pointer);}
void operator delete[](void* pointer)noexcept{deletion::release(pointer);}
void operator delete(void* pointer,std::size_t)noexcept{deletion::release(pointer);}
void operator delete[](void* pointer,std::size_t)noexcept{deletion::release(pointer);}

namespace {
using namespace anadius;namespace cd=configuration_defaults;namespace c=configuration;
unsigned checks=0;
void require(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
std::string text(char value){return std::string(257,value);}
void ordered_release(){
    bool empty=false,scalars=false,freed_during_release=false,order=false;
    std::array<unsigned,128> before_compiler_destruction{};
    {
        c::State state{};achievement_storage::Paths paths;
        std::vector<recovered::libraries::Module> modules{0x123400,0x567800};
        std::optional<license_recovered::AuthenticationCode> authentication(std::in_place);
        c::Path* path_fields[]{&state.executable_path,&state.language_registry_key,&state.license_directory,&state.denuvo_dll,&state.documents,&state.avatar,&state.user_directory,&state.cookie_path,&state.last_profile_path,&state.last_avatar_path,&state.default_avatar,&paths.xml_path,&paths.ini_path};
        for(auto* value:path_fields)value->assign(257,u'P');
        std::string* string_fields[]{&state.content_id,&state.denuvo_token,&state.exe_hash,&state.dll_hash,&state.sdk,&state.username,&state.persona_id,&state.user_id,&state.country,&state.currency,&state.is_subscriber,&state.is_steam_subscriber,&state.subscriber_level,&state.is_under_age,&state.achievements_set,&state.steam_app_id};
        for(auto* value:string_fields)value->assign(257,'S');
        state.language_from_arguments=0x31;state.registry_spoof=0x32;state.server_port=0x1234;
        state.pretend_connected=0x33;state.fake_auth=0x34;state.block_reruns=0x35;state.use_last_online_profile=0x36;
        state.extra_dlls={c::Path(257,u'a'),c::Path(257,u'b')};state.extra_dlls_from_main={c::Path(257,u'c'),c::Path(257,u'd')};
        state.content.reserve(2);state.entitlements.reserve(2);
        for(unsigned i=0;i<2;++i){const auto character=static_cast<char>('a'+i);
            state.content.push_back({text(character),text(character),text(character),text(character)});
            state.entitlements.push_back({text(character),text(character),text(character),text(character),text(character)});
        }
        state.achievements.emplace("one",c::Achievement{5,1,4,text('t'),text('n'),text('s'),99});
        state.game_attributes.emplace("game",text('g'));state.emulator_attributes.emplace("emulator",text('e'));
        using deletion::watch_data;
        // Native field-group order. Map payload probes establish group placement
        // only; no ordering among native map nodes or host pair members is inferred.
        watch_data(state.documents,"documents+518");watch_data(modules,"loaded module backing+4f8");
        watch_data(state.extra_dlls[0],"extra DLL0+4e0");watch_data(state.extra_dlls[1],"extra DLL1+4e0");watch_data(state.extra_dlls,"extra DLL backing+4e0");
        watch_data(state.extra_dlls_from_main[0],"main DLL0+4c8");watch_data(state.extra_dlls_from_main[1],"main DLL1+4c8");watch_data(state.extra_dlls_from_main,"main DLL backing+4c8");
        watch_data(paths.ini_path,"INI+4a8");watch_data(state.steam_app_id,"Steam app+488");watch_data(state.achievements.at("one").timestamp,"achievement payload group+478");
        watch_data(paths.xml_path,"XML+408");watch_data(state.achievements_set,"achievement set+3e8");
        // Retained normal vector traversal is forward; each element's strings
        // are explicitly retired in tag..id / state..id order before backing.
        for(const auto& value:state.entitlements){watch_data(value.tag,"entitlement tag");watch_data(value.type,"entitlement type");watch_data(value.version,"entitlement version");watch_data(value.group,"entitlement group");watch_data(value.id,"entitlement id");}
        watch_data(state.entitlements,"entitlement backing+3d0");
        for(const auto& value:state.content){watch_data(value.state,"content state");watch_data(value.version,"content version");watch_data(value.name,"content name");watch_data(value.id,"content id");}
        watch_data(state.content,"content backing+3b8");
        watch_data(state.game_attributes.at("game"),"game attributes+3a8");watch_data(state.emulator_attributes.at("emulator"),"emulator attributes+398");
        watch_data(state.is_under_age,"under age+378");watch_data(state.subscriber_level,"subscriber level+358");watch_data(state.is_steam_subscriber,"Steam subscriber+338");watch_data(state.is_subscriber,"subscriber+318");
        watch_data(state.currency,"currency+2f8");watch_data(state.country,"country+2d8");watch_data(state.sdk,"SDK+2b8");
        watch_data(state.user_id,"user ID+238");watch_data(state.persona_id,"persona+218");watch_data(state.username,"username+1f8");watch_data(state.denuvo_dll,"Denuvo path+1d8");
        watch_data(state.dll_hash,"DLL hash+1b8");watch_data(state.exe_hash,"EXE hash+198");watch_data(state.denuvo_token,"token+178");watch_data(state.license_directory,"license path+158");
        watch_data(authentication->value(),"authentication+138");watch_data(state.content_id,"content ID+118");watch_data(state.language_registry_key,"registry key+f8");
        watch_data(state.default_avatar,"default avatar+d0");watch_data(state.avatar,"avatar+b0");watch_data(state.last_avatar_path,"last avatar+80");watch_data(state.last_profile_path,"last profile+60");watch_data(state.cookie_path,"cookie+40");watch_data(state.user_directory,"user directory+20");watch_data(state.executable_path,"executable+0");
        deletion::active=true;
        cd::release_aggregate_storage_037f70({state,paths,modules,authentication});
        empty=!authentication&&modules.empty()&&!modules.capacity()&&state.extra_dlls.empty()&&!state.extra_dlls.capacity()&&state.extra_dlls_from_main.empty()&&!state.extra_dlls_from_main.capacity()&&state.content.empty()&&!state.content.capacity()&&state.entitlements.empty()&&!state.entitlements.capacity()&&state.achievements.empty()&&state.game_attributes.empty()&&state.emulator_attributes.empty();
        for(auto* value:path_fields)empty=empty&&value->empty();
        for(auto* value:string_fields)empty=empty&&value->empty();
        scalars=state.language_from_arguments==0x31&&state.registry_spoof==0x32&&state.server_port==0x1234&&state.pretend_connected==0x33&&state.fake_auth==0x34&&state.block_reruns==0x35&&state.use_last_online_profile==0x36;
        freed_during_release=true;order=true;
        for(std::size_t i=0;i<deletion::count;++i){const auto& entry=deletion::entries[i];freed_during_release=freed_during_release&&entry.frees==1;order=order&&entry.order==i+1;before_compiler_destruction[i]=entry.frees;}
        // All host objects now undergo ordinary compiler destruction once.
    }
    deletion::active=false;
    bool no_second_free=true;
    for(std::size_t i=0;i<deletion::count;++i)no_second_free=no_second_free&&deletion::entries[i].frees==before_compiler_destruction[i];
    require(empty,"Every modeled host string/path/map/vector is empty and vector backing storage retired");
    require(scalars,"Aggregate storage release preserves all modeled scalar flags and server port");
    require(freed_during_release,"Every watched actual payload/backing allocation is freed exactly once by selected teardown");
    require(order,"Actual frees follow field groups, forward DLL paths and content/entitlement element/member order, including authentication before content ID");
    require(no_second_free,"Later ordinary compiler destruction never frees a watched allocation again");
}
void empty_storage(){
    c::State state{};achievement_storage::Paths paths;std::vector<recovered::libraries::Module> modules;std::optional<license_recovered::AuthenticationCode> authentication;
    cd::release_aggregate_storage_037f70({state,paths,modules,authentication});
    require(!authentication&&modules.empty()&&state.documents.empty()&&paths.xml_path.empty()&&paths.ini_path.empty()&&state.server_port==0,"A separate initially empty aggregate needs no authentication object or provider");
}
}
int main(){try{ordered_release();empty_storage();std::cout<<"PASS: "<<checks<<" aggregate teardown checks; "<<deletion::count<<" actual allocation frees observed, host ordering only\n";}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}}
