#include "recovered_effects.hpp"
#include "../anadius-path-source/paths.hpp"
#include <exception>

namespace anadius::online_recovered {
std::uint32_t RecoveredEffects::suppress_errors_0450a0() {
    return recovered::libraries::suppress_errors_0450a0(libraries_);
}
void RecoveredEffects::restore_error_mode(std::uint32_t mode) {
    (void)libraries_.set_error_mode(mode);
}
Path RecoveredEffects::filesystem_widen_006d70(std::string_view value) {
    return encoding_recovered::filesystem_widen_006d70(value, encoding_, errors_);
}
Path RecoveredEffects::parent_path_019820(const Path& value) {
    return path_recovered::parent_path_019820(value);
}
Path RecoveredEffects::join_016b00(const Path& a, const Path& b) {
    return path_recovered::join_016b00(a, b);
}
Module RecoveredEffects::load_copy_0450d0(const Path& value) {
    static_assert(sizeof(wchar_t) == sizeof(char16_t));
    std::wstring wide;
    wide.reserve(value.size());
    for (char16_t unit : value) wide.push_back(static_cast<wchar_t>(unit));
    return recovered::libraries::load_copy_0450d0(libraries_, wide);
}
Procedure RecoveredEffects::get_proc_address(Module module, std::string_view name) {
    return remaining_.get_proc_address(module, name);
}
std::int32_t RecoveredEffects::call_version_check(Procedure procedure) {
    return remaining_.call_version_check(procedure);
}
Object RecoveredEffects::call_create_online(Procedure procedure, const char16_t* cookie) {
    return remaining_.call_create_online(procedure, cookie);
}
std::int32_t RecoveredEffects::status_virtual_08(Object object) { return remaining_.status_virtual_08(object); }
const char* RecoveredEffects::username_virtual_10(Object object) { return remaining_.username_virtual_10(object); }
const char* RecoveredEffects::persona_virtual_18(Object object) { return remaining_.persona_virtual_18(object); }
const char* RecoveredEffects::user_id_virtual_20(Object object) { return remaining_.user_id_virtual_20(object); }
std::string RecoveredEffects::copy_username(Object o){return remaining_.copy_username(o);}
std::string RecoveredEffects::copy_persona(Object o){return remaining_.copy_persona(o);}
std::string RecoveredEffects::copy_user_id(Object o){return remaining_.copy_user_id(o);}
void RecoveredEffects::destroy_virtual_00(Object object) { remaining_.destroy_virtual_00(object); }
bool RecoveredEffects::free_library(Module module) { return libraries_.free_library(module); }
status_recovered::Status RecoveredEffects::status_01ea20(const Path& path, std::uint32_t flags) {
    return status_recovered::status_01ea20(path, flags, stats_);
}
[[noreturn]] void RecoveredEffects::filesystem_error_018130(std::string_view operation,
    std::uint32_t code, const Path& path) {
    remaining_.filesystem_error_018130(operation, code, path);
    std::terminate();
}
std::string RecoveredEffects::read_binary_03b3f0(const Path& path) {
    return file_recovered::read_binary_03b3f0(path, files_);
}
bool RecoveredEffects::write_binary_03c1d0(const Path& path, std::string_view value) {
    return file_recovered::write_binary_03c1d0(path, value, files_);
}
void RecoveredEffects::log_literal_006880(std::string_view value) { remaining_.log_literal_006880(value); }
void RecoveredEffects::log_finish_0088e0() { remaining_.log_finish_0088e0(); }
void RecoveredEffects::dialog_038bc0(std::string_view a, std::string_view b, std::uint32_t flags) {
    remaining_.dialog_038bc0(a, b, flags);
}
void RecoveredEffects::wait_steps_036880(std::int32_t count) { remaining_.wait_steps_036880(count); }
void RecoveredEffects::finish_flag_036820() { remaining_.finish_flag_036820(); }
} // namespace anadius::online_recovered
