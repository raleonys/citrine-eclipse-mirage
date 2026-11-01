#include "preloader_l/handlers/runtime_settings_routes.hpp"
#include "preloader_l/handlers/setting_value_route.hpp"
#include "preloader_l/handlers/settings_snapshot_route.hpp"
#include "preloader_l/handlers/selector_pipeline.hpp"
#include "preloader_l/xml/attribute_parser.hpp"
#include "preloader_l/xml/value_codec.hpp"

namespace preloader_l { namespace handlers {

HandlerStatus handle_runtime_get_setting(
    const preloader_l::xml::RequestView& request,
    char* output,
    size_t capacity,
    size_t* written) noexcept
{
    if (written) *written = 0;
    if (!preloader_l::xml::span_equals(request.command, "GetSetting"))
        return HandlerStatus::wrong_command;

    preloader_l::xml::TextSpan setting_id{};
    const auto selector_status = preloader_l::xml::find_attribute(
        request.command_open_tag, "SettingId", &setting_id);
    if (selector_status == preloader_l::xml::AttributeStatus::malformed ||
        selector_status == preloader_l::xml::AttributeStatus::invalid_argument)
        return HandlerStatus::invalid_request;
    if (selector_status != preloader_l::xml::AttributeStatus::found)
        return HandlerStatus::unsupported;

    // Phase x002 corrects the old representation label: behavioral GetSetting
    // consumes the required SettingId XML attribute directly. F1425-002: the
    // parser exposes source spelling, so cross the semantic boundary exactly
    // once before selector lookup.
    preloader_l::xml::SemanticValue semantic_selector;
    const auto semantic_status = semantic_selector.assign_parsed(setting_id);
    if (semantic_status == preloader_l::xml::ValueCodecStatus::malformed_entity ||
        semantic_status == preloader_l::xml::ValueCodecStatus::invalid_character ||
        semantic_status == preloader_l::xml::ValueCodecStatus::invalid_argument)
        return HandlerStatus::invalid_request;
    if (semantic_status != preloader_l::xml::ValueCodecStatus::ok)
        return HandlerStatus::response_error;
    const auto resolution = resolve_get_setting_selector(semantic_selector.view());
    if (!resolution.recognized || !resolution.value.present)
        return HandlerStatus::unsupported;
    return format_get_setting_response(request, resolution.value, output, capacity, written);
}

HandlerStatus handle_runtime_get_settings(
    const preloader_l::xml::RequestView& request,
    char* output,
    size_t capacity,
    size_t* written) noexcept
{
    if (!preloader_l::xml::span_equals(request.command, "GetSettings")) {
        if (written) *written = 0;
        return HandlerStatus::wrong_command;
    }

    const auto policy = resolve_runtime_settings_policy();
    preloader_l::config::SettingsSnapshot s = preloader_l::config::empty_settings();
    s.launch_offline_mode = policy.manual_offline;
    s.environment = policy.environment;
    s.telemetry_enabled = policy.telemetry_enabled;
    s.igo_available = policy.igo_available;
    s.igo_enabled = policy.igo_enabled;
    s.language = policy.language;
    return format_get_settings_response(request, s, output, capacity, written);
}

} }
