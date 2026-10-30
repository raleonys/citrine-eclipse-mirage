#include "preloader_l/handlers/handlers.hpp"
#include <array>
#include <utility>

namespace preloader_l::handlers_recovered {
bool get_config_02e5f0(Xml& xml) {
    static constexpr std::pair<const char*,const char*> services[] = {
        {"EbisuSDK","SDK"},{"EbisuSDK","PROFILE"},{"XMPP","PRESENCE"},
        {"XMPP","FRIENDS"},{"Commerce","COMMERCE"},{"EbisuSDK","RECENTPLAYER"},
        {"EbisuSDK","IGO"},{"EbisuSDK","MISC"},{"EALS","LOGIN"},
        {"Utility","UTILITY"},{"XMPP","XMPP"},{"XMPP","CHAT"},
        {"EbisuSDK","IGO_EVENT"},{"EALS","EALS_EVENTS"},{"EbisuSDK","LOGIN_EVENT"},
        {"XMPP","INVITE_EVENT"},{"EbisuSDK","PROFILE_EVENT"},{"XMPP","PRESENCE_EVENT"},
        {"XMPP","FRIENDS_EVENT"},{"Commerce","COMMERCE_EVENT"},{"XMPP","CHAT_EVENT"},
        {"EbisuSDK","DOWNLOAD_EVENT"},{"EbisuSDK","PERMISSION"},{"EbisuSDK","RESOURCES"},
        {"EbisuSDK","BLOCKED_USERS"},{"EbisuSDK","BLOCKED_USER_EVENT"},
        {"EbisuSDK","GET_USERID"},{"EbisuSDK","ONLINE_STATUS_EVENT"},
        {"EbisuSDK","ACHIEVEMENT"},{"EbisuSDK","ACHIEVEMENT_EVENT"},
        {"EbisuSDK","BROADCAST_EVENT"},{"PI","PROGRESSIVE_INSTALLATION"},
        {"PI","PROGRESSIVE_INSTALLATION_EVENT"},{"EbisuSDK","CONTENT"}
    };
    static_assert(std::size(services)==34);
    xml.open_004080("GetConfigResponse",false);
    for(const auto& [name,facility]:services) {
        xml.open_004080("Service",false);
        xml.attribute_005560("Name",name);
        xml.attribute_005560("Facility",facility);
        xml.close_virtual_48(false);
    }
    xml.close_virtual_48(false);
    return true; // Original only assigns the low byte of its return register.
}

bool get_profile_02f530(Request request,Xml& xml,StateReferences state,Effects& effects) {
    const auto index=xml.attribute_002cc0(request,"index");
    if(!index) return false;
    const auto& profile=state.profile;
    effects.log_profile_name_084f48(profile.username);
    xml.open_004080("GetProfileResponse",false);
    xml.attribute_005560("Country",profile.country.c_str());
    xml.attribute_005560("GeoCountry",profile.country.c_str());
    xml.attribute_005560("PersonaId",profile.persona_id.c_str());
    xml.attribute_005560("IsTrialSubscriber","false");
    xml.attribute_005560("SubscriberLevel",profile.subscriber_level.c_str());
    xml.attribute_005560("Persona",profile.username.c_str());
    xml.attribute_005560("UserId",profile.user_id.c_str());
    xml.attribute_005560("CommerceCountry",profile.country.c_str());
    xml.attribute_005560("CommerceCurrency",profile.currency.c_str());
    xml.attribute_005560("AvatarId","");
    xml.attribute_005560("IsSubscriber",profile.is_subscriber.c_str());
    xml.attribute_005560("UserIndex",index);
    xml.attribute_005560("IsUnderAge",profile.is_under_age.c_str());
    xml.attribute_005560("IsSteamSubscriber",profile.is_steam_subscriber.c_str());
    xml.close_virtual_48(false);
    return true;
}

bool get_connected_02f2d0(Xml& xml,StateReferences state) {
    xml.open_004080("InternetConnectedState",false);
    // Literal numeric strings; not true/false XML boolean spellings.
    xml.attribute_005560("connected",state.online_0a3c3b || state.pretend_connected_0a3c3a ? "1":"0");
    xml.close_virtual_48(false);
    return true;
}

static bool lookup_response(Request request,Xml& xml,Attributes& values,
    const char* request_attribute,const char* response,const char* output_attribute) {
    const char* key=xml.attribute_002cc0(request,request_attribute);
    if(!key || values.find(key)==values.end()) return false;
    xml.open_004080(response,false);
    // Keep the observed second, inserting lookup after opening the response.
    xml.attribute_005560(output_attribute,values[key].c_str());
    xml.close_virtual_48(false);
    return true;
}
bool get_setting_02f7c0(Request request,Xml& xml,StateReferences state) {
    return lookup_response(request,xml,state.settings_0a3d78,"SettingId","GetSettingResponse","Setting");
}
bool get_game_info_02efd0(Request request,Xml& xml,StateReferences state) {
    return lookup_response(request,xml,state.game_0a3d88,"GameInfoId","GetGameInfoResponse","GameInfo");
}

bool get_settings_02fa60(Xml& xml,StateReferences state) {
    static constexpr std::pair<const char*,const char*> entries[] = {
        {"Environment","ENVIRONMENT"},{"IsIGOAvailable","IS_IGO_AVAILABLE"},
        {"IsTelemetryEnabled","IS_TELEMETRY_ENABLED"},{"IsManualOffline","IS_MANUAL_OFFLINE"},
        {"Language","LANGUAGE"},{"IsIGOEnabled","IS_IGO_ENABLED"}
    };
    xml.open_004080("GetSettingsResponse",false);
    for(const auto& [attribute,key]:entries)
        xml.attribute_005560(attribute,state.settings_0a3d78[key].c_str());
    xml.close_virtual_48(false);
    return true;
}

bool get_all_game_info_02da90(Xml& xml,StateReferences state,Effects& effects) {
    static constexpr std::pair<const char*,const char*> entries[] = {
        {"DisplayName","DISPLAY_NAME"},{"InstalledVersion","INSTALLED_VERSION"},
        {"AvailableVersion","AVAILABLE_VERSION"},{"UpToDate","UPTODATE"},
        {"FullGameReleased","FULLGAME_IS_RELEASED"},{"FullGameReleaseDate","FULLGAME_RELEASE_DATE"},
        {"FullGamePurchased","FULLGAME_PURCHASED"},{"FreeTrial","FREETRIAL"},
        {"Expiration","EXPIRATION"},{"EntitlementSource","ENTITLEMENT_SOURCE"},
        {"MaxGroupSize","MAX_GROUP_SIZE"},{"Languages","LANGUAGES"},
        {"InstalledLanguage","INSTALLED_LANGUAGE"}
    };
    xml.open_004080("GetAllGameInfoResponse",false);
    for(const auto& [attribute,key]:entries)
        xml.attribute_005560(attribute,state.game_0a3d88[key].c_str());
    xml.attribute_005560("HasExpiration","false");
    const auto time=effects.current_time_041b60();
    xml.attribute_005560("SystemTime",time.c_str());
    xml.close_virtual_48(false);
    return true;
}
} // namespace preloader_l::handlers_recovered
