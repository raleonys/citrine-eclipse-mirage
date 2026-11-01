#include "preloader_l/platform/addon_selection.hpp"
#include "preloader_l/platform/addon_hook_policy.hpp"
#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/config/settings_values.hpp"

namespace preloader_l { namespace platform {
bool read_committed_addon_selection(AddonHookSelection* out) noexcept {
    if (!out) return false;
    out->mask=0u;
    preloader_l::config::RuntimeConfigurationReadLease lease{};
    if (!preloader_l::config::begin_runtime_configuration_read(&lease)) return false;
    if (preloader_l::config::runtime_configuration_loaded()) {
        const auto block = preloader_l::config::runtime_configuration_setting("BlockReruns");
        const auto docs = preloader_l::config::runtime_configuration_setting("Documents");
        if (block.present && addon_hook_policy::exact_true(block.data,block.size))
            out->mask |= 1u;
        if (docs.present && addon_hook_policy::selected_documents(5,docs.data,docs.size))
            out->mask |= 2u;
    }
    preloader_l::config::end_runtime_configuration_read(&lease);
    return true;
}
} }
