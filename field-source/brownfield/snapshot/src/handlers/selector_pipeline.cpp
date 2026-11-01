#include "preloader_l/handlers/selector_pipeline.hpp"
#include "preloader_l/config/settings_provider.hpp"
#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/config/runtime_provider.hpp"

namespace preloader_l { namespace handlers {
namespace {
constexpr char kLanguagesFallback[] =
    "ar_SA,cs_CZ,da_DK,de_DE,el_GR,en_AU,en_CA,en_GB,en_US,es_ES,es_MX,fi_FI,fr_CA,fr_FR,hu_HU,it_IT,ja_JP,ko_KR,nl_NL,no_NO,pl_PL,pt_BR,pt_PT,ru_RU,sv_SE,th_TH,tr_TR,zh_CN,zh_HK,zh_TW";

bool eq(preloader_l::xml::TextSpan s, const char* literal) noexcept {
    return preloader_l::xml::span_equals(s, literal);
}

preloader_l::config::ValueView setting_or_default(preloader_l::config::SettingKey key,
                                              const char* fallback,
                                              bool* used_fallback) noexcept {
    preloader_l::config::ValueView value = preloader_l::config::missing_value();
    const auto& provider = preloader_l::config::runtime_settings_provider();
    if (preloader_l::config::lookup_setting(provider, key, &value)) {
        if (used_fallback) *used_fallback = false;
        return value;
    }
    if (used_fallback) *used_fallback = true;
    return preloader_l::config::literal_value(fallback);
}

preloader_l::config::ValueView configuration_or_default(const char* key,
                                                    const char* fallback,
                                                    bool* used_fallback) noexcept {
    const auto value = preloader_l::config::runtime_configuration_setting(key);
    if (value.present) {
        if (used_fallback) *used_fallback = false;
        return value;
    }
    if (used_fallback) *used_fallback = true;
    return preloader_l::config::literal_value(fallback);
}

preloader_l::config::ValueView installed_language_policy(bool* used_fallback) noexcept {
    // F1418-003: INSTALLED_LANGUAGE reads the same shared policy as
    // GetSetting/GetSettings and environment publication. Bootstrap publishes
    // the process -emuGameLang override into SettingKey::language first.
    return setting_or_default(
        preloader_l::config::SettingKey::language, "en_US", used_fallback);
}

struct BooleanPolicy {
    bool value;
    bool used_fallback;
    bool valid;
};

BooleanPolicy boolean_policy(
    preloader_l::config::SettingKey explicit_key,
    bool invert_online) noexcept
{
    preloader_l::config::ValueView explicit_value = preloader_l::config::missing_value();
    const auto& provider = preloader_l::config::runtime_settings_provider();
    if (preloader_l::config::lookup_setting(provider, explicit_key, &explicit_value)) {
        if (preloader_l::config::explicit_true(explicit_value))
            return {true, false, true};
        if (preloader_l::config::explicit_false(explicit_value))
            return {false, false, true};
        return {false, false, false};
    }

    const auto online = setting_or_default(
        preloader_l::config::SettingKey::online, "true", nullptr);
    const bool enabled = preloader_l::config::online_enabled(online);
    return {invert_online ? !enabled : enabled, true, true};
}

BooleanPolicy manual_offline_policy() noexcept {
    bool used_default = false;
    const auto online = setting_or_default(
        preloader_l::config::SettingKey::online, "true", &used_default);
    return {!preloader_l::config::online_enabled(online), used_default, true};
}

preloader_l::config::ValueView boolean_value(
    BooleanPolicy policy,
    const char* true_value,
    const char* false_value) noexcept
{
    if (!policy.valid) return preloader_l::config::missing_value();
    return preloader_l::config::literal_value(
        policy.value ? true_value : false_value);
}

SelectorResolution empty_unrecognized() noexcept {
    return {preloader_l::config::literal_value(""), false, false};
}
} // namespace

SelectorResolution resolve_get_setting_selector(preloader_l::xml::TextSpan selector) noexcept {
    if (eq(selector, "IS_IGO_ENABLED")) {
        const auto policy = boolean_policy(
            preloader_l::config::SettingKey::igo_enabled, false);
        return {boolean_value(policy, "true", "DOWN"), true, policy.used_fallback};
    }
    if (eq(selector, "IS_IGO_AVAILABLE")) {
        const auto policy = boolean_policy(
            preloader_l::config::SettingKey::igo_available, false);
        return {boolean_value(policy, "true", "DOWN"), true, policy.used_fallback};
    }
    if (eq(selector, "ENVIRONMENT")) {
        bool fallback = false;
        const auto value = setting_or_default(preloader_l::config::SettingKey::environment, "production", &fallback);
        return {value, true, fallback};
    }
    if (eq(selector, "LANGUAGE")) {
        bool fallback = false;
        const auto value = setting_or_default(preloader_l::config::SettingKey::language, "en_US", &fallback);
        return {value, true, fallback};
    }
    if (eq(selector, "IS_TELEMETRY_ENABLED")) {
        bool fallback=false;
        const auto value=setting_or_default(preloader_l::config::SettingKey::telemetry_enabled,"false",&fallback);
        return {value,true,fallback};
    }
    if (eq(selector, "IS_MANUAL_OFFLINE")) {
        const auto policy = manual_offline_policy();
        return {boolean_value(policy, "true", "false"), true, policy.used_fallback};
    }
    return empty_unrecognized();
}

RuntimeSettingsPolicy resolve_runtime_settings_policy() noexcept {
    const auto manual = manual_offline_policy();
    const auto available = boolean_policy(
        preloader_l::config::SettingKey::igo_available, false);
    const auto enabled = boolean_policy(
        preloader_l::config::SettingKey::igo_enabled, false);

    return RuntimeSettingsPolicy{
        boolean_value(manual, "true", "false"),
        setting_or_default(
            preloader_l::config::SettingKey::environment, "production", nullptr),
        setting_or_default(
            preloader_l::config::SettingKey::telemetry_enabled, "false", nullptr),
        boolean_value(available, "true", "false"),
        boolean_value(enabled, "true", "false"),
        setting_or_default(
            preloader_l::config::SettingKey::language, "en_US", nullptr)
    };
}

SelectorResolution resolve_get_game_info_selector(preloader_l::xml::TextSpan selector) noexcept {
    if (eq(selector, "DISPLAY_NAME")) {
        bool fallback = false;
        const auto value = configuration_or_default("GameName", "Game", &fallback);
        return {value, true, fallback};
    }
    if (eq(selector, "INSTALLED_VERSION") || eq(selector, "AVAILABLE_VERSION")) {
        bool fallback = false;
        // Phase x001 closes AVAILABLE_VERSION as a post-config copy of the
        // final INSTALLED_VERSION value, so both selectors share one policy.
        const auto value = configuration_or_default("GameVersion", "1.0.0.0", &fallback);
        return {value, true, fallback};
    }
    if (eq(selector, "UPTODATE"))
        return {preloader_l::config::literal_value("true"), true, false};
    if (eq(selector, "FULLGAME_IS_RELEASED"))
        return {preloader_l::config::literal_value("true"), true, false};
    if (eq(selector, "FULLGAME_RELEASE_DATE"))
        return {preloader_l::config::literal_value("2010-01-01T00:00:00"), true, false};
    if (eq(selector, "FULLGAME_PURCHASED"))
        return {preloader_l::config::literal_value("true"), true, false};
    if (eq(selector, "FREETRIAL"))
        return {preloader_l::config::literal_value("false"), true, false};
    if (eq(selector, "EXPIRATION"))
        return {preloader_l::config::literal_value("0000-00-00T00:00:00"), true, false};
    if (eq(selector, "ENTITLEMENT_SOURCE"))
        return {preloader_l::config::literal_value("ORIGIN"), true, false};
    if (eq(selector, "MAX_GROUP_SIZE"))
        return {preloader_l::config::literal_value("16"), true, false};
    if (eq(selector, "EXPIRATION_DURATION"))
        return {preloader_l::config::literal_value("36000000"), true, false};
    if (eq(selector, "LANGUAGES")) {
        bool fallback = false;
        const auto value = configuration_or_default("Languages", kLanguagesFallback, &fallback);
        return {value, true, fallback};
    }
    if (eq(selector, "INSTALLED_LANGUAGE")) {
        bool fallback = false;
        const auto value = installed_language_policy(&fallback);
        return {value, true, fallback};
    }
    return empty_unrecognized();
}

} }
