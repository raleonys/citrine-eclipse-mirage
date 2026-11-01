#include "callbacks.hpp"

namespace anadius::host_callbacks_recovered {
std::int32_t create_process_a_00f3f0(const Values& values, Effects& effects,
                                   const ProcessCall<char>& call) {
    if (registry_recovered::compare_narrow_0086b0(values, effects, call.application, "ClientPath") == 0)
        return 0;
    return effects.original_create_process_a(call);
}
std::int32_t create_process_w_00f4d0(const Values& values, Effects& effects,
                                   const ProcessCall<char16_t>& call) {
    if (registry_recovered::compare_wide_0087f0(values, effects, call.application, "ClientPath") == 0)
        return 0;
    return effects.original_create_process_w(call);
}
bool folder_value_a_008db0(const Values& values, registry_recovered::ValueEffects& effects,
                         std::int32_t folder_id, char* path, std::uint32_t* result) {
    if (folder_id != 5) return false;
    const auto value = registry_recovered::resolve_narrow_008980(values, effects, "Documents");
    if (value.empty()) return false;
    const auto error = effects.crt_strcpy_s(path, 0x104, value.c_str());
    *result = static_cast<std::uint32_t>(error != 0);
    return true;
}
bool folder_value_w_008fa0(const Values& values, registry_recovered::ValueEffects& effects,
                         std::int32_t folder_id, char16_t* path, std::uint32_t* result) {
    if (folder_id != 5) return false;
    const auto value = registry_recovered::resolve_wide_008b80(values, "Documents");
    if (value.empty()) return false;
    const auto error = effects.crt_wcscpy_s(path, 0x104, value.c_str());
    *result = static_cast<std::uint32_t>(error != 0);
    return true;
}
std::uint32_t folder_a_00fc00(const Values& values, Effects& effects, const FolderCall<char>& call) {
    std::uint32_t result;
    if (folder_value_a_008db0(values, effects, call.folder_id, call.path, &result)) return result;
    return effects.original_folder_a(call);
}
std::uint32_t folder_w_00fc80(const Values& values, Effects& effects, const FolderCall<char16_t>& call) {
    std::uint32_t result;
    if (folder_value_w_008fa0(values, effects, call.folder_id, call.path, &result)) return result;
    return effects.original_folder_w(call);
}
}
