#if defined(_WIN32)
#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/config/process_configuration.hpp"
#include "preloader_l/config/runtime_provider.hpp"
#include "preloader_l/config/settings_provider.hpp"
#include "preloader_l/handlers/selector_pipeline.hpp"
#include "preloader_l/platform/environment_policy.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

using namespace preloader_l;

namespace {
char g_command_line[256] = "game.exe";
const char* g_config_language = "en_US";

bool eq(config::ValueView value, const char* literal) {
    return value.present && value.data && literal &&
           value.size == std::strlen(literal) &&
           std::memcmp(value.data, literal, value.size) == 0;
}

void command_line(const char* text) {
    std::memset(g_command_line, 0, sizeof(g_command_line));
    const size_t n = std::strlen(text);
    if (n >= sizeof(g_command_line)) std::abort();
    std::memcpy(g_command_line, text, n);
}

struct EnvironmentCapture {
    char locale[64]{};
    char offline[16]{};
};

uint32_t env_length(const char*, void*) noexcept { return 0u; }

bool env_set(const char* name, const char* value, void* context) noexcept {
    auto* capture = static_cast<EnvironmentCapture*>(context);
    if (!capture || !name || !value) return false;
    char* out = nullptr;
    size_t cap = 0;
    if (std::strcmp(name, "EAGameLocale") == 0) {
        out = capture->locale; cap = sizeof(capture->locale);
    } else if (std::strcmp(name, "EALaunchOfflineMode") == 0) {
        out = capture->offline; cap = sizeof(capture->offline);
    } else {
        return false;
    }
    const size_t n = std::strlen(value);
    if (n >= cap) return false;
    std::memcpy(out, value, n + 1u);
    return true;
}

bool selector_language_is(const char* expected) {
    constexpr char id[] = "LANGUAGE";
    const auto single = handlers::resolve_get_setting_selector({id, sizeof(id) - 1u});
    if (!single.recognized || !eq(single.value, expected)) return false;
    const auto aggregate = handlers::resolve_runtime_settings_policy();
    if (!eq(aggregate.language, expected)) return false;
    constexpr char installed[] = "INSTALLED_LANGUAGE";
    const auto game = handlers::resolve_get_game_info_selector({installed, sizeof(installed) - 1u});
    return game.recognized && eq(game.value, expected);
}

bool environment_language_is(const char* expected) {
    EnvironmentCapture capture{};
    const platform::EnvironmentBackend backend{env_length, env_set, &capture};
    uint32_t written = 0, preserved = 0;
    const auto status = platform::apply_environment(
        config::runtime_settings_provider(), &backend, &written, &preserved);
    return status == platform::EnvironmentStatus::ok && preserved == 0u &&
           written == 3u && std::strcmp(capture.locale, expected) == 0;
}
}

extern "C" char* GetCommandLineA() { return g_command_line; }
extern "C" void* GetProcessHeap() { return reinterpret_cast<void*>(1); }
extern "C" void* HeapAlloc(void*, unsigned long, size_t bytes) { return std::malloc(bytes); }
extern "C" int HeapFree(void*, unsigned long, void* memory) { std::free(memory); return 1; }

namespace preloader_l { namespace config {
ValueView runtime_configuration_setting(const char* key) noexcept {
    if (!key) return missing_value();
    if (std::strcmp(key, "Language") == 0) return literal_value(g_config_language);
    if (std::strcmp(key, "Online") == 0) return literal_value("true");
    if (std::strcmp(key, "Environment") == 0) return literal_value("production");
    return missing_value();
}
} }

int main() {
    config::clear_runtime_overrides();

    // No CLI override: Config2 Language remains shared policy.
    command_line("game.exe");
    config::synchronize_process_language_policy();
    if (!selector_language_is("en_US")) return 1;
    if (!environment_language_is("en_US")) return 2;

    // CLI override must reverse-publish into every shared language consumer.
    command_line("game.exe -emuGameLang fr_FR");
    config::synchronize_process_language_policy();
    if (!selector_language_is("fr_FR")) return 3;
    if (!environment_language_is("fr_FR")) return 4;

    config::ValueView shared = config::missing_value();
    if (!config::lookup_setting(config::runtime_settings_provider(), config::SettingKey::language, &shared) ||
        !eq(shared, "fr_FR")) return 5;

    // Preserve behavioral last-write-wins behavior for repeated exact switches.
    command_line("game.exe -emuGameLang fr_FR -other x -emuGameLang de_DE");
    config::synchronize_process_language_policy();
    if (!selector_language_is("de_DE")) return 6;
    if (!environment_language_is("de_DE")) return 7;

    // Retraction is explicit: absent CLI override restores Config2 policy.
    command_line("game.exe -unrelated value");
    config::synchronize_process_language_policy();
    if (!selector_language_is("en_US")) return 8;
    if (!environment_language_is("en_US")) return 9;

    config::clear_runtime_overrides();
    return 0;
}

#else
int main(){return 0;}
#endif
