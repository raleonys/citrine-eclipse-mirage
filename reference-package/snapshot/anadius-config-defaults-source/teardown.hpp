#pragma once
#include "../anadius-config-source/configuration.hpp"
#include "../anadius-achievement-storage-source/storage.hpp"
#include "../anadius-libraries-source/libraries.hpp"
#include "../anadius-license-source/auth_state.hpp"
#include <optional>

namespace anadius::configuration_defaults {
struct AggregateStorage {
    configuration::State& configuration;
    achievement_storage::Paths& achievement_paths;
    std::vector<recovered::libraries::Module>& loaded_modules;
    std::optional<license_recovered::AuthenticationCode>& authentication;
};
// Owner-only normal cleanup after ALL workers/providers/callbacks retire.
// Releases host containers in native037f70 field-group order. Does not invoke
// FreeLibrary, join, online destruction, OS mutex destruction or outer CRT.
// Host containers remain valid/empty for later compiler destruction; native
// node/sentinel/allocator ABI and repeated native destruction are not claimed.
void release_aggregate_storage_037f70(AggregateStorage) noexcept;
}
