#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <string_view>

namespace preloader_l::handlers {
using Request = std::uintptr_t;
using Attributes = std::map<std::string,std::string,std::less<>>;
class Xml {
public:
    virtual ~Xml() = default;
    // Nullable C-string is important: absent and present-but-empty differ.
    virtual const char* attribute_002cc0(Request,const char* key) = 0;
    virtual void open_004080(const char* element,bool compact) = 0;
    virtual void attribute_005560(const char* key,const char* value) = 0;
    virtual void close_virtual_48(bool compact) = 0;
};
class Effects {
public:
    virtual ~Effects() = default;
    virtual void log_profile_name_084f48(std::string_view username) = 0;
    virtual std::string current_time_041b60() = 0;
};
struct Profile {
    std::string username,persona_id,user_id,country,currency;
    std::string subscriber_level,is_subscriber,is_under_age,is_steam_subscriber;
};
struct StateReferences {
    Attributes& game_0a3d88;
    Attributes& settings_0a3d78;
    const Profile& profile;
    const std::uint8_t& online_0a3c3b;
    const std::uint8_t& pretend_connected_0a3c3a;
};
bool get_config_02e5f0(Xml&);
bool get_profile_02f530(Request,Xml&,StateReferences,Effects&);
bool get_connected_02f2d0(Xml&,StateReferences);
bool get_setting_02f7c0(Request,Xml&,StateReferences);
bool get_settings_02fa60(Xml&,StateReferences);
bool get_game_info_02efd0(Request,Xml&,StateReferences);
bool get_all_game_info_02da90(Xml&,StateReferences,Effects&);
} // namespace preloader_l::handlers_recovered
