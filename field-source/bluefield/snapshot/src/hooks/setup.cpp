#include "src/hooks/setup.hpp"
#include "src/paths/paths.hpp"

namespace preloader_l::host {
std::int32_t create_named_hook_046970(Effects& effects, std::u16string_view module,
    std::string_view name, Callback callback, Address& original, Address* target) {
    effects.trace_hook("resolve begin",name,0);
    const Address handle = effects.get_module_handle(module);
    if (handle == 0) {effects.trace_hook("resolve end",name,11);return 11;}
    const Address address = effects.get_proc_address(handle, name);
    if (address == 0) {effects.trace_hook("resolve end",name,12);return 12;}
    if (target != nullptr) *target = address;
    effects.trace_hook("create begin",name,0);
    const auto status=effects.create_hook_0466e0(address, callback, original);
    effects.trace_hook("create end",name,status);return status;
}

void install_hook_00f120(StateRefs state, Effects& effects, std::u16string_view module,
    std::string_view name, Callback callback) {
    Address target = 0;
    std::int32_t status = create_named_hook_046970(effects, module, name, callback,
                                                state.originals.at(slot(callback)), &target);
    if (status == 0) {
        effects.trace_hook("queue begin",name,0);
        status = effects.queue_hook_enable(target);
        effects.trace_hook("queue end",name,status);
        if (status == 0) {
            state.enabled_targets_0a3810.push_back(target);
            return;
        }
        effects.log_text_006880("Queueing hook for ");
    } else {
        effects.log_text_006880("Hooking function ");
    }
    effects.log_text_006880(name);
    effects.log_text_006880(" failed; error ");
    effects.log_integer_00af90(status);
    effects.finish_log_0088e0();
}

void initialize_host_00e310(StateRefs state, Effects& effects) {
    auto& values = state.values_0a3828;
    auto& config = state.configuration;
    values.clear();
    // Obtain each map value BEFORE conversion: a conversion exception leaves the
    // newly inserted empty value visible, as in the binary's operator[] sequence.
    auto& client_key = values["ClientKey"];
    client_key = effects.filesystem_widen_006d70("SOFTWARE\\Wow6432Node\\Origin");
    auto& second_key = values["ClientKey2"];
    second_key = effects.filesystem_widen_006d70("SOFTWARE\\Electronic Arts\\EA Desktop");
    auto& client_value = values["ClientValue"];
    client_value = effects.filesystem_widen_006d70("ClientPath");
    values["ClientPath"] = config.executable_path;
    if (config.language_registry_key.empty()) {
        config.registry_spoof = 0;
    } else {
        auto parent = paths::parent_path_019820(config.language_registry_key);
        values["LanguageKey"] = std::move(parent);
        auto filename = paths::filename_00e120(config.language_registry_key);
        values["LanguageValue"] = std::move(filename);
        // Original map helper 0084e0 inserts a missing INSTALLED_LANGUAGE as empty.
        const auto& installed = config.game_attributes["INSTALLED_LANGUAGE"];
        auto& language = values["Language"];
        language = effects.filesystem_widen_006d70(installed);
    }
    state.enabled_targets_0a3810.clear();
    auto mask = effects.version_condition_mask(0, 2, 3);
    mask = effects.version_condition_mask(mask, 1, 3);
    mask = effects.version_condition_mask(mask, 0x20, 3);
    const bool modern = effects.verify_version(VersionRequest{}, 0x23, mask);
    const auto status = effects.initialize_engine_046a20();
    effects.log_text_006880("MinHook initialization ");
    effects.log_text_006880(status == 0 ? "succeeded" : "failed");
    effects.finish_log_0088e0();
    if (status != 0) return;
    install_hook_00f120(state, effects, u"ws2_32.dll", "connect", Callback::connect_00fd00);
    const auto registry_module = modern ? u"KernelBase.dll" : u"advapi32.dll";
    install_hook_00f120(state, effects, registry_module, "RegOpenKeyExA", Callback::reg_open_a_00f800);
    install_hook_00f120(state, effects, registry_module, "RegOpenKeyExW", Callback::reg_open_w_00f960);
    install_hook_00f120(state, effects, registry_module, "RegCloseKey", Callback::reg_close_00f5b0);
    install_hook_00f120(state, effects, registry_module, "RegQueryValueExA", Callback::reg_query_a_00fac0);
    install_hook_00f120(state, effects, registry_module, "RegQueryValueExW", Callback::reg_query_w_00fb60);
    if (config.block_reruns != 0) {
        const auto process_module = modern ? u"KernelBase.dll" : u"Kernel32.dll";
        install_hook_00f120(state, effects, process_module, "CreateProcessA", Callback::create_process_a_00f3f0);
        install_hook_00f120(state, effects, process_module, "CreateProcessW", Callback::create_process_w_00f4d0);
    }
    if (!config.documents.empty()) {
        values["Documents"] = config.documents;
        effects.log_text_006880("hooking special folders");
        effects.finish_log_0088e0();
        install_hook_00f120(state, effects, u"shell32.dll", "SHGetFolderPathA", Callback::folder_a_00fc00);
        install_hook_00f120(state, effects, u"shell32.dll", "SHGetFolderPathW", Callback::folder_w_00fc80);
    }
}
}
