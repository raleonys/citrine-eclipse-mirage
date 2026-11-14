// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/platform/windows_effects.hpp"
#include <bit>
#include <exception>

namespace laureondo::windows {
license::FileStatus WindowsEffects::status_01ea20(const Path& path,std::uint32_t flags) {
    const auto status=status::status_01ea20(path,flags,stats_);
    return {status.kind,std::bit_cast<std::int32_t>(status.error)};
}
WindowsEffects::Path WindowsEffects::filesystem_widen_006d70(std::string_view text) {
    return encoding::filesystem_widen_006d70(text,encoding_,errors_);
}
WindowsEffects::Path WindowsEffects::filename_00e120(const Path& path){return path::filename_00e120(path);}
WindowsEffects::Path WindowsEffects::parent_path_019820(const Path& path){return path::parent_path_019820(path);}
WindowsEffects::Path WindowsEffects::join_016b00(const Path& left,const Path& right){return path::join_016b00(left,right);}
std::string WindowsEffects::path_utf8_01e3f0(const Path& path){return encoding::path_utf8_01e3f0(path,encoding_,errors_);}
const char* WindowsEffects::online_auth_virtual_30(const char* client){return remaining_.online_auth_virtual_30(client);}
[[noreturn]] void WindowsEffects::filesystem_error_018130(std::string_view op,std::int32_t error,const Path& path){remaining_.filesystem_error_018130(op,error,path);std::terminate();}
bool WindowsEffects::file_hash_matches_042d20(const Path& path,std::string expected){return remaining_.file_hash_matches_042d20(path,std::move(expected));}
std::string WindowsEffects::load_token_02afa0(const Path& path){return remaining_.load_token_02afa0(path);}
std::string WindowsEffects::current_time_041b60_null_true(){return remaining_.current_time_041b60_null_true();}
bool WindowsEffects::wine_detected_045010(){return remaining_.wine_detected_045010();}
void WindowsEffects::log_literal_006880(std::string_view text){remaining_.log_literal_006880(text);}
void WindowsEffects::log_string_006860(std::string_view text){remaining_.log_string_006860(text);}
bool WindowsEffects::write_ticket_03c1d0(const Path& path,std::string_view text){return remaining_.write_ticket_03c1d0(path,text);}
void WindowsEffects::dialog_038bc0(std::string_view title,std::string_view text,std::uint32_t flags){remaining_.dialog_038bc0(title,text,flags);}
[[noreturn]] void WindowsEffects::exit_05cdc0(std::int32_t code){remaining_.exit_05cdc0(code);std::terminate();}
[[noreturn]] void WindowsEffects::no_response_02bcd0_04d228(){remaining_.no_response_02bcd0_04d228();std::terminate();}
} // namespace laureondo::windows
