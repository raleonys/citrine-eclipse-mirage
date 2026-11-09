// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/runtime/delegating_effects.hpp"
#include "citrine_eclipse_mirage/core/paths.hpp"
#include <exception>

namespace citrine_eclipse_mirage::online {
std::uint32_t DelegatingEffects::suppress_errors_0450a0() {
    return system_services::libraries::suppress_errors_0450a0(libraries_);
}
void DelegatingEffects::restore_error_mode(std::uint32_t mode) {
    (void)libraries_.set_error_mode(mode);
}
Path DelegatingEffects::filesystem_widen_006d70(std::string_view value) {
    return encoding::filesystem_widen_006d70(value, encoding_, errors_);
}
Path DelegatingEffects::parent_path_019820(const Path& value) {
    return path::parent_path_019820(value);
}
Path DelegatingEffects::join_016b00(const Path& a, const Path& b) {
    return path::join_016b00(a, b);
}
Module DelegatingEffects::load_copy_0450d0(const Path& value) {
    static_assert(sizeof(wchar_t) == sizeof(char16_t));
    std::wstring wide;
    wide.reserve(value.size());
    for (char16_t unit : value) wide.push_back(static_cast<wchar_t>(unit));
    return system_services::libraries::load_copy_0450d0(libraries_, wide);
}
Procedure DelegatingEffects::get_proc_address(Module module, std::string_view name) {
    return remaining_.get_proc_address(module, name);
}
std::int32_t DelegatingEffects::call_version_check(Procedure procedure) {
    return remaining_.call_version_check(procedure);
}
Object DelegatingEffects::call_create_online(Procedure procedure, const char16_t* cookie) {
    return remaining_.call_create_online(procedure, cookie);
}
std::int32_t DelegatingEffects::status_virtual_08(Object object) { return remaining_.status_virtual_08(object); }
const char* DelegatingEffects::username_virtual_10(Object object) { return remaining_.username_virtual_10(object); }
const char* DelegatingEffects::persona_virtual_18(Object object) { return remaining_.persona_virtual_18(object); }
const char* DelegatingEffects::user_id_virtual_20(Object object) { return remaining_.user_id_virtual_20(object); }
void DelegatingEffects::destroy_virtual_00(Object object) { remaining_.destroy_virtual_00(object); }
bool DelegatingEffects::free_library(Module module) { return libraries_.free_library(module); }
status::Status DelegatingEffects::status_01ea20(const Path& path, std::uint32_t flags) {
    return status::status_01ea20(path, flags, stats_);
}
[[noreturn]] void DelegatingEffects::filesystem_error_018130(std::string_view operation,
    std::uint32_t code, const Path& path) {
    remaining_.filesystem_error_018130(operation, code, path);
    std::terminate();
}
std::string DelegatingEffects::read_binary_03b3f0(const Path& path) {
    return file::read_binary_03b3f0(path, files_);
}
bool DelegatingEffects::write_binary_03c1d0(const Path& path, std::string_view value) {
    return file::write_binary_03c1d0(path, value, files_);
}
void DelegatingEffects::log_literal_006880(std::string_view value) { remaining_.log_literal_006880(value); }
void DelegatingEffects::log_finish_0088e0() { remaining_.log_finish_0088e0(); }
void DelegatingEffects::dialog_038bc0(std::string_view a, std::string_view b, std::uint32_t flags) {
    remaining_.dialog_038bc0(a, b, flags);
}
void DelegatingEffects::wait_steps_036880(std::int32_t count) { remaining_.wait_steps_036880(count); }
void DelegatingEffects::finish_flag_036820() { remaining_.finish_flag_036820(); }
} // namespace citrine_eclipse_mirage::online
