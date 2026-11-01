#include "runtime_configuration_internal.hpp"
#include "config2_pegtl.hpp"
namespace preloader_l { namespace config { namespace runtime_configuration_detail {
namespace {
using Token=config2_pegtl::Span;
struct Cursor { const char* data; };
enum class SectionPolicy : uint8_t { none=0, game, emulator, user, achievements };

bool token_equals(const Cursor& c,Token t,const char* literal) noexcept {
    if(!literal) return false;
    const size_t n=cstr_len(literal);
    return bytes_equal(c.data+t.off,t.len,literal,n);
}

SectionPolicy section_policy(const Cursor& c,Token section) noexcept {
    if(token_equals(c,section,"Game")) return SectionPolicy::game;
    if(token_equals(c,section,"Emulator")) return SectionPolicy::emulator;
    if(token_equals(c,section,"User")) return SectionPolicy::user;
    if(token_equals(c,section,"Achievements")) return SectionPolicy::achievements;
    return SectionPolicy::none;
}

uint32_t section_bit(const Cursor& c,Token section) noexcept {
    switch(section_policy(c,section)){
    case SectionPolicy::game: return 1u;
    case SectionPolicy::emulator: return 2u;
    case SectionPolicy::user: return 4u;
    case SectionPolicy::achievements: return 8u;
    default: return 0u;
    }
}

SectionPolicy reserved_key_policy(const Cursor& c,Token key) noexcept {
    if(token_equals(c,key,"Name")||token_equals(c,key,"Version")||
       token_equals(c,key,"GameName")||token_equals(c,key,"GameVersion")||
       token_equals(c,key,"ContentId")||token_equals(c,key,"Languages")||
       token_equals(c,key,"Language")||token_equals(c,key,"LanguageRegistryKey")||
       token_equals(c,key,"LanguageRegistrySpoof")||token_equals(c,key,"ClientKey")||
       token_equals(c,key,"ClientKey2")||token_equals(c,key,"ClientValue")||
       token_equals(c,key,"ClientPath")||token_equals(c,key,"BlockReruns")||
       token_equals(c,key,"Documents")) return SectionPolicy::game;
    if(token_equals(c,key,"Online")||token_equals(c,key,"Environment")||
       token_equals(c,key,"SDK")||token_equals(c,key,"EALaunchOfflineMode")||
       token_equals(c,key,"IGOAvailable")||token_equals(c,key,"IGOEnabled")||
       token_equals(c,key,"IsTelemetryEnabled")) return SectionPolicy::emulator;
    if(token_equals(c,key,"Username")||token_equals(c,key,"PersonaId")||
       token_equals(c,key,"UserId")||token_equals(c,key,"Country")||
       token_equals(c,key,"Currency")||token_equals(c,key,"IsSubscriber")||
       token_equals(c,key,"IsSteamSubscriber")||token_equals(c,key,"SubscriberLevel")||
       token_equals(c,key,"IsUnderAge")||token_equals(c,key,"Avatar")||
       token_equals(c,key,"AvatarImage")||token_equals(c,key,"AuthCode"))
        return SectionPolicy::user;
    if(token_equals(c,key,"AchievementsSet")||token_equals(c,key,"SteamAppId")||
       token_equals(c,key,"SteamIdPrefix")) return SectionPolicy::achievements;
    return SectionPolicy::none;
}

bool put_normalized_leaf_setting(State& st,const Cursor& c,Token section,Token key,Token value) noexcept {
    const SectionPolicy actual=section_policy(c,section);
    // Config2 leaves are authoritative only inside one of the curio top-level
    // sections. Root-level leaves and unknown sections remain parse-compatible
    // but cannot publish into the flat runtime policy namespace.
    if(actual==SectionPolicy::none) return true;
    const SectionPolicy reserved=reserved_key_policy(c,key);
    if(reserved!=SectionPolicy::none&&reserved!=actual) return true;

    const char* alias=nullptr;
    if(actual==SectionPolicy::game&&token_equals(c,key,"Name")) alias="GameName";
    else if(actual==SectionPolicy::game&&token_equals(c,key,"Version")) alias="GameVersion";
    if(alias) return put_setting(st,alias,cstr_len(alias),c.data+value.off,value.len);
    return put_setting(st,c.data+key.off,key.len,c.data+value.off,value.len);
}


struct Context { State* staged; Cursor cursor; };
bool on_section(void* opaque,Token section) noexcept {
    auto& x=*static_cast<Context*>(opaque);
    x.staged->present|=section_bit(x.cursor,section);
    return true;
}
bool on_leaf(void* opaque,const Token* path,size_t depth,Token key,Token value) noexcept {
    auto& x=*static_cast<Context*>(opaque);
    if(depth==1u)return put_normalized_leaf_setting(*x.staged,x.cursor,path[0],key,value);
    return true;
}
} // namespace
ParseCode parse_config2(State& st,const char* data,size_t length) noexcept {
    Context context{&st,Cursor{data}};
    const config2_pegtl::Visitor visitor{&context,on_section,on_leaf};
    const auto parsed=config2_pegtl::parse(data,length,visitor);
    if(parsed!=config2_pegtl::Result::ok)
        return parsed==config2_pegtl::Result::resource?ParseCode::resource:ParseCode::invalid;
    st.loaded=true;
    return ParseCode::ok;
}
RuntimeConfigurationStatus map_code(ParseCode c) noexcept {
    return c==ParseCode::resource ? RuntimeConfigurationStatus::resource_error
         : c==ParseCode::invalid ? RuntimeConfigurationStatus::invalid_cfg
                                 : RuntimeConfigurationStatus::ok;
}

} } }
