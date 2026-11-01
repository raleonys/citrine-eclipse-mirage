#include "teardown.hpp"
#include <type_traits>

namespace anadius::configuration_defaults {
namespace {
template<class T>void release(T& value) noexcept {
    static_assert(std::is_nothrow_default_constructible_v<T>);
    static_assert(noexcept(value.swap(value)));
    T empty;value.swap(empty);
}
void release_paths(std::vector<configuration::Path>& paths) noexcept {
    // Native vector elements are visited forward before backing storage free.
    for(auto& path:paths)release(path);
    release(paths);
}
void release_content(std::vector<configuration::Content>& values) noexcept {
    for(auto& value:values){
        release(value.state);release(value.version);release(value.name);release(value.id);
    }
    release(values);
}
void release_entitlements(std::vector<configuration::Entitlement>& values) noexcept {
    for(auto& value:values){
        release(value.tag);release(value.type);release(value.version);release(value.group);release(value.id);
    }
    release(values);
}
}
void release_aggregate_storage_037f70(AggregateStorage storage) noexcept {
    auto& c=storage.configuration;auto& p=storage.achievement_paths;
    release(c.documents);                         // +518
    release(storage.loaded_modules);              // +4f8 storage only
    release_paths(c.extra_dlls);                   // +4e0
    release_paths(c.extra_dlls_from_main);         // +4c8
    release(p.ini_path);                          // +4a8
    release(c.steam_app_id);                      // +488
    release(c.achievements);                      // +478 host map
    // +428 CALL017a30 is RET, not an OS mutex-destroy operation.
    release(p.xml_path);                          // +408
    release(c.achievements_set);                  // +3e8
    release_entitlements(c.entitlements);         // +3d0 forward, tag..id
    release_content(c.content);                   // +3b8 forward, state..id
    release(c.game_attributes);                   // +3a8 host map
    release(c.emulator_attributes);               // +398 host map
    release(c.is_under_age);                      // +378
    release(c.subscriber_level);                  // +358
    release(c.is_steam_subscriber);               // +338
    release(c.is_subscriber);                     // +318
    release(c.currency);                          // +2f8
    release(c.country);                           // +2d8
    release(c.sdk);                               // +2b8
    // +260 CALL017a30 is RET. Execution metadata retires independently.
    release(c.user_id);                           // +238
    release(c.persona_id);                        // +218
    release(c.username);                          // +1f8
    release(c.denuvo_dll);                        // +1d8
    release(c.dll_hash);                          // +1b8
    release(c.exe_hash);                          // +198
    release(c.denuvo_token);                      // +178
    release(c.license_directory);                 // +158
    storage.authentication.reset();              // +138
    release(c.content_id);                        // +118
    release(c.language_registry_key);             // +f8
    release(c.default_avatar);                    // +d0
    release(c.avatar);                            // +b0
    release(c.last_avatar_path);                  // +80
    release(c.last_profile_path);                 // +60
    release(c.cookie_path);                       // +40
    release(c.user_directory);                    // +20
    release(c.executable_path);                   // +0 native tail JMP00ce10
}
}
