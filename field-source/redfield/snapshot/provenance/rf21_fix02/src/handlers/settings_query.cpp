#include "preloader_l/handlers/settings_query.hpp"
#include "preloader_l/handlers/setting_value_route.hpp"
#include "preloader_l/handlers/settings_snapshot_route.hpp"
#include "preloader_l/handlers/connection_state_route.hpp"

namespace preloader_l { namespace handlers {
HandlerStatus handle_settings_query(
    const preloader_l::xml::RequestView& request,
    const preloader_l::config::SettingsProvider& provider,
    bool has_get_setting_key,
    preloader_l::config::SettingKey get_setting_key,
    char* output,
    size_t capacity,
    size_t* written) noexcept
{
    if (preloader_l::xml::span_equals(request.command,"GetSetting")) {
        if (!has_get_setting_key) { if (written) *written=0; return HandlerStatus::unsupported; }
        return handle_get_setting(request,get_setting_key,provider,output,capacity,written);
    }
    if (preloader_l::xml::span_equals(request.command,"GetSettings")) {
        return handle_get_settings(request,provider,output,capacity,written);
    }
    if (preloader_l::xml::span_equals(request.command,"GetInternetConnectedState")) {
        return handle_get_internet_connected_state(request,provider,output,capacity,written);
    }
    if (written) *written=0;
    return HandlerStatus::wrong_command;
}
} }
