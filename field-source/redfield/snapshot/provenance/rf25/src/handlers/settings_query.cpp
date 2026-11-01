// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/handlers/settings_query.hpp"
#include "redfield/handlers/setting_value_route.hpp"
#include "redfield/handlers/settings_snapshot_route.hpp"
#include "redfield/handlers/connection_state_route.hpp"

namespace redfield { namespace handlers {
HandlerStatus handle_settings_query(
    const redfield::xml::RequestView& request,
    const redfield::config::SettingsProvider& provider,
    bool has_get_setting_key,
    redfield::config::SettingKey get_setting_key,
    char* output,
    size_t capacity,
    size_t* written) noexcept
{
    if (redfield::xml::span_equals(request.command,"GetSetting")) {
        if (!has_get_setting_key) { if (written) *written=0; return HandlerStatus::unsupported; }
        return handle_get_setting(request,get_setting_key,provider,output,capacity,written);
    }
    if (redfield::xml::span_equals(request.command,"GetSettings")) {
        return handle_get_settings(request,provider,output,capacity,written);
    }
    if (redfield::xml::span_equals(request.command,"GetInternetConnectedState")) {
        return handle_get_internet_connected_state(request,provider,output,capacity,written);
    }
    if (written) *written=0;
    return HandlerStatus::wrong_command;
}
} }
