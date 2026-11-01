// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/settings_response.hpp"
#include "redfield/config_snapshot_view.hpp"
#include "permission_formatter.hpp"
#include "redfield/handlers/setting_value_route.hpp"
#include "redfield/handlers/settings_snapshot_route.hpp"
#include "redfield/handlers/connection_state_route.hpp"
#include "redfield/config/settings_provider.hpp"
namespace redfield {
namespace {
namespace config=redfield::config;
namespace px=redfield::xml;
struct SettingsView {
    const ConfigSnapshot& snapshot;
    config::ValueView leaf(const char* section,const char* key,const char* fallback,bool* used_fallback=nullptr) const noexcept {
        return ConfigSnapshotView{snapshot}.or_default(section,key,fallback,used_fallback);
    }
    config::ValueView value(config::SettingKey key) const noexcept {
        using config::SettingKey;
        switch(key) {
        // Explicit normalized booleans prevent the adopted provider's absent
        // Online compatibility default from changing REDFIELD's offline default.
        case SettingKey::online:return config::literal_value(snapshot.online?"true":"false");
        case SettingKey::launch_offline_mode:return config::literal_value(snapshot.online?"false":"true");
        case SettingKey::language:return leaf("Game","Language","en_US");
        case SettingKey::environment:return leaf("Emulator","Environment","production");
        case SettingKey::telemetry_enabled:return leaf("Emulator","IsTelemetryEnabled","false");
        case SettingKey::igo_available:return leaf("Emulator","IGOAvailable",snapshot.online?"true":"false");
        case SettingKey::igo_enabled:return leaf("Emulator","IGOEnabled",snapshot.online?"true":"false");
        }
        return config::missing_value();
    }
    static bool lookup(const void* context,config::SettingKey key,config::ValueView* out) noexcept {
        if(!context || !out)return false;
        *out=static_cast<const SettingsView*>(context)->value(key);return out->present;
    }
    config::SettingsProvider provider() const noexcept{return {this,lookup};}
};
config::ValueView selector_value(const SettingsView& view,px::TextSpan selector) noexcept {
    using config::SettingKey;
    if(px::span_equals(selector,"LANGUAGE"))return view.value(SettingKey::language);
    if(px::span_equals(selector,"ENVIRONMENT"))return view.value(SettingKey::environment);
    if(px::span_equals(selector,"IS_TELEMETRY_ENABLED"))return view.value(SettingKey::telemetry_enabled);
    if(px::span_equals(selector,"IS_MANUAL_OFFLINE"))return view.value(SettingKey::launch_offline_mode);
    config::SettingKey key;
    if(px::span_equals(selector,"IS_IGO_AVAILABLE"))key=SettingKey::igo_available;
    else if(px::span_equals(selector,"IS_IGO_ENABLED"))key=SettingKey::igo_enabled;
    else return config::missing_value();
    const auto value=view.value(key);
    if(config::explicit_true(value))return config::literal_value("true");
    if(config::explicit_false(value))return config::literal_value("DOWN");
    return config::missing_value();
}
}
config::ValueView settings_language_value(const ConfigSnapshot& snapshot,bool* used_fallback) noexcept {
    return SettingsView{snapshot}.leaf("Game","Language","en_US",used_fallback);
}
ResponseResult dispatch_settings_response(const xml::Request& request,const ConfigSnapshot& snapshot,char* output,std::size_t capacity,std::size_t* written) noexcept {
    if(!written)return ResponseResult::rejected;
    *written=0;if(!output || !capacity)return ResponseResult::rejected;
    const SettingsView view{snapshot};
    using namespace redfield::handlers;
    HandlerStatus status;
    if(redfield::xml::span_equals(request.command,"GetSettings"))
        status=handle_get_settings(request,view.provider(),output,capacity,written);
    else if(redfield::xml::span_equals(request.command,"GetInternetConnectedState"))
        status=handle_get_internet_connected_state(request,view.provider(),output,capacity,written);
    else if(redfield::xml::span_equals(request.command,"CheckPermission"))
        status=redfield::handlers::permission_detail::format_check_permission(request,config::explicit_true(view.value(config::SettingKey::online)),output,capacity,written);
    else if(redfield::xml::span_equals(request.command,"GetSetting")) {
        redfield::xml::TextSpan selector;
        const auto found=redfield::xml::find_attribute(request.command_open_tag,"SettingId",&selector);
        if(found==redfield::xml::AttributeStatus::not_found)return ResponseResult::no_response;
        if(found!=redfield::xml::AttributeStatus::found)return ResponseResult::rejected;
        const auto setting=selector_value(view,selector);
        if(!setting.present)return ResponseResult::no_response;
        status=format_get_setting_response(request,setting,output,capacity,written);
    }else return ResponseResult::no_response;
    if(status==HandlerStatus::unsupported || status==HandlerStatus::no_response_required){*written=0;return ResponseResult::no_response;}
    if(status!=HandlerStatus::ok || *written>=capacity){*written=0;return ResponseResult::rejected;}
    return ResponseResult::response;
}
}
