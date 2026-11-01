#include "preloader_l/platform/environment_policy.hpp"
#include "preloader_l/config/settings_values.hpp"
#include <stddef.h>
#if defined(_WIN32)
extern "C" {
__declspec(dllimport) void* __stdcall GetProcessHeap();
__declspec(dllimport) void* __stdcall HeapAlloc(void*, unsigned long, size_t);
__declspec(dllimport) int __stdcall HeapFree(void*, unsigned long, void*);
}
#else
#include <stdlib.h>
#endif

namespace preloader_l { namespace platform {
namespace {
bool equals(preloader_l::config::ValueView value, const char* literal) noexcept {
    return preloader_l::config::value_equals(value, literal);
}

char* allocate_text(size_t bytes) noexcept {
    if(!bytes) return nullptr;
#if defined(_WIN32)
    void* heap=GetProcessHeap();
    return heap ? static_cast<char*>(HeapAlloc(heap,0,bytes)) : nullptr;
#else
    return static_cast<char*>(malloc(bytes));
#endif
}
void release_text(char* text) noexcept {
    if(!text) return;
#if defined(_WIN32)
    void* heap=GetProcessHeap();
    if(heap) (void)HeapFree(heap,0,text);
#else
    free(text);
#endif
}
}

EnvironmentStatus apply_environment(const preloader_l::config::SettingsProvider& settings,
                                    const EnvironmentBackend* backend,
                                    uint32_t* written_mask,
                                    uint32_t* preserved_mask) noexcept {
    if (!written_mask || !preserved_mask) return EnvironmentStatus::invalid_argument;
    *written_mask = 0;
    *preserved_mask = 0;
    if (!backend || !backend->length || !backend->set) return EnvironmentStatus::backend_required;

    const char* names[2] = {"EALaunchOfflineMode", "EAGameLocale"};
    EnvironmentStatus result = EnvironmentStatus::ok;

    for (size_t i = 0; i < 2; ++i) {
        const uint32_t existing = backend->length(names[i], backend->context);
        if (existing == environment_length_query_error) {
            // Fail closed: an unreadable process variable may already exist.
            // Never overwrite it or later classify it as module-created.
            *preserved_mask |= static_cast<uint32_t>(1u << i);
            result = EnvironmentStatus::query_failed;
            continue;
        }
        if (existing != 0) {
            // Any nonzero length is evidence of pre-existing host ownership.
            // This deliberately includes values >= the bounded read policy.
            *preserved_mask |= static_cast<uint32_t>(1u << i);
            continue;
        }

        preloader_l::config::ValueView value = preloader_l::config::missing_value();
        if (i == 0) {
            if (!preloader_l::config::lookup_setting(settings, preloader_l::config::SettingKey::online, &value))
                value = preloader_l::config::literal_value("true");
            const char* transformed = equals(value, "false") ? "true" : "false";
            if (backend->set(names[i], transformed, backend->context))
                *written_mask |= 1u;
            else
                result = EnvironmentStatus::set_failed;
        } else {
            if (!preloader_l::config::lookup_setting(settings, preloader_l::config::SettingKey::language, &value))
                value = preloader_l::config::literal_value("en_US");
            if (!value.data || value.size == static_cast<size_t>(-1)) {
                result = EnvironmentStatus::set_failed;
                continue;
            }
            const size_t bytes=value.size+1u;
            char* text=allocate_text(bytes);
            if(!text){ result=EnvironmentStatus::set_failed; continue; }
            for(size_t j=0;j<value.size;++j) text[j]=value.data[j];
            text[value.size]=0;
            const bool set_ok=backend->set(names[i],text,backend->context);
            release_text(text);
            if(set_ok) *written_mask |= 2u;
            else result = EnvironmentStatus::set_failed;
        }
    }
    return result;
}

EnvironmentStatus restore_environment(const EnvironmentBackend* backend,
                                      uint32_t written_mask,
                                      uint32_t* restored_mask) noexcept {
    if (!restored_mask) return EnvironmentStatus::invalid_argument;
    *restored_mask = 0;
    if (!backend || !backend->set) return EnvironmentStatus::backend_required;

    const char* names[2] = {"EALaunchOfflineMode", "EAGameLocale"};
    EnvironmentStatus result = EnvironmentStatus::ok;
    for (size_t i = 0; i < 2; ++i) {
        const uint32_t bit = static_cast<uint32_t>(1u << i);
        if ((written_mask & bit) == 0u) continue;
        // SetEnvironmentVariableA(name, nullptr) deletes the process variable.
        if (backend->set(names[i], nullptr, backend->context))
            *restored_mask |= bit;
        else
            result = EnvironmentStatus::set_failed;
    }
    return result;
}

} }
